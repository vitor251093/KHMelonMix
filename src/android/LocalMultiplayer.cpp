#include "LocalMultiplayer.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <linux/ashmem.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <time.h>
#include "MelonDS.h"
#include "ancillary.h"
#include <android/log.h>
#include <linux/un.h>

enum RequestType {
    SHARED_MEMORY_FD = 1,
    SEMAPHORE_MEMORY_FD = 2,
};

void debug(std::string message)
{
#ifndef NDEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "LocalMultiplayer", "%s", message.c_str());
#endif
}

struct MPQueueHeader
{
    u16 NumInstances;
    u16 InstanceBitmask;  // bitmask of all instances present
    u16 ConnectedBitmask; // bitmask of which instances are ready to send/receive packets
    u32 PacketWriteOffset;
    u32 ReplyWriteOffset;
    u16 MPHostInstanceID; // instance ID from which the last CMD frame was sent
    u16 MPReplyBitmask;   // bitmask of which clients replied in time
};

struct MPPacketHeader
{
    u32 Magic;
    u32 SenderID;
    u32 Type;       // 0=regular 1=CMD 2=reply 3=ack
    u32 Length;
    u64 Timestamp;
};

const u32 kMemorySize = 0x20000;
const u32 kPacketStart = sizeof(MPQueueHeader);
const u32 kReplyStart = kMemorySize / 2;
const u32 kPacketEnd = kReplyStart;
const u32 kReplyEnd = kMemorySize;

const u32 kSemMemorySize = sizeof(short) * 32;

bool IsMasterInstance = true;
volatile bool Running = false;
Platform::Thread* MasterThread;

int MemoryFile = -1;
u8* Memory;
int SemMemoryFile = -1;
u16* SemMemory;
int InstanceID;
u32 PacketReadOffset;
u32 ReplyReadOffset;

int RecvTimeout;

int LastHostID;

bool SemInited[32];
int SemPool[32];

void StopMasterInstanceThread();

void MasterInstanceThread()
{
    int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd < 0)
    {
        debug("Failed to create socket");
        debug(strerror(errno));
        return;
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));

    std::string socketPath = MelonDSAndroid::internalFilesDir + "/mp.socket";
    sprintf(address.sun_path, "%s", socketPath.c_str());
    address.sun_family = AF_UNIX;

    unlink(address.sun_path);
    if (bind(socketFd, (struct sockaddr*) &address, sizeof(struct sockaddr_un)) != 0)
    {
        debug("Failed to bind socket");
        debug(strerror(errno));
        close(socketFd);
        return;
    }

    if (listen(socketFd, 4) != 0)
    {
        debug("Failed to start listening");
        debug(strerror(errno));
        return;
    }

    u8 buffer[16];
    while (Running)
    {
        struct sockaddr_un remote;
        socklen_t length = (socklen_t) sizeof(struct sockaddr_un);
        debug("Waiting for connection...");
        int remoteSocketFd = accept(socketFd, (struct sockaddr*) &remote, &length);
        if (!Running)
            break;

        debug("Connection received");
        int readBytes;

        for(;;)
        {
            readBytes = recv(remoteSocketFd, buffer, 16, 0);
            if (readBytes <= 0)
                break;

            switch (buffer[0])
            {
                case RequestType::SHARED_MEMORY_FD:
                    debug("Sending FD to slave");
                    if (ancil_send_fd(remoteSocketFd, MemoryFile) != 0)
                    {
                        debug("Failed to send FD");
                        debug(strerror(errno));
                    }
                    break;
                case RequestType::SEMAPHORE_MEMORY_FD:
                    debug("Sending shared semaphores FD to slave");
                    if (ancil_send_fd(remoteSocketFd, SemMemoryFile) != 0)
                    {
                        debug("Failed to send shared semaphores FD");
                        debug(strerror(errno));
                    }
                    break;
            }
        }

        close(remoteSocketFd);
    }

    close(socketFd);
}

void SemPoolInit()
{
    for (int i = 0; i < 32; i++)
    {
        SemPool[i] = -1;
        SemInited[i] = false;
    }
}

void SemDeinit(int num);

void SemPoolDeinit()
{
    for (int i = 0; i < 32; i++)
        SemDeinit(i);
}

