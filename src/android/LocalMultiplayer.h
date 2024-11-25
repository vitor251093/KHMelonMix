#ifndef LOCALMULTIPLAYER_H
#define LOCALMULTIPLAYER_H

#include <types.h>

namespace LocalMultiplayer
{
    bool Init();
    void DeInit();

    void SetRecvTimeout(int timeout);

    void Begin();
    void End();

    int SendPacket(u8* data, int len, u64 timestamp);
    int RecvPacket(u8* data, u64* timestamp);
    int SendCmd(u8* data, int len, u64 timestamp);
    int SendReply(u8* data, int len, u64 timestamp, u16 aid);
    int SendAck(u8* data, int len, u64 timestamp);
    int RecvHostPacket(u8* data, u64* timestamp);
    u16 RecvReplies(u8* data, u64 timestamp, u16 aidmask);

    void SetIsMasterInstance(bool masterInstance);
}

#endif //LOCALMULTIPLAYER_H
