/*
    Copyright 2016-2019 StapleButter

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "pcap/pcap.h"
#include "Platform.h"
#include "MelonDS.h"
#include "android_fopen.h"
#include "LAN_Socket.h"
#include "LAN_PCap.h"
#include "FileUtils.h"
#include "Config.h"
#include "ROMManager.h"
#include "LocalMultiplayer.h"
#include <string>

namespace Platform
{
    FILE* OpenInternalFile(char* path, char* mode);

    typedef struct
    {
        int val;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
    } AndroidSemaphore;

    typedef struct
    {
        pthread_t ID;
        std::function<void()> Func;

    } AndroidThread;

    typedef pthread_mutex_t AndroidMutex;

    void* ThreadEntry(void* data)
    {
        AndroidThread* thread = (AndroidThread*)data;
        thread->Func();
        return NULL;
    }

    int InstanceID()
    {
        return 0;
    }

    std::string InstanceFileSuffix()
    {
        return "";
    }

    int GetConfigInt(ConfigEntry entry)
    {
        const int imgsizes[] = {0, 256, 512, 1024, 2048, 4096};

        switch (entry)
        {
#ifdef JIT_ENABLED
            case JIT_MaxBlockSize: return Config::JIT_MaxBlockSize;
#endif

            case DLDI_ImageSize: return imgsizes[Config::DLDISize];

            case DSiSD_ImageSize: return imgsizes[Config::DSiSDSize];

            case Firm_Language: return Config::FirmwareLanguage;
            case Firm_BirthdayMonth: return Config::FirmwareBirthdayMonth;
            case Firm_BirthdayDay: return Config::FirmwareBirthdayDay;
            case Firm_Color: return Config::FirmwareFavouriteColour;

            case AudioBitrate: return Config::AudioBitrate;
        }

        return 0;
    }

    bool GetConfigBool(ConfigEntry entry)
    {
        switch (entry)
        {
#ifdef JIT_ENABLED
            case JIT_Enable: return Config::JIT_Enable != 0;
            case JIT_LiteralOptimizations: return Config::JIT_LiteralOptimisations != 0;
            case JIT_BranchOptimizations: return Config::JIT_BranchOptimisations != 0;
            case JIT_FastMemory: return Config::JIT_FastMemory != 0;
#endif

            case ExternalBIOSEnable: return Config::ExternalBIOSEnable != 0;

            case DLDI_Enable: return Config::DLDIEnable != 0;
            case DLDI_ReadOnly: return Config::DLDIReadOnly != 0;
            case DLDI_FolderSync: return Config::DLDIFolderSync != 0;

            case DSiSD_Enable: return Config::DSiSDEnable != 0;
            case DSiSD_ReadOnly: return Config::DSiSDReadOnly != 0;
            case DSiSD_FolderSync: return Config::DSiSDFolderSync != 0;

            case Firm_RandomizeMAC: return Config::RandomizeMAC != 0;
            case Firm_OverrideSettings: return Config::FirmwareOverrideSettings != 0;
        }

        return false;
    }

    std::string GetConfigString(ConfigEntry entry)
    {
        switch (entry)
        {
            case BIOS9Path: return Config::BIOS9Path;
            case BIOS7Path: return Config::BIOS7Path;
            case FirmwarePath: return Config::FirmwarePath;

            case DSi_BIOS9Path: return Config::DSiBIOS9Path;
            case DSi_BIOS7Path: return Config::DSiBIOS7Path;
            case DSi_FirmwarePath: return Config::DSiFirmwarePath;
            case DSi_NANDPath: return Config::DSiNANDPath;

            case DLDI_ImagePath: return Config::DLDISDPath;
            case DLDI_FolderPath: return Config::DLDIFolderPath;

            case DSiSD_ImagePath: return Config::DSiSDPath;
            case DSiSD_FolderPath: return Config::DSiSDFolderPath;

            case Firm_Username: return Config::FirmwareUsername;
            case Firm_Message: return Config::FirmwareMessage;
        }

        return "";
    }

    bool GetConfigArray(ConfigEntry entry, void* data)
    {
        switch (entry)
        {
            case Firm_MAC:
            {
                std::string& mac_in = Config::FirmwareMAC;
                u8* mac_out = (u8*)data;

                int o = 0;
                u8 tmp = 0;
                for (int i = 0; i < 18; i++)
                {
                    char c = mac_in[i];
                    if (c == '\0') break;

                    int n;
                    if      (c >= '0' && c <= '9') n = c - '0';
                    else if (c >= 'a' && c <= 'f') n = c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F') n = c - 'A' + 10;
                    else continue;

                    if (!(o & 1))
                        tmp = n;
                    else
                        mac_out[o >> 1] = n | (tmp << 4);

                    o++;
                    if (o >= 12) return true;
                }
            }
                return false;
        }

        return false;
    }

    FILE* OpenFile(std::string path, std::string mode, bool mustexist)
    {
        if (path.empty())
        {
            return nullptr;
        }

        // If it's a standard absolute file path, open it a simple file. If not, delegate to the file handler
        if (path[0] == '/')
        {
            if (mustexist)
            {
                if (access(path.c_str(), F_OK) == 0)
                    return fopen(path.c_str(), mode.c_str());
                else
                    return nullptr;
            }
            else
                return fopen(path.c_str(), mode.c_str());
        }
        else
        {
            return MelonDSAndroid::fileHandler->open(path.c_str(), mode.c_str());
        }
    }

    FILE* OpenLocalFile(std::string path, std::string mode)
    {
        if (path.empty())
            return nullptr;

        // Always open file as absolute
        return OpenFile(path, mode, mode[0] == 'r');
    }

    FILE* OpenDataFile(const char* path)
    {
        return android_fopen(MelonDSAndroid::assetManager, path, "rb");
    }

    FILE* OpenInternalFile(std::string path, std::string mode)
    {
        std::string fullFilePath = MelonDSAndroid::internalFilesDir + "/" + path;
        return OpenFile(fullFilePath, mode, mode[0] == 'r');
    }

    Thread* Thread_Create(std::function<void()> func)
    {
        AndroidThread* data = new AndroidThread;
        data->Func = func;
        pthread_create(&data->ID, nullptr, ThreadEntry, data);
        return (Thread*) data;
    }

    void Thread_Free(Thread* thread)
    {
        delete (AndroidThread*)thread;
    }

    void Thread_Wait(Thread* thread)
    {
        pthread_t pthread = ((AndroidThread*) thread)->ID;
        pthread_join(pthread, NULL);
    }

    Semaphore* Semaphore_Create()
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) malloc(sizeof(AndroidSemaphore));
        pthread_mutex_init(&semaphore->mutex, NULL);
        pthread_cond_init(&semaphore->cond, NULL);
        semaphore->val = 0;
        return (Semaphore*) semaphore;
    }

    void Semaphore_Free(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_destroy(&semaphore->mutex);
        pthread_cond_destroy(&semaphore->cond);
        free(semaphore);
    }

    void Semaphore_Reset(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        semaphore->val = 0;
        pthread_cond_broadcast(&semaphore->cond);
        pthread_mutex_unlock(&semaphore->mutex);
    }

    void Semaphore_Wait(Semaphore* sema)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        while (semaphore->val == 0)
            pthread_cond_wait(&semaphore->cond, &semaphore->mutex);

        semaphore->val--;
        pthread_mutex_unlock(&semaphore->mutex);
    }

    void Semaphore_Post(Semaphore* sema, int count)
    {
        AndroidSemaphore* semaphore = (AndroidSemaphore*) sema;
        pthread_mutex_lock(&semaphore->mutex);
        semaphore->val += count;
        pthread_cond_broadcast(&semaphore->cond);
        pthread_mutex_unlock(&semaphore->mutex);
    }

    Mutex* Mutex_Create() {
        AndroidMutex* mutex = (AndroidMutex*) malloc(sizeof(AndroidMutex));
        pthread_mutex_init(mutex, nullptr);
        return (Mutex*) mutex;
    }

    void Mutex_Free(Mutex* mutex) {
        pthread_mutex_destroy((AndroidMutex*) mutex);
        free((AndroidMutex*) mutex);
    }

    void Mutex_Lock(Mutex* mutex) {
        pthread_mutex_lock((AndroidMutex*) mutex);
    }

    void Mutex_Unlock(Mutex* mutex) {
        pthread_mutex_unlock((AndroidMutex*) mutex);
    }

    bool Mutex_TryLock(Mutex* mutex) {
        return pthread_mutex_trylock((AndroidMutex*) mutex) == 0;
    }

    void WriteNDSSave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen)
    {
        if (ROMManager::NDSSave)
            ROMManager::NDSSave->RequestFlush(savedata, savelen, writeoffset, writelen);
    }

    void WriteGBASave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen)
    {
        if (ROMManager::GBASave)
            ROMManager::GBASave->RequestFlush(savedata, savelen, writeoffset, writelen);
    }

    bool MP_Init()
    {
        return LocalMultiplayer::Init();
    }

    void MP_DeInit()
    {
        LocalMultiplayer::DeInit();
        return;
    }

    void MP_Begin()
    {
        LocalMultiplayer::Begin();
    }

    void MP_End()
    {
        LocalMultiplayer::End();
    }

    int MP_SendPacket(u8* data, int len, u64 timestamp)
    {
        return LocalMultiplayer::SendPacket(data, len, timestamp);
    }

    int MP_RecvPacket(u8* data, u64* timestamp)
    {
        return LocalMultiplayer::RecvPacket(data, timestamp);
    }

    int MP_SendCmd(u8* data, int len, u64 timestamp)
    {
        return LocalMultiplayer::SendCmd(data, len, timestamp);
    }

    int MP_SendReply(u8* data, int len, u64 timestamp, u16 aid)
    {
        return LocalMultiplayer::SendReply(data, len, timestamp, aid);
    }

    int MP_SendAck(u8* data, int len, u64 timestamp)
    {
        return LocalMultiplayer::SendAck(data, len, timestamp);
    }

    int MP_RecvHostPacket(u8* data, u64* timestamp)
    {
        return LocalMultiplayer::RecvHostPacket(data, timestamp);
    }

    u16 MP_RecvReplies(u8* data, u64 timestamp, u16 aidmask)
    {
        return LocalMultiplayer::RecvReplies(data, timestamp, aidmask);
    }

    bool LAN_Init()
    {
        if (Config::DirectLAN)
        {
            if (!LAN_PCap::Init(true))
                return false;
        }
        else
        {
            if (!LAN_Socket::Init())
                return false;
        }

        return true;
    }

    void LAN_DeInit()
    {
        // checkme. blarg
        //if (Config::DirectLAN)
        //    LAN_PCap::DeInit();
        //else
        //    LAN_Socket::DeInit();
        LAN_PCap::DeInit();
        LAN_Socket::DeInit();
    }

    int LAN_SendPacket(u8* data, int len)
    {
        if (Config::DirectLAN)
            return LAN_PCap::SendPacket(data, len);
        else
            return LAN_Socket::SendPacket(data, len);
    }

    int LAN_RecvPacket(u8* data)
    {
        if (Config::DirectLAN)
            return LAN_PCap::RecvPacket(data);
        else
            return LAN_Socket::RecvPacket(data);
    }

    void Sleep(u64 usecs)
    {
        usleep(usecs);
    }

    void StopEmu()
    {
    }

    void Camera_Start(int num)
    {
        MelonDSAndroid::cameraHandler->startCamera(num);
    }

    void Camera_Stop(int num)
    {
        MelonDSAndroid::cameraHandler->stopCamera(num);
    }

    void Camera_CaptureFrame(int num, u32* frame, int width, int height, bool yuv)
    {
        MelonDSAndroid::cameraHandler->captureFrame(num, frame, width, height, yuv);
    }
}