bool SemInit(int num)
{
    if (SemInited[num])
        return true;

    debug("Initializing semaphore " + std::to_string(num));
    char semname[18];
    sprintf(semname, "/melonNIFI_Lock%02d", num);

    std::string filename = MelonDSAndroid::internalFilesDir + semname;
    int lockFd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    SemPool[num] = lockFd;
    SemInited[num] = true;

    return lockFd != -1;
}

void SemDeinit(int num)
{
    debug("Destroying semaphore " + std::to_string(num));
    if (SemPool[num] != -1)
    {
        close(SemPool[num]);
        SemPool[num] = -1;
    }

    SemInited[num] = false;
}

bool SemPost(int num)
{
    SemInit(num);

    flock(SemPool[num], LOCK_EX);
    SemMemory[num]++;

    return flock(SemPool[num], LOCK_UN) == 0;
}

bool SemWait(int num, int timeout)
{
    long remainingWaitingTime = timeout;
    do
    {
        flock(SemPool[num], LOCK_EX);
        bool gotLock = false;
        if (SemMemory[num] > 0)
        {
            SemMemory[num]--;
            gotLock = true;
        }
        flock(SemPool[num], LOCK_UN);

        if (gotLock)
            return true;

        if (remainingWaitingTime <= 0)
            break;

        usleep(1000);
        remainingWaitingTime--;
    } while (true);

    return false;
}

void SemReset(int num)
{
    while (SemWait(num, 0));
}

void LockMemory()
{
    flock(MemoryFile, LOCK_EX);
}

void UnlockMemory()
{
    flock(MemoryFile, LOCK_UN);
}

int CreateSharedMemory(const char* name, u32 size)
{
    static void* libandroid = dlopen("libandroid.so", RTLD_LAZY | RTLD_LOCAL);
    using type_ASharedMemory_create = int(*)(const char* name, size_t size);
    static void* symbol = dlsym(libandroid, "ASharedMemory_create");
    static auto shared_memory_create = reinterpret_cast<type_ASharedMemory_create>(symbol);

    if (shared_memory_create)
    {
        return shared_memory_create(name, size);
    }
    else
    {
        int fd = open("/dev/ashmem", O_RDWR);
        if (fd == -1)
            return -1;

        ioctl(fd, ASHMEM_SET_NAME, name);
        ioctl(fd, ASHMEM_SET_SIZE, size);
        return fd;
    }
}

void ReleaseResources()
{
    if (Memory != nullptr && Memory != MAP_FAILED)
        munmap(Memory, kMemorySize);

    if (MemoryFile != -1)
    {
        close(MemoryFile);
        MemoryFile = -1;
    }

    if (SemMemory != nullptr && SemMemory != MAP_FAILED)
        munmap(SemMemory, kSemMemorySize);

    if (SemMemoryFile != -1)
    {
        close(SemMemoryFile);
        SemMemoryFile = -1;
    }
}

