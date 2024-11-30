#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true

#define SHOW_GAME_SCENE false
#define DEBUG_MODE_ENABLED false
#define DEBUG_LOG_FILE_ENABLED false

#define RAM_SEARCH_ENABLED true
#define RAM_SEARCH_SIZE 32
#define RAM_SEARCH_LIMIT_MIN 0
#define RAM_SEARCH_LIMIT_MAX 0x3FFFFF
#define RAM_SEARCH_INTERVAL_MARGIN 0x050

// #define RAM_SEARCH_EXACT_VALUE     0x05B07E00
// #define RAM_SEARCH_EXACT_VALUE_MIN 0x05B07E00
// #define RAM_SEARCH_EXACT_VALUE_MAX 0x05BEE334


// #define RAM_SEARCH_LIMIT_MIN 0x04f6c5 - 0x1F00
// #define RAM_SEARCH_LIMIT_MAX 0x04f6c5 + 0x1F00
// #define RAM_SEARCH_LIMIT_MAX 0x19FFFF

// WARNING: THE MACRO BELOW CAN ONLY BE USED ALONGSIDE RAM_SEARCH_EXACT_VALUE* MACROS,
// OTHERWISE IT WILL DO NOTHING BUT MAKE SEARCH IMPOSSIBLE, AND DECREASE THE FRAMERATE
#define RAM_SEARCH_EVERY_SINGLE_FRAME false

#if RAM_SEARCH_SIZE == 32
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read32(addr)
#elif RAM_SEARCH_SIZE == 16
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read16(addr)
#else
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read8(addr)
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80000000 ? '1' : '0'), \
  ((byte) & 0x40000000 ? '1' : '0'), \
  ((byte) & 0x20000000 ? '1' : '0'), \
  ((byte) & 0x10000000 ? '1' : '0'), \
  ((byte) & 0x08000000 ? '1' : '0'), \
  ((byte) & 0x04000000 ? '1' : '0'), \
  ((byte) & 0x02000000 ? '1' : '0'), \
  ((byte) & 0x01000000 ? '1' : '0'), \
  ((byte) & 0x00800000 ? '1' : '0'), \
  ((byte) & 0x00400000 ? '1' : '0'), \
  ((byte) & 0x00200000 ? '1' : '0'), \
  ((byte) & 0x00100000 ? '1' : '0'), \
  ((byte) & 0x00080000 ? '1' : '0'), \
  ((byte) & 0x00040000 ? '1' : '0'), \
  ((byte) & 0x00020000 ? '1' : '0'), \
  ((byte) & 0x00010000 ? '1' : '0'), \
  ((byte) & 0x00008000 ? '1' : '0'), \
  ((byte) & 0x00004000 ? '1' : '0'), \
  ((byte) & 0x00002000 ? '1' : '0'), \
  ((byte) & 0x00001000 ? '1' : '0'), \
  ((byte) & 0x00000800 ? '1' : '0'), \
  ((byte) & 0x00000400 ? '1' : '0'), \
  ((byte) & 0x00000200 ? '1' : '0'), \
  ((byte) & 0x00000100 ? '1' : '0'), \
  ((byte) & 0x00000080 ? '1' : '0'), \
  ((byte) & 0x00000040 ? '1' : '0'), \
  ((byte) & 0x00000020 ? '1' : '0'), \
  ((byte) & 0x00000010 ? '1' : '0'), \
  ((byte) & 0x00000008 ? '1' : '0'), \
  ((byte) & 0x00000004 ? '1' : '0'), \
  ((byte) & 0x00000002 ? '1' : '0'), \
  ((byte) & 0x00000001 ? '1' : '0') 

#ifndef __APPLE__

#define PRINT_AS_8_BIT_HEX(ADDRESS) printf("0x%08x: 0x%02x\n", ADDRESS, nds->ARM7Read8(ADDRESS))
#define PRINT_AS_8_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read8(ADDRESS)))

#define PRINT_AS_16_BIT_HEX(ADDRESS) printf("0x%08x: 0x%04x\n", ADDRESS, nds->ARM7Read16(ADDRESS))
#define PRINT_AS_16_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read16(ADDRESS)))

