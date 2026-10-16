/*
    Copyright 2016-2025 melonDS team

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Platform.h"
#include "Core.h"

#ifdef __WIN32__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <wchar.h>
#include <windows.h>
#define fseek _fseeki64
#define ftell _ftelli64
#else
#include <dlfcn.h>
#endif // __WIN32__

namespace melonDS::Platform
{

#ifdef __WIN32__
constexpr char PathSeparator = '\\';
#else
constexpr char PathSeparator = '/';
#endif

void SignalStop(StopReason reason, void* userdata)
{
    // The environment callback may only be used from the frontend's own thread,
    // and this can be reached from deep inside the emulated console's execution.
    // retro_run picks the flag up on the next boundary and shuts down there.
    Libretro::State.Stopped = true;

    const char* description;
    switch (reason)
    {
        case StopReason::External: description = "stopped by the frontend"; break;
        case StopReason::GBAModeNotSupported: description = "GBA mode is not supported"; break;
        case StopReason::BadExceptionRegion: description = "bad exception region"; break;
        case StopReason::PowerOff: description = "console powered off"; break;
        default: description = "unknown reason"; break;
    }

    Libretro::Log(RETRO_LOG_INFO, "Emulation stopped: %s\n", description);
}


#ifdef __WIN32__
static std::wstring UTF8ToUTF16(const std::string& str)
{
    if (str.empty())
        return std::wstring();

    int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), nullptr, 0);
    if (length <= 0)
        return std::wstring();

    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], length);
    return result;
}
#endif // __WIN32__

// Existence has to be probed without std::filesystem: on Windows a path built
// from a narrow string is reinterpreted in the active ANSI code page, which
// mangles the UTF-8 paths RetroArch hands us.
static bool FileIsPresent(const std::string& path)
{
#ifdef __WIN32__
    std::wstring widePath = UTF8ToUTF16(path);
    if (widePath.empty())
        return false;

    FILE* file = _wfopen(widePath.c_str(), L"rb");
#else
    FILE* file = fopen(path.c_str(), "rb");
#endif

    if (!file)
        return false;

    fclose(file);
    return true;
}

static void DeleteFileByPath(const std::string& path)
{
#ifdef __WIN32__
    std::wstring widePath = UTF8ToUTF16(path);
    if (!widePath.empty())
        _wremove(widePath.c_str());
#else
    remove(path.c_str());
#endif
}

// A leading separator, or a drive letter on Windows, is enough to tell an
// absolute path apart; inspecting only ASCII keeps this safe on UTF-8 input.
static bool IsAbsolutePath(const std::string& path)
{
    if (path.empty())
        return false;

#ifdef __WIN32__
    if (path.length() >= 2 && path[1] == ':')
        return true;
#endif

    return path[0] == '/' || path[0] == '\\';
}

constexpr char AccessMode(FileMode mode, bool file_exists)
{
    if (mode & FileMode::Append)
        return  'a';

    if (!(mode & FileMode::Write))
        // If we're only opening the file for reading...
        return 'r';

    if (mode & (FileMode::NoCreate))
        // If we're not allowed to create a new file...
        return 'r'; // Open in "r+" mode (IsExtended will add the "+")

    if ((mode & FileMode::Preserve) && file_exists)
        // If we're not allowed to overwrite a file that already exists...
        return 'r'; // Open in "r+" mode (IsExtended will add the "+")

    return 'w';
}

constexpr bool IsExtended(FileMode mode)
{
    // fopen's "+" flag always opens the file for read/write
    return (mode & FileMode::ReadWrite) == FileMode::ReadWrite;
}

static std::string GetModeString(FileMode mode, bool file_exists)
{
    std::string modeString;

    modeString += AccessMode(mode, file_exists);

    if (IsExtended(mode))
        modeString += '+';

    if (!(mode & FileMode::Text))
        modeString += 'b';

    return modeString;
}

FileHandle* OpenFile(const std::string& path, FileMode mode)
{
    if ((mode & (FileMode::ReadWrite | FileMode::Append)) == FileMode::None)
    { // If we aren't reading or writing, then we can't open the file
        Log(LogLevel::Error, "Attempted to open \"%s\" in neither read nor write mode (FileMode 0x%x)\n", path.c_str(), mode);
        return nullptr;
    }

    std::string modeString = GetModeString(mode, FileIsPresent(path));

#ifdef __WIN32__
    std::wstring widePath = UTF8ToUTF16(path);
    std::wstring wideMode = UTF8ToUTF16(modeString);
    FILE* file = nullptr;
    if (!widePath.empty() && !wideMode.empty())
        file = _wfopen(widePath.c_str(), wideMode.c_str());
#else
    FILE* file = fopen(path.c_str(), modeString.c_str());
#endif

    if (file)
    {
        Log(LogLevel::Debug, "Opened \"%s\" with FileMode 0x%x (effective mode \"%s\")\n", path.c_str(), mode, modeString.c_str());
        return reinterpret_cast<FileHandle *>(file);
    }

    Log(LogLevel::Warn, "Failed to open \"%s\" with FileMode 0x%x (effective mode \"%s\")\n", path.c_str(), mode, modeString.c_str());
    return nullptr;
}

std::string GetLocalFilePath(const std::string& filename)
{
    if (IsAbsolutePath(filename))
        return filename;

    // Relative names are the BIOS and firmware images the core asks for, and
    // RetroArch keeps those in the system directory.
    if (Libretro::SystemDirectory.empty())
        return filename;

    std::string fullpath = Libretro::SystemDirectory;
    if (fullpath.back() != '/' && fullpath.back() != '\\')
        fullpath += PathSeparator;

    return fullpath + filename;
}

FileHandle* OpenLocalFile(const std::string& path, FileMode mode)
{
    return OpenFile(GetLocalFilePath(path), mode);
}

bool CloseFile(FileHandle* file)
{
    return fclose(reinterpret_cast<FILE *>(file)) == 0;
}

bool IsEndOfFile(FileHandle* file)
{
    return feof(reinterpret_cast<FILE *>(file)) != 0;
}

bool FileReadLine(char* str, int count, FileHandle* file)
{
    return fgets(str, count, reinterpret_cast<FILE *>(file)) != nullptr;
}

bool FileExists(const std::string& name)
{
    FileHandle* f = OpenFile(name, FileMode::Read);
    if (!f) return false;
    CloseFile(f);
    return true;
}

bool LocalFileExists(const std::string& name)
{
    FileHandle* f = OpenLocalFile(name, FileMode::Read);
    if (!f) return false;
    CloseFile(f);
    return true;
}

bool CheckFileWritable(const std::string& filepath)
{
    FileHandle* file = OpenFile(filepath, FileMode::Read);

    if (file)
    {
        // if the file exists, check if it can be opened for writing.
        CloseFile(file);
        file = OpenFile(filepath, FileMode::Append);
        if (file)
        {
            CloseFile(file);
            return true;
        }
        else return false;
    }
    else
    {
        // if the file does not exist, creating it is the only portable way to
        // learn whether the directory is writable, so remove it again instead
        // of leaving a stray empty file behind.
        file = OpenFile(filepath, FileMode::Write);
        if (file)
        {
            CloseFile(file);
            DeleteFileByPath(filepath);
            return true;
        }
        else return false;
    }
}

bool CheckLocalFileWritable(const std::string& filepath)
{
    return CheckFileWritable(GetLocalFilePath(filepath));
}

bool FileSeek(FileHandle* file, s64 offset, FileSeekOrigin origin)
{
    int stdorigin = SEEK_SET;
    switch (origin)
    {
        case FileSeekOrigin::Start: stdorigin = SEEK_SET; break;
        case FileSeekOrigin::Current: stdorigin = SEEK_CUR; break;
        case FileSeekOrigin::End: stdorigin = SEEK_END; break;
    }

    return fseek(reinterpret_cast<FILE *>(file), offset, stdorigin) == 0;
}

void FileRewind(FileHandle* file)
{
    rewind(reinterpret_cast<FILE *>(file));
}

u64 FilePosition(FileHandle* file)
{
    return ftell(reinterpret_cast<FILE *>(file));
}

u64 FileRead(void* data, u64 size, u64 count, FileHandle* file)
{
    return fread(data, size, count, reinterpret_cast<FILE *>(file));
}

bool FileFlush(FileHandle* file)
{
    return fflush(reinterpret_cast<FILE *>(file)) == 0;
}

u64 FileWrite(const void* data, u64 size, u64 count, FileHandle* file)
{
    return fwrite(data, size, count, reinterpret_cast<FILE *>(file));
}

u64 FileWriteFormatted(FileHandle* file, const char* fmt, ...)
{
    if (fmt == nullptr)
        return 0;

    va_list args;
    va_start(args, fmt);
    u64 ret = vfprintf(reinterpret_cast<FILE *>(file), fmt, args);
    va_end(args);
    return ret;
}

u64 FileLength(FileHandle* file)
{
    FILE* stdfile = reinterpret_cast<FILE *>(file);
    s64 pos = ftell(stdfile);
    fseek(stdfile, 0, SEEK_END);
    s64 len = ftell(stdfile);
    fseek(stdfile, pos, SEEK_SET);
    return len;
}

std::vector<std::string> ContentsOfFolder(const std::string& path, bool includeFolders, bool includeFiles)
{
    std::vector<std::string> contents;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            const auto& p = entry.path();
            if ((includeFiles && std::filesystem::is_regular_file(entry)) ||
                (includeFolders && std::filesystem::is_directory(entry))) {
                contents.push_back(p.filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Log(LogLevel::Warn, "Failed to list contents of folder \"%s\"\n", path.c_str());
    } catch (const std::exception& e) {
        Log(LogLevel::Warn, "Failed to list contents of folder \"%s\"\n", path.c_str());
    }

    return contents;
}

void Log(LogLevel level, const char* fmt, ...)
{
    if (fmt == nullptr)
        return;

    retro_log_level retroLevel;
    switch (level)
    {
        case LogLevel::Debug: retroLevel = RETRO_LOG_DEBUG; break;
        case LogLevel::Warn: retroLevel = RETRO_LOG_WARN; break;
        case LogLevel::Error: retroLevel = RETRO_LOG_ERROR; break;
        default: retroLevel = RETRO_LOG_INFO; break;
    }

    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    // Libretro::Log is variadic as well, so the already-expanded text has to be
    // passed through a literal format string to avoid a second expansion pass.
    Libretro::Log(retroLevel, "%s", message);
}

struct Thread
{
    std::thread Handle;
};

Thread* Thread_Create(std::function<void()> func)
{
    Thread* thread = new Thread();
    thread->Handle = std::thread(std::move(func));
    return thread;
}

void Thread_Free(Thread* thread)
{
    if (!thread)
        return;

    // Joining rather than killing the thread: a detached or terminated worker
    // would leave the emulator state it was mutating half-written.
    if (thread->Handle.joinable())
        thread->Handle.join();

    delete thread;
}

void Thread_Wait(Thread* thread)
{
    if (thread && thread->Handle.joinable())
        thread->Handle.join();
}

// C++17 has no counting semaphore, so it is built from a mutex and a condvar.
struct Semaphore
{
    std::mutex Lock;
    std::condition_variable Cond;
    int Count = 0;
};

Semaphore* Semaphore_Create()
{
    return new Semaphore();
}

void Semaphore_Free(Semaphore* sema)
{
    delete sema;
}

void Semaphore_Reset(Semaphore* sema)
{
    std::lock_guard<std::mutex> lock(sema->Lock);
    sema->Count = 0;
}

void Semaphore_Wait(Semaphore* sema)
{
    std::unique_lock<std::mutex> lock(sema->Lock);
    sema->Cond.wait(lock, [sema]() { return sema->Count > 0; });
    sema->Count--;
}

bool Semaphore_TryWait(Semaphore* sema, int timeout_ms)
{
    std::unique_lock<std::mutex> lock(sema->Lock);

    if (timeout_ms <= 0)
    {
        if (sema->Count <= 0)
            return false;
    }
    else if (!sema->Cond.wait_for(lock, std::chrono::milliseconds(timeout_ms), [sema]() { return sema->Count > 0; }))
        return false;

    sema->Count--;
    return true;
}

void Semaphore_Post(Semaphore* sema, int count)
{
    {
        std::lock_guard<std::mutex> lock(sema->Lock);
        sema->Count += count;
    }

    // Several waiters may become runnable at once when count > 1.
    sema->Cond.notify_all();
}

struct Mutex
{
    std::mutex Handle;
};

Mutex* Mutex_Create()
{
    return new Mutex();
}

void Mutex_Free(Mutex* mutex)
{
    delete mutex;
}

void Mutex_Lock(Mutex* mutex)
{
    mutex->Handle.lock();
}

void Mutex_Unlock(Mutex* mutex)
{
    mutex->Handle.unlock();
}

bool Mutex_TryLock(Mutex* mutex)
{
    return mutex->Handle.try_lock();
}

void Sleep(u64 usecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}

// Both counters share one baseline so a millisecond reading can never disagree
// with a microsecond reading taken right next to it.
static std::chrono::steady_clock::duration ElapsedSinceStart()
{
    static const std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    return std::chrono::steady_clock::now() - startTime;
}

u64 GetMSCount()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(ElapsedSinceStart()).count();
}

u64 GetUSCount()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(ElapsedSinceStart()).count();
}


void WriteNDSSave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen, void* userdata)
{
    // RetroArch owns the .srm file and reads the save through
    // retro_get_memory_data, so the core must not write it itself.
    Libretro::State.SaveDirty = true;
}

void WriteGBASave(const u8* savedata, u32 savelen, u32 writeoffset, u32 writelen, void* userdata)
{
    Libretro::State.SaveDirty = true;
}

void WriteFirmware(const Firmware& firmware, u32 writeoffset, u32 writelen, void* userdata)
{
    // The libretro core never persists firmware changes: the frontend owns the
    // system directory and the core is not allowed to modify what is in it.
}

void WriteDateTime(int year, int month, int day, int hour, int minute, int second, void* userdata)
{
    // Keeping the RTC offset would need frontend-side config storage, which
    // libretro does not offer.
}


// Local multiplayer is not supported: the libretro target links neither the
// MPInterface backends nor src/net, so every packet is silently dropped.

void MP_Begin(void* userdata)
{
}

void MP_End(void* userdata)
{
}

int MP_SendPacket(u8* data, int len, u64 timestamp, void* userdata)
{
    return 0;
}

int MP_RecvPacket(u8* data, u64* timestamp, void* userdata)
{
    return 0;
}

int MP_SendCmd(u8* data, int len, u64 timestamp, void* userdata)
{
    return 0;
}

int MP_SendReply(u8* data, int len, u64 timestamp, u16 aid, void* userdata)
{
    return 0;
}

int MP_SendAck(u8* data, int len, u64 timestamp, void* userdata)
{
    return 0;
}

int MP_RecvHostPacket(u8* data, u64* timestamp, void* userdata)
{
    return 0;
}

u16 MP_RecvReplies(u8* data, u64 timestamp, u16 aidmask, void* userdata)
{
    return 0;
}


int Net_SendPacket(u8* data, int len, void* userdata)
{
    return 0;
}

int Net_RecvPacket(u8* data, void* userdata)
{
    return 0;
}


void Camera_Start(int num, void* userdata)
{
}

void Camera_Stop(int num, void* userdata)
{
}

void Camera_CaptureFrame(int num, u32* frame, int width, int height, bool yuv, void* userdata)
{
    // No camera is exposed to the core, so hand back a black frame. YUYV packs
    // two pixels per word, hence the halved length, and its neutral black is
    // luma 0 with both chroma components sitting at their 0x80 midpoint.
    u32 value = yuv ? 0x80008000 : 0x00000000;
    int length = width * height;
    if (yuv) length /= 2;

    for (int i = 0; i < length; i++)
        frame[i] = value;
}


void Mic_Start(void* userdata)
{
}

void Mic_Stop(void* userdata)
{
}

int Mic_ReadInput(s16* data, int maxlength, void* userdata)
{
    // Reporting a zero-length read feeds silence; the caller handles that.
    return 0;
}


// The libretro target does not link libfaad, so DSi DSP HLE AAC decoding is
// unavailable and every entry point fails gracefully.

AACDecoder* AAC_Init()
{
    return nullptr;
}

void AAC_DeInit(AACDecoder* dec)
{
}

bool AAC_Configure(AACDecoder* dec, int frequency, int channels)
{
    return false;
}

bool AAC_DecodeFrame(AACDecoder* dec, const void* input, int inputlen, void* output, int outputlen)
{
    return false;
}


bool Addon_KeyDown(KeyType type, void* userdata)
{
    return false;
}

void Addon_RumbleStart(u32 len, void* userdata)
{
}

void Addon_RumbleStop(void* userdata)
{
}

float Addon_MotionQuery(MotionQueryType type, void* userdata)
{
    return 0.0f;
}

DynamicLibrary* DynamicLibrary_Load(const char* lib)
{
#ifdef __WIN32__
    return reinterpret_cast<DynamicLibrary*>(LoadLibraryA(lib));
#else
    return reinterpret_cast<DynamicLibrary*>(dlopen(lib, RTLD_NOW | RTLD_LOCAL));
#endif
}

void DynamicLibrary_Unload(DynamicLibrary* lib)
{
    if (!lib)
        return;

#ifdef __WIN32__
    FreeLibrary(reinterpret_cast<HMODULE>(lib));
#else
    dlclose(reinterpret_cast<void*>(lib));
#endif
}

void* DynamicLibrary_LoadFunction(DynamicLibrary* lib, const char* name)
{
    if (!lib)
        return nullptr;

#ifdef __WIN32__
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(reinterpret_cast<void*>(lib), name);
#endif
}

}