bool LocalMultiplayer::Init()
{
    debug("Init");
    Running = true;

    if (IsMasterInstance)
    {
        debug("Master instance. Creating...");
        MemoryFile = CreateSharedMemory("melonNIFI", kMemorySize);
        if (MemoryFile == -1)
            return false;

        Memory = (u8*) mmap(NULL, kMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, MemoryFile, 0);
        if (Memory == MAP_FAILED)
        {
            ReleaseResources();
            return false;
        }

        SemMemoryFile = CreateSharedMemory("melonNIFISem", kSemMemorySize);
        if (SemMemoryFile == -1)
        {
            ReleaseResources();
            return false;
        }

        SemMemory = (u16*) mmap(NULL, kSemMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, SemMemoryFile, 0);
        if (SemMemory == MAP_FAILED)
        {
            ReleaseResources();
            return false;
        }

        LockMemory();
        memset(Memory, 0, kMemorySize);
        MPQueueHeader* header = (MPQueueHeader*) Memory;
        header->PacketWriteOffset = kPacketStart;
        header->ReplyWriteOffset = kReplyStart;
        UnlockMemory();

        for (int i = 0; i < 32; ++i)
        {
            SemMemory[i] = 1;
        }

        MasterThread = Platform::Thread_Create(MasterInstanceThread);
    }
    else
    {
        debug("Slave instance. Retrieving FD");

        int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFd < 0)
        {
            debug("Failed to create socket");
            debug(strerror(errno));
            return false;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));

        std::string socketPath = MelonDSAndroid::internalFilesDir + "/mp.socket";
        sprintf(addr.sun_path, "%s", socketPath.c_str());
        addr.sun_family = AF_UNIX;

        if (connect(socketFd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)) != 0)
        {
            debug("Failed to connect to master instance");
            debug(strerror(errno));
            close(socketFd);
            return false;
        }

        debug("Connected to master instance. Waiting for shared memory FD...");
        u8 data = RequestType::SHARED_MEMORY_FD;
        send(socketFd, &data, 1, 0);
        if (ancil_recv_fd(socketFd, &MemoryFile) != 0)
        {
            debug("Failed to receive shared memory FD");
            debug(strerror(errno));
            close(socketFd);
            return false;
        }

        data = RequestType::SEMAPHORE_MEMORY_FD;
        send(socketFd, &data, 1, 0);
        if (ancil_recv_fd(socketFd, &SemMemoryFile) != 0)
        {
            debug("Failed to receive semaphores shared memory FD");
            debug(strerror(errno));
            close(socketFd);
            return false;
        }

        close(socketFd);

        debug("Shared memory FD: " + std::to_string(MemoryFile));
        if (MemoryFile < 0)
        {
            debug("Failed to retrieve FD");
            ReleaseResources();
            return false;
        }

        if (SemMemoryFile < 0)
        {
            debug("Failed to retrieve shared semaphores FD");
            ReleaseResources();
            return false;
        }

        Memory = (u8*) mmap(NULL, kMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, MemoryFile, 0);
        if (Memory == MAP_FAILED)
        {
            debug("Failed to map memory");
            debug(strerror(errno));
            ReleaseResources();
            return false;
        }

        SemMemory = (u16*) mmap(NULL, kSemMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, SemMemoryFile, 0);
        if (Memory == MAP_FAILED)
        {
            debug("Failed to map semaphore memory");
            ReleaseResources();
            return false;
        }
    }

    LockMemory();
    MPQueueHeader* header = (MPQueueHeader*) Memory;

    u16 mask = header->InstanceBitmask;
    for (int i = 0; i < 16; i++)
    {
        if (!(mask & (1<<i)))
        {
            InstanceID = i;
            header->InstanceBitmask |= (1<<i);
            //header->ConnectedBitmask |= (1 << i);
            break;
        }
    }
    header->NumInstances++;

    PacketReadOffset = header->PacketWriteOffset;
    ReplyReadOffset = header->ReplyWriteOffset;

    UnlockMemory();

    // prepare semaphores
    // semaphores 0-15: regular frames; semaphore I is posted when instance I needs to process a new frame
    // semaphores 16-31: MP replies; semaphore I is posted when instance I needs to process a new MP reply

    SemPoolInit();
    SemInit(InstanceID);
    SemInit(16+InstanceID);

    debug("MP comm init OK, instance ID " + std::to_string(InstanceID));

    LastHostID = -1;
    RecvTimeout = 25;

    return true;
}

void LocalMultiplayer::DeInit()
{
    debug("DeInit");
    Running = false;
    if (MasterThread != nullptr)
    {
        debug("Waiting for master instance thread to stop");
        StopMasterInstanceThread();
        Platform::Thread_Wait(MasterThread);
        debug("Master instance thread stopped");
        Platform::Thread_Free(MasterThread);
        MasterThread = nullptr;
    }

    LockMemory();
    MPQueueHeader* header = (MPQueueHeader*) Memory;
    header->ConnectedBitmask &= ~(1 << InstanceID);
    header->InstanceBitmask &= ~(1 << InstanceID);
    header->NumInstances--;
    UnlockMemory();

    SemPoolDeinit();
    ReleaseResources();
}

void LocalMultiplayer::SetRecvTimeout(int timeout)
{
    RecvTimeout = timeout;
}

void LocalMultiplayer::Begin()
{
    LockMemory();
    MPQueueHeader* header = (MPQueueHeader*) Memory;
    PacketReadOffset = header->PacketWriteOffset;
    ReplyReadOffset = header->ReplyWriteOffset;
    SemReset(InstanceID);
    SemReset(16+InstanceID);
    header->ConnectedBitmask |= (1 << InstanceID);
    UnlockMemory();
}

void LocalMultiplayer::End()
{
    LockMemory();
    MPQueueHeader* header = (MPQueueHeader*) Memory;
    //SemReset(InstanceID);
    //SemReset(16+InstanceID);
    header->ConnectedBitmask &= ~(1 << InstanceID);
    UnlockMemory();
}

