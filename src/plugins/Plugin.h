#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true
#define REPLACEMENT_BGM_ENABLED true

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

#define CUTSCENE_SKIP_START_FRAMES_COUNT 40
#define CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT 40

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

    u32 GameCode = 0;
    u32 getGameCode() {
        return GameCode;
    };
    static bool isCart(u32 gameCode) {return true;};

    void setNds(melonDS::NDS* Nds) {nds = Nds;};
    virtual void onLoadROM() {};
    virtual void onLoadState() {};

    virtual std::string assetsFolder() = 0;
    virtual std::string tomlUniqueIdentifier() {return assetsFolder();};

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) {};
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) {};

    virtual const char* gpu3DOpenGLClassic_VS_Z() { return nullptr; };
    virtual void gpu3DOpenGLClassic_VS_Z_initVariables(GLuint prog, u32 flags) {};
    virtual void gpu3DOpenGLClassic_VS_Z_updateVariables(u32 flags) {};

    virtual void gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32* x, s32* y, s32 z, s32* rgb) {};

    bool togglePause()
    {
        if (_RunningReplacementCutscene) {
            if (_PausedReplacementCutscene) {
                _ShouldUnpauseReplacementCutscene = true;
            }
            else {
                _ShouldPauseReplacementCutscene = true;
            }
            return true;
        }
        if (_RunningReplacementBgmMusic) {
            if (_PausedReplacementBgmMusic) {
                _ShouldUnpauseReplacementBgmMusic = true;
            }
            else {
                _ShouldPauseReplacementBgmMusic = true;
            }
        }
        return false;
    }

    bool _superApplyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress)
    {
        ramSearch(nds, *HotkeyPress);

        if (_IsUnskippableCutscene)
        {
            *InputMask = 0xFFF;
            return false;
        }

        if (_RunningReplacementCutscene && !_PausedReplacementCutscene && (_SkipDsCutscene || (~(*InputMask)) & (1 << 3)) && _CanSkipHdCutscene) { // Start (skip HD cutscene)
            _SkipDsCutscene = true;
            if (!_ShouldTerminateIngameCutscene) { // can only skip after DS cutscene was skipped
                _SkipDsCutscene = false;
                _CanSkipHdCutscene = false;
                _ShouldStopReplacementCutscene = true;
                *InputMask |= (1<<3);
            }
            else {
                if (_StartPressCount == 0) {
                    bool requiresDoubleStart = (_CurrentCutscene->dsScreensState & 4) == 4;
                    if (requiresDoubleStart) {
                        _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT*2 + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT;
                    }
                    else {
                        _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT;
                    }
                }
            }
        }

        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene) {
            if (_StartPressCount > 0) {
                _StartPressCount--;

                bool requiresDoubleStart = (_CurrentCutscene->dsScreensState & 4) == 4;
                if (requiresDoubleStart) {
                    if (_StartPressCount < CUTSCENE_SKIP_START_FRAMES_COUNT || _StartPressCount > CUTSCENE_SKIP_START_FRAMES_COUNT + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT) {
                        *InputMask &= ~(1<<3); // Start (skip DS cutscene)
                    }
                }
                else {
                    *InputMask &= ~(1<<3); // Start (skip DS cutscene)
                }
            }
        }

        if ((*HotkeyPress) & (1 << 18)) { // HUD Toggle (HK_HUDToggle)
            hudToggle();
        }

        return true;
    }
    virtual void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) = 0;
    virtual bool applyTouchKeyMask(u32 TouchKeyMask) = 0;

    bool shouldExportTextures() {
        return ExportTextures;
    }
    bool shouldStartInFullscreen() {
        return FullscreenOnStartup;
    }

    void initCutsceneVariables() {
        _StartPressCount = 0;
        _ReplayLimitCount = 0;
        _CanSkipHdCutscene = false;
        _SkipDsCutscene = false;
        _IsUnskippableCutscene = false;
        _StartedReplacementCutscene = false;
        _RunningReplacementCutscene = false;
        _PausedReplacementCutscene = false;
        _ShouldTerminateIngameCutscene = false;
        _StoppedIngameCutscene = false;
        _ShouldStartReplacementCutscene = false;
        _ShouldPauseReplacementCutscene = false;
        _ShouldUnpauseReplacementCutscene = false;
        _ShouldStopReplacementCutscene = false;
        _ShouldReturnToGameAfterCutscene = false;
        _ShouldUnmuteAfterCutscene = false;
        _ShouldHideScreenForTransitions = false;
        _CurrentCutscene = nullptr;
        _NextCutscene = nullptr;
        _LastCutscene = nullptr;
    }
    bool ShouldTerminateIngameCutscene() {return _ShouldTerminateIngameCutscene;}
    bool StoppedIngameCutscene() {
        if (_StoppedIngameCutscene) {
            _StoppedIngameCutscene = false;
            return true;
        }
        return false;
    }
    bool ShouldStartReplacementCutscene() {
        if (_ShouldStartReplacementCutscene) {
            _ShouldStartReplacementCutscene = false;
            return true;
        }
        return false;
    }
    bool StartedReplacementCutscene() {
        if (_StartedReplacementCutscene) {
            _StartedReplacementCutscene = false;
            return true;
        }
        return false;
    }
    bool RunningReplacementCutscene() {return _RunningReplacementCutscene;}
    bool ShouldPauseReplacementCutscene() {
        if (_ShouldPauseReplacementCutscene) {
            _ShouldPauseReplacementCutscene = false;
            _PausedReplacementCutscene = true;
            return true;
        }
        return false;
    }
    bool ShouldUnpauseReplacementCutscene() {
        if (_ShouldUnpauseReplacementCutscene) {
            _ShouldUnpauseReplacementCutscene = false;
            _PausedReplacementCutscene = false;
            return true;
        }
        return false;
    }
    bool ShouldStopReplacementCutscene() {
        if (_ShouldStopReplacementCutscene) {
            _ShouldStopReplacementCutscene = false;
            return true;
        }
        return false;
    }
    bool ShouldReturnToGameAfterCutscene() {return _ShouldReturnToGameAfterCutscene;}
    bool ShouldUnmuteAfterCutscene() {
        if (_ShouldUnmuteAfterCutscene) {
            _ShouldUnmuteAfterCutscene = false;
            return true;
        }
        return false;
    }
    CutsceneEntry* CurrentCutscene() {return _CurrentCutscene;};

    virtual u32 detectTopScreenMobiCutsceneAddress() {return 0;};
    virtual u32 detectBottomScreenMobiCutsceneAddress() {return 0;};
    virtual CutsceneEntry* detectTopScreenMobiCutscene() {return nullptr;};
    virtual CutsceneEntry* detectBottomScreenMobiCutscene() {return nullptr;};
    CutsceneEntry* detectCutscene()
    {
        CutsceneEntry* cutscene1 = detectTopScreenMobiCutscene();
        CutsceneEntry* cutscene2 = detectBottomScreenMobiCutscene();

        if (cutscene1 == nullptr && cutscene2 != nullptr) {
            cutscene1 = cutscene2;
        }

        return cutscene1;
    }
    virtual bool isCutsceneGameScene() {return false;};
    virtual bool didMobiCutsceneEnded() {return !isCutsceneGameScene();};
    virtual bool canReturnToGameAfterReplacementCutscene() {return true;};

    void refreshCutscene()
    {
#if !REPLACEMENT_CUTSCENES_ENABLED
        return;
#endif

        bool isCutsceneScene = isCutsceneGameScene();
        CutsceneEntry* cutscene = detectCutscene();

        if (_ReplayLimitCount > 0) {
            _ReplayLimitCount--;
            if (cutscene != nullptr && cutscene->usAddress == _LastCutscene->usAddress) {
                cutscene = nullptr;
            }
        }

        
        if (cutscene != nullptr) {
            onIngameCutsceneIdentified(cutscene);
        }

        // Natural progression for all cutscenes
        if (_ShouldTerminateIngameCutscene && !_RunningReplacementCutscene && isCutsceneScene) {
            _ShouldStartReplacementCutscene = true;
        }

        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && didMobiCutsceneEnded()) {
            onTerminateIngameCutscene();
        }

        if (_ShouldReturnToGameAfterCutscene && canReturnToGameAfterReplacementCutscene()) {
            onReturnToGameAfterCutscene();
        }
    }

    virtual std::string replacementCutsceneFilePath(CutsceneEntry* cutscene) = 0;
    virtual std::string localizationFilePath(std::string language) = 0;
    virtual bool isUnskippableMobiCutscene(CutsceneEntry* cutscene) {return false;}

    void onIngameCutsceneIdentified(CutsceneEntry* cutscene) {
        if (_CurrentCutscene != nullptr && _CurrentCutscene->usAddress == cutscene->usAddress) {
            return;
        }

        std::string path = replacementCutsceneFilePath(cutscene);
        if (path == "") {
            return;
        }

        if (_CurrentCutscene != nullptr) {
            _NextCutscene = cutscene;
            return;
        }

        printf("Preparing to load cutscene: %s\n", cutscene->Name);

        _CanSkipHdCutscene = true;
        _CurrentCutscene = cutscene;
        _NextCutscene = nullptr;
        _ShouldTerminateIngameCutscene = true;
        _IsUnskippableCutscene = isUnskippableMobiCutscene(cutscene);
    }
    void onTerminateIngameCutscene() {
        if (_CurrentCutscene == nullptr) {
            return;
        }
        printf("Ingame cutscene terminated\n");
        _ShouldTerminateIngameCutscene = false;
        _StoppedIngameCutscene = true;

        if (_IsUnskippableCutscene) {
            _StoppedIngameCutscene = false;
        }
    }
    void onReplacementCutsceneStarted() {
        printf("Cutscene started\n");
        _ShouldStartReplacementCutscene = false;
        _StartedReplacementCutscene = true;
        _RunningReplacementCutscene = true;
    }

    void onReplacementCutsceneEnd() {
        printf("Replacement cutscene ended\n");
        _StartedReplacementCutscene = false;
        _RunningReplacementCutscene = false;
        _ShouldStopReplacementCutscene = false;
        _ShouldReturnToGameAfterCutscene = true;
        _ShouldHideScreenForTransitions = false;
    }
    void onReturnToGameAfterCutscene() {
        printf("Returning to the game\n");
        _StartPressCount = 0;
        _IsUnskippableCutscene = false;
        _ShouldStartReplacementCutscene = false;
        _StartedReplacementCutscene = false;
        _RunningReplacementCutscene = false;
        _ShouldReturnToGameAfterCutscene = false;
        _ShouldUnmuteAfterCutscene = true;

        _LastCutscene = _CurrentCutscene;
        _CurrentCutscene = nullptr;
        _ReplayLimitCount = 30;

        if (_NextCutscene == nullptr) {
            u32 cutsceneAddress = detectTopScreenMobiCutsceneAddress();
            if (cutsceneAddress != 0) {
                nds->ARM7Write32(cutsceneAddress, 0x0);
            }

            u32 cutsceneAddress2 = detectBottomScreenMobiCutsceneAddress();
            if (cutsceneAddress2 != 0) {
                nds->ARM7Write32(cutsceneAddress2, 0x0);
            }
        }
    }


    void initBgmVariables() {
        _CurrentBackgroundMusic = 0;
        _LastSoundtrackId = 0;
        _PausedReplacementBgmMusic = false;
        _ShouldPauseReplacementBgmMusic = false;
        _ShouldUnpauseReplacementBgmMusic = false;
    }
    bool ShouldStartReplacementBgmMusic() {
        if (_ShouldStartReplacementBgmMusic) {
            _ShouldStartReplacementBgmMusic = false;
            return true;
        }
        return false;
    }
    int delayBeforeStartReplacementBackgroundMusic() {return 0;}
    bool StartedReplacementBgmMusic() {
        if (_StartedReplacementBgmMusic) {
            _StartedReplacementBgmMusic = false;
            return true;
        }
        return false;
    }
    bool RunningReplacementBgmMusic() {return _RunningReplacementBgmMusic;}
    bool ShouldPauseReplacementBgmMusic() {
        if (_ShouldPauseReplacementBgmMusic) {
            _ShouldPauseReplacementBgmMusic = false;
            _PausedReplacementBgmMusic = true;
            return true;
        }
        return false;
    }
    bool ShouldUnpauseReplacementBgmMusic() {
        if (_ShouldUnpauseReplacementBgmMusic) {
            _ShouldUnpauseReplacementBgmMusic = false;
            _PausedReplacementBgmMusic = false;
            return true;
        }
        return false;
    }
    bool ShouldStopReplacementBgmMusic() {
        if (_ShouldStopReplacementBgmMusic) {
            _ShouldStopReplacementBgmMusic = false;
            return true;
        }
        return false;
    }
    u16 CurrentBackgroundMusic() {return _CurrentBackgroundMusic;};

    virtual std::string replacementBackgroundMusicFilePath(std::string name) = 0;

    void onReplacementBackgroundMusicStarted() {
        printf("Background music started\n");
        _ShouldStartReplacementBgmMusic = false;
        _StartedReplacementBgmMusic = true;
        _RunningReplacementBgmMusic = true;
    }

    virtual void refreshBackgroundMusic() {}


    virtual const char* getGameSceneName() = 0;

    bool _superShouldRenderFrame()
    {
        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene)
        {
            return false;
        }

        return true;
    }
    virtual bool shouldRenderFrame() = 0;

    virtual int detectGameScene() {return -1;}
    bool setGameScene(int newGameScene)
    {
        bool updated = false;
        if (GameScene != newGameScene) 
        {
            updated = true;

            // Game scene
            PriorGameScene = GameScene;
            GameScene = newGameScene;
        }

        return updated;
    }
    bool refreshGameScene()
    {
        int newGameScene = detectGameScene();
        
        debugLogs(newGameScene);

        bool updated = setGameScene(newGameScene);

        refreshCutscene();

        refreshBackgroundMusic();

        return updated;
    }

    virtual void setAspectRatio(float aspectRatio) {}

    void _superLoadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        std::string root = tomlUniqueIdentifier();
        DisableEnhancedGraphics = getBoolConfig(root + ".DisableEnhancedGraphics");
        ExportTextures = getBoolConfig(root + ".ExportTextures");
        FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    }
    virtual void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        _superLoadConfigs(getBoolConfig, getStringConfig);
    }

    virtual void hudToggle() {}

    virtual void debugLogs(int gameScene) {}

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
protected:
    melonDS::NDS* nds;

    int PriorGameScene;
    int GameScene;
    int HUDState;

    bool DisableEnhancedGraphics = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;

    int _FastForwardPressCount;
    int _StartPressCount;
    int _ReplayLimitCount;
    bool _CanSkipHdCutscene;
    bool _SkipDsCutscene;
    bool _IsUnskippableCutscene;
    bool _ShouldTerminateIngameCutscene;
    bool _StoppedIngameCutscene;
    bool _ShouldStartReplacementCutscene;
    bool _StartedReplacementCutscene;
    bool _RunningReplacementCutscene;
    bool _PausedReplacementCutscene;
    bool _ShouldPauseReplacementCutscene;
    bool _ShouldUnpauseReplacementCutscene;
    bool _ShouldStopReplacementCutscene;
    bool _ShouldReturnToGameAfterCutscene;
    bool _ShouldUnmuteAfterCutscene;
    bool _ShouldHideScreenForTransitions;
    CutsceneEntry* _CurrentCutscene;
    CutsceneEntry* _NextCutscene;
    CutsceneEntry* _LastCutscene;


    bool _StartedReplacementBgmMusic;
    bool _RunningReplacementBgmMusic;
    bool _PausedReplacementBgmMusic;
    bool _ShouldPauseReplacementBgmMusic;
    bool _ShouldUnpauseReplacementBgmMusic;
    bool _ShouldStartReplacementBgmMusic;
    bool _ShouldStopReplacementBgmMusic;
    u16 _CurrentBackgroundMusic;
    u16 _LastSoundtrackId;

};
}

#endif