#define PRINT_AS_32_BIT_HEX(ADDRESS) printf("0x%08x: 0x%08x\n", ADDRESS, nds->ARM7Read32(ADDRESS))
#define PRINT_AS_32_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read32(ADDRESS)))

#else

#define PRINT_AS_8_BIT_HEX(ADDRESS)
#define PRINT_AS_8_BIT_BIN(ADDRESS)

#define PRINT_AS_16_BIT_HEX(ADDRESS)
#define PRINT_AS_16_BIT_BIN(ADDRESS)

#define PRINT_AS_32_BIT_HEX(ADDRESS)
#define PRINT_AS_32_BIT_BIN(ADDRESS)

#endif

#include <functional>

#include "../NDS.h"

#include "../OpenGLSupport.h"

namespace Plugins
{
using namespace melonDS;

struct CutsceneEntry
{
    char DsName[12];
    char MmName[12];
    char Name[40];
    int usAddress;
    int euAddress;
    int jpAddress;
    int dsScreensState;
};

class Plugin
{
public:
    virtual ~Plugin() { };

    virtual u32 getGameCode() = 0;
    static bool isCart(u32 gameCode) {return true;};

    virtual void setNds(melonDS::NDS* Nds) = 0;
    virtual void onLoadROM() = 0;

    virtual std::string assetsFolder() = 0;

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual const char* gpu3DOpenGL_VS_Z() { return nullptr; };

    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) { };
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) { };
    virtual void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags) { };
    virtual void gpu3DOpenGL_VS_Z_updateVariables(u32 flags) { };

    virtual void onLoadState() { };
    virtual bool togglePause() {return false;};

    virtual void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) = 0;
    virtual bool applyTouchKeyMask(u32 TouchKeyMask) = 0;

    virtual bool ShouldTerminateIngameCutscene() = 0;
    virtual bool StoppedIngameCutscene() = 0;
    virtual bool ShouldStartReplacementCutscene() = 0;
    virtual bool StartedReplacementCutscene() = 0;
    virtual bool RunningReplacementCutscene() = 0;
    virtual bool ShouldPauseReplacementCutscene() = 0;
    virtual bool ShouldUnpauseReplacementCutscene() = 0;
    virtual bool ShouldStopReplacementCutscene() = 0;
    virtual bool ShouldReturnToGameAfterCutscene() = 0;
    virtual bool ShouldUnmuteAfterCutscene() = 0;
    virtual CutsceneEntry* CurrentCutscene() = 0;
    virtual std::string CutsceneFilePath(CutsceneEntry* cutscene) = 0;
    virtual std::string LocalizationFilePath(std::string language) = 0;
    virtual void onIngameCutsceneIdentified(CutsceneEntry* cutscene) = 0;
    virtual void onTerminateIngameCutscene() = 0;
    virtual void onReturnToGameAfterCutscene() = 0;
    virtual void onReplacementCutsceneStarted() = 0;
    virtual void onReplacementCutsceneEnd() = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldRenderFrame() = 0;

    virtual bool refreshGameScene() = 0;

    virtual void setAspectRatio(float aspectRatio) {}

    virtual void loadConfigs(std::function<std::string(std::string)> getStringConfig) {}

    void log(const char* log) {
        printf("%s\n", log);

        if (DEBUG_LOG_FILE_ENABLED) {
            std::string fileName = std::string("debug.log");
            Platform::FileHandle* logf = Platform::OpenFile(fileName, Platform::FileMode::Append);
            Platform::FileWrite(log, strlen(log), 1, logf);
            Platform::FileWrite("\n", 1, 1, logf);
            Platform::CloseFile(logf);
        }
    }

    u32 LastMainRAM[0xFFFFFF];
    bool MainRAMState[0xFFFFFF];

    void ramSearch(melonDS::NDS* nds, u32 HotkeyPress) {
#if !RAM_SEARCH_ENABLED
        return;
#endif

        int byteSize = RAM_SEARCH_SIZE/8;
        u32 limitMin = RAM_SEARCH_LIMIT_MIN;
        u32 limitMax = RAM_SEARCH_LIMIT_MAX;
        if (RAM_SEARCH_EVERY_SINGLE_FRAME || HotkeyPress & (1 << 12)) { // HK_PowerButton (reset RAM search)
            if (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
                printf("Resetting RAM search\n");
            }
            for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                u32 addr = (0x02000000 | index);
                u32 newVal = RAM_SEARCH_READ(nds, addr);
                MainRAMState[index] = true;
#ifdef RAM_SEARCH_EXACT_VALUE
                MainRAMState[index] = RAM_SEARCH_EXACT_VALUE == newVal;
#endif
#ifdef RAM_SEARCH_EXACT_VALUE_MIN
                if (newVal < RAM_SEARCH_EXACT_VALUE_MIN) {
                    MainRAMState[index] = false;
                }
#endif
#ifdef RAM_SEARCH_EXACT_VALUE_MAX
                if (newVal > RAM_SEARCH_EXACT_VALUE_MAX) {
                    MainRAMState[index] = false;
                }
#endif
                LastMainRAM[index] = newVal;
            }
        }
        if (HotkeyPress & (1 << 13)) { // HK_VolumeUp (filter RAM by equal values)
            printf("Filtering RAM by equal values\n");
            for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                u32 addr = (0x02000000 | index);
                u32 newVal = RAM_SEARCH_READ(nds, addr);
                MainRAMState[index] = MainRAMState[index] && (LastMainRAM[index] == newVal);
                LastMainRAM[index] = newVal;
            }
        }
        if (HotkeyPress & (1 << 14)) { // HK_VolumeDown (filter RAM by different values)
            printf("Filtering RAM by different values\n");
            for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                u32 addr = (0x02000000 | index);
                u32 newVal = RAM_SEARCH_READ(nds, addr);
                MainRAMState[index] = MainRAMState[index] && (LastMainRAM[index] != newVal);
                LastMainRAM[index] = newVal;
            }
        }
        if (RAM_SEARCH_EVERY_SINGLE_FRAME || HotkeyPress & (1 << 12) || HotkeyPress & (1 << 13) || HotkeyPress & (1 << 14)) {
            int total = 0;
            for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                if (MainRAMState[index]) {
                    total += 1;
                }
            }
            if (total > 0) {
                if (total < 50*(4/byteSize)) {
                    for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                        u32 addr = (0x02000000 | index);
                        if (MainRAMState[index]) {
                            printf("0x%08x: %d\n", addr, LastMainRAM[index]);
                        }
                    }
                    printf("\n");
                }
                else {
                    int validDistance = RAM_SEARCH_INTERVAL_MARGIN;
                    u32 firstAddr = 0;
                    u32 lastAddr = 0;
                    for (u32 index = (limitMin == 0 ? byteSize : limitMin); index < limitMax; index += byteSize) {
                        u32 addr = (0x02000000 | index);
                        if (MainRAMState[index]) {
                            if (firstAddr == 0) {
                                firstAddr = addr;
                                lastAddr = addr;
                            }
                            else {
                                lastAddr = addr;
                            }
                        }
                        else {
                            if (firstAddr != 0 && lastAddr < (addr - byteSize*validDistance)) {
                                if (firstAddr == lastAddr) {
                                    printf("0x%08x\n", firstAddr);
                                }
                                else {
                                    printf("0x%08x - 0x%08x\n", firstAddr, lastAddr);
                                }
                                firstAddr = 0;
                                lastAddr = 0;
                            }
                        }
                    }
                    if (firstAddr != 0) {
                        if (firstAddr == lastAddr) {
                            printf("0x%08x\n", firstAddr);
                        }
                        else {
                            printf("0x%08x - 0x%08x\n", firstAddr, lastAddr);
                        }
                    }
                    printf("\n");
                }
            }
            if (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
                printf("Addresses matching the search: %d\n", total);
            }
        }
    }
};
}

#endif