void FIFORead(int fifo, void* buf, int len)
{
    u8* data = (u8*) Memory;

    u32 offset, start, end;
    if (fifo == 0)
    {
        offset = PacketReadOffset;
        start = kPacketStart;
        end = kPacketEnd;
    }
    else
    {
        offset = ReplyReadOffset;
        start = kReplyStart;
        end = kReplyEnd;
    }

    if ((offset + len) >= end)
    {
        u32 part1 = end - offset;
        memcpy(buf, &data[offset], part1);
        memcpy(&((u8*)buf)[part1], &data[start], len - part1);
        offset = start + len - part1;
    }
    else
    {
        memcpy(buf, &data[offset], len);
        offset += len;
    }

    if (fifo == 0) PacketReadOffset = offset;
    else           ReplyReadOffset = offset;
}

void FIFOWrite(int fifo, void* buf, int len)
{
    u8* data = Memory;
    MPQueueHeader* header = (MPQueueHeader*)&data[0];

    u32 offset, start, end;
    if (fifo == 0)
    {
        offset = header->PacketWriteOffset;
        start = kPacketStart;
        end = kPacketEnd;
    }
    else
    {
        offset = header->ReplyWriteOffset;
        start = kReplyStart;
        end = kReplyEnd;
    }

    if ((offset + len) >= end)
    {
        u32 part1 = end - offset;
        memcpy(&data[offset], buf, part1);
        memcpy(&data[start], &((u8*)buf)[part1], len - part1);
        offset = start + len - part1;
    }
    else
    {
        memcpy(&data[offset], buf, len);
        offset += len;
    }

    if (fifo == 0) header->PacketWriteOffset = offset;
    else           header->ReplyWriteOffset = offset;
}

int SendPacketGeneric(u32 type, u8* packet, int len, u64 timestamp)
{
    LockMemory();
    MPQueueHeader* header = (MPQueueHeader*) Memory;

    u16 mask = header->ConnectedBitmask;

    // TODO: check if the FIFO is full!

    MPPacketHeader pktheader;
    pktheader.Magic = 0x4946494E;
    pktheader.SenderID = InstanceID;
    pktheader.Type = type;
    pktheader.Length = len;
    pktheader.Timestamp = timestamp;

    type &= 0xFFFF;
    int nfifo = (type == 2) ? 1 : 0;
    FIFOWrite(nfifo, &pktheader, sizeof(pktheader));
    if (len)
        FIFOWrite(nfifo, packet, len);

    if (type == 1)
    {
        // NOTE: this is not guarded against, say, multiple multiplay games happening on the same machine
        // we would need to pass the packet's SenderID through the wifi module for that
        header->MPHostInstanceID = InstanceID;
        header->MPReplyBitmask = 0;
        ReplyReadOffset = header->ReplyWriteOffset;
        SemReset(16 + InstanceID);
    }
    else if (type == 2)
    {
        header->MPReplyBitmask |= (1 << InstanceID);
    }

    UnlockMemory();

    if (type == 2)
    {
        SemPost(16 + header->MPHostInstanceID);
    }
    else
    {
        for (int i = 0; i < 16; i++)
        {
            if (mask & (1<<i))
                SemPost(i);
        }
    }

    return len;
}

int RecvPacketGeneric(u8* packet, bool block, u64* timestamp)
{
    for (;;)
    {
        if (!SemWait(InstanceID, block ? RecvTimeout : 0))
        {
            return 0;
        }

        LockMemory();
        u8* data = (u8*) Memory;
        MPQueueHeader* header = (MPQueueHeader*)&data[0];

        MPPacketHeader pktheader;
        FIFORead(0, &pktheader, sizeof(pktheader));

        if (pktheader.Magic != 0x4946494E)
        {
            debug("PACKET FIFO OVERFLOW\n");
            PacketReadOffset = header->PacketWriteOffset;
            SemReset(InstanceID);
            UnlockMemory();
            return 0;
        }

        if (pktheader.SenderID == InstanceID)
        {
            // skip this packet
            PacketReadOffset += pktheader.Length;
            if (PacketReadOffset >= kPacketEnd)
                PacketReadOffset += kPacketStart - kPacketEnd;

            UnlockMemory();
            continue;
        }

        if (pktheader.Length)
        {
            FIFORead(0, packet, pktheader.Length);

            if (pktheader.Type == 1)
                LastHostID = pktheader.SenderID;
        }

        if (timestamp) *timestamp = pktheader.Timestamp;
        UnlockMemory();
        return pktheader.Length;
    }
}

