#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true

#define DEBUG_MODE_ENABLED false
#define DEBUG_LOG_FILE_ENABLED false

#define RAM_SEARCH_ENABLED true
#define RAM_SEARCH_SIZE 8
#define RAM_SEARCH_LIMIT_MIN 0
#define RAM_SEARCH_LIMIT_MAX 0x3FFFFF

#if RAM_SEARCH_SIZE == 32
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read32(addr)
#elif RAM_SEARCH_SIZE == 16
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read16(addr)
#else
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read8(addr)
#endif

#include "NDS.h"

#include "OpenGLSupport.h"

namespace Plugins
{
using namespace melonDS;

struct CutsceneEntry
{
    char DsName[12];
    char Name[40];
    int usAddress;
    int euAddress;
    int jpAddress;
};

class Plugin
{
public:
    virtual ~Plugin() { };

    u32 GameCode = 0;
    static bool isCart(u32 gameCode) {return true;};

    virtual void setNds(melonDS::NDS* Nds) = 0;

    virtual std::string assetsFolder() = 0;

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual const char* gpu3DOpenGL_VS_Z() { return nullptr; };

    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) { };
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) { };
    virtual void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags) { };
    virtual void gpu3DOpenGL_VS_Z_updateVariables(u32 flags) { };

    virtual void onLoadState() { };

    virtual u32 applyHotkeyToInputMask(u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) = 0;
    virtual void applyTouchKeyMask(u32 TouchKeyMask) = 0;

    virtual bool ShouldTerminateIngameCutscene() = 0;
    virtual bool StoppedIngameCutscene() = 0;
    virtual bool ShouldStartReplacementCutscene() = 0;
    virtual bool StartedReplacementCutscene() = 0;
    virtual bool RunningReplacementCutscene() = 0;
    virtual bool ShouldStopReplacementCutscene() = 0;
    virtual bool ShouldReturnToGameAfterCutscene() = 0;
    virtual bool ShouldUnmuteAfterCutscene() = 0;
    virtual CutsceneEntry* CurrentCutscene() = 0;
    virtual std::string CutsceneFilePath(CutsceneEntry* cutscene) = 0;
    virtual void onIngameCutsceneIdentified(CutsceneEntry* cutscene) = 0;
    virtual void onTerminateIngameCutscene() = 0;
    virtual void onReturnToGameAfterCutscene() = 0;
    virtual void onReplacementCutsceneStarted() = 0;
    virtual void onReplacementCutsceneEnd() = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldRenderFrame() = 0;

    virtual bool refreshGameScene() = 0;

    virtual void setAspectRatio(float aspectRatio) = 0;

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
        if (HotkeyPress & (1 << 12)) { // HK_PowerButton (reset RAM search)
            printf("Resetting RAM search\n");
            for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                u32 addr = (0x02000000 | index);
                u32 newVal = RAM_SEARCH_READ(nds, addr);
                MainRAMState[index] = true;
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
        if (HotkeyPress & (1 << 12) || HotkeyPress & (1 << 13) || HotkeyPress & (1 << 14)) {
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
                else if (total < 100000) {
                    u32 firstAddr = 0;
                    for (u32 index = (limitMin == 0 ? byteSize : limitMin); index < limitMax; index += byteSize) {
                        u32 addr = (0x02000000 | index);
                        if (MainRAMState[index]) {
                            if (firstAddr == 0) {
                                firstAddr = addr;
                            }
                        }
                        else {
                            int validDistance = 0x200;
                            if (firstAddr != 0 && firstAddr < (addr - byteSize*(validDistance - 1))) {
                                printf("0x%08x - 0x%08x\n", firstAddr, addr - byteSize*validDistance);
                                firstAddr = 0;
                            }
                        }
                    }
                    if (firstAddr != 0) {
                        printf("0x%08x - 0x%08x\n", firstAddr, firstAddr);
                    }
                    printf("\n");
                }
            }
            printf("Addresses matching the search: %d\n", total);
        }
    }
};
}

#endif