int LocalMultiplayer::SendPacket(u8* packet, int len, u64 timestamp)
{
    return SendPacketGeneric(0, packet, len, timestamp);
}

int LocalMultiplayer::RecvPacket(u8* packet, u64* timestamp)
{
    return RecvPacketGeneric(packet, false, timestamp);
}

int LocalMultiplayer::SendCmd(u8* packet, int len, u64 timestamp)
{
    return SendPacketGeneric(1, packet, len, timestamp);
}

int LocalMultiplayer::SendReply(u8* packet, int len, u64 timestamp, u16 aid)
{
    return SendPacketGeneric(2 | (aid<<16), packet, len, timestamp);
}

int LocalMultiplayer::SendAck(u8* packet, int len, u64 timestamp)
{
    return SendPacketGeneric(3, packet, len, timestamp);
}

int LocalMultiplayer::RecvHostPacket(u8* packet, u64* timestamp)
{
    if (LastHostID != -1)
    {
        // check if the host is still connected

        LockMemory();
        u8* data = (u8*) Memory;
        MPQueueHeader* header = (MPQueueHeader*)&data[0];
        u16 curinstmask = header->ConnectedBitmask;
        UnlockMemory();

        if (!(curinstmask & (1 << LastHostID)))
            return -1;
    }

    return RecvPacketGeneric(packet, true, timestamp);
}

u16 LocalMultiplayer::RecvReplies(u8* packets, u64 timestamp, u16 aidmask)
{
    u16 ret = 0;
    u16 myinstmask = (1 << InstanceID);
    u16 curinstmask;

    {
        LockMemory();
        u8* data = (u8*) Memory;
        MPQueueHeader* header = (MPQueueHeader*)&data[0];
        curinstmask = header->ConnectedBitmask;
        UnlockMemory();
    }

    // if all clients have left: return early
    if ((myinstmask & curinstmask) == curinstmask)
        return 0;

    for (;;)
    {
        if (!SemWait(16+InstanceID, RecvTimeout))
        {
            // no more replies available
            return ret;
        }

        LockMemory();
        u8* data = (u8*) Memory;
        MPQueueHeader* header = (MPQueueHeader*)&data[0];

        MPPacketHeader pktheader;
        FIFORead(1, &pktheader, sizeof(pktheader));

        if (pktheader.Magic != 0x4946494E)
        {
            debug("REPLY FIFO OVERFLOW\n");
            ReplyReadOffset = header->ReplyWriteOffset;
            SemReset(16+InstanceID);
            UnlockMemory();
            return 0;
        }

        if ((pktheader.SenderID == InstanceID) || // packet we sent out (shouldn't happen, but hey)
            (pktheader.Timestamp < (timestamp - 32))) // stale packet
        {
            // skip this packet
            ReplyReadOffset += pktheader.Length;
            if (ReplyReadOffset >= kReplyEnd)
                ReplyReadOffset += kReplyStart - kReplyEnd;

            UnlockMemory();
            continue;
        }

        if (pktheader.Length)
        {
            u32 aid = (pktheader.Type >> 16);
            FIFORead(1, &packets[(aid-1)*1024], pktheader.Length);
            ret |= (1 << aid);
        }

        myinstmask |= (1 << pktheader.SenderID);
        if (((myinstmask & curinstmask) == curinstmask) ||
            ((ret & aidmask) == aidmask))
        {
            // all the clients have sent their reply

            UnlockMemory();
            return ret;
        }

        UnlockMemory();
    }
}

void LocalMultiplayer::SetIsMasterInstance(bool masterInstance)
{
    IsMasterInstance = masterInstance;
}

void StopMasterInstanceThread()
{
    // The master instance thread is stopped by making a connection to the master instance socket. This way, the thread is unblocked and can then exit

    int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFd < 0)
    {
        debug("Failed to create socket");
        debug(strerror(errno));
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));

    std::string socketPath = MelonDSAndroid::internalFilesDir + "/mp.socket";
    sprintf(addr.sun_path, "%s", socketPath.c_str());
    addr.sun_family = AF_UNIX;

    if (connect(socketFd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)) != 0)
    {
        debug("Failed to connect to master instance");
        debug(strerror(errno));
    }

    close(socketFd);
}