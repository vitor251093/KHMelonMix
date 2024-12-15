#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true
#define REPLACEMENT_BGM_ENABLED true
#define MOUSE_CURSOR_AS_CAMERA_ENABLED false

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
#include <math.h>

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

    bool togglePause();

    bool _superApplyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    virtual void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) = 0;

    virtual bool overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching) {return false;}
    void _superApplyTouchKeyMask(u32 TouchKeyMask, u16 sensitivity, bool resetOnEdge, u16* touchX, u16* touchY, bool* isTouching);
    virtual void applyTouchKeyMask(u32 TouchKeyMask, u16* touchX, u16* touchY, bool* isTouching) = 0;

    bool shouldExportTextures() {
        return ExportTextures;
    }
    bool shouldStartInFullscreen() {
        return FullscreenOnStartup;
    }

    void initCutsceneVariables();
    bool ShouldTerminateIngameCutscene();
    bool StoppedIngameCutscene();
    bool ShouldStartReplacementCutscene();
    bool StartedReplacementCutscene();
    bool RunningReplacementCutscene();
    bool ShouldPauseReplacementCutscene();
    bool ShouldUnpauseReplacementCutscene();
    bool ShouldStopReplacementCutscene();
    bool ShouldReturnToGameAfterCutscene();
    bool ShouldUnmuteAfterCutscene();
    CutsceneEntry* CurrentCutscene();

    virtual CutsceneEntry* getMobiCutsceneByAddress(u32 cutsceneAddressValue) {return nullptr;}
    virtual u32 detectTopScreenMobiCutsceneAddress() {return 0;};
    virtual u32 detectBottomScreenMobiCutsceneAddress() {return 0;};
    CutsceneEntry* detectTopScreenMobiCutscene();
    CutsceneEntry* detectBottomScreenMobiCutscene();
    CutsceneEntry* detectCutscene();
    virtual bool isCutsceneGameScene() {return false;};
    virtual bool didMobiCutsceneEnded() {return !isCutsceneGameScene();};
    virtual bool canReturnToGameAfterReplacementCutscene() {return true;};

    void refreshCutscene();

    virtual std::string replacementCutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    virtual std::string localizationFilePath(std::string language) {return "";}
    virtual bool isUnskippableMobiCutscene(CutsceneEntry* cutscene) {return false;}

    void onIngameCutsceneIdentified(CutsceneEntry* cutscene);
    void onTerminateIngameCutscene();
    void onReplacementCutsceneStarted();
    void onReplacementCutsceneEnd();
    void onReturnToGameAfterCutscene();


    void initBgmVariables();
    bool ShouldStartReplacementBgmMusic();
    int delayBeforeStartReplacementBackgroundMusic();
    bool StartedReplacementBgmMusic();
    bool RunningReplacementBgmMusic();
    bool ShouldPauseReplacementBgmMusic();
    bool ShouldUnpauseReplacementBgmMusic();
    bool ShouldStopReplacementBgmMusic();
    u16 CurrentBackgroundMusic();

    virtual std::string replacementBackgroundMusicFilePath(std::string name) {return "";}

    void onReplacementBackgroundMusicStarted();

    virtual void refreshBackgroundMusic() {}


    virtual void refreshMouseStatus() {}

    bool ShouldGrabMouseCursor();
    bool ShouldReleaseMouseCursor();
    bool isMouseCursorGrabbed();


    virtual const char* getGameSceneName() = 0;

    bool _superShouldRenderFrame();
    virtual bool shouldRenderFrame() {
        return _superShouldRenderFrame();
    }

    virtual int detectGameScene() {return -1;}
    bool setGameScene(int newGameScene);
    bool refreshGameScene();

    virtual u32 getAspectRatioAddress() {return 0;}
    virtual void setAspectRatio(float aspectRatio);

    void _superLoadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig);
    virtual void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig);

    virtual void hudToggle() {}

    virtual void debugLogs(int gameScene) {}

    void log(const char* log);

    u32 LastMainRAM[0xFFFFFF];
    bool MainRAMState[0xFFFFFF];

    void ramSearch(melonDS::NDS* nds, u32 HotkeyPress);
protected:
    melonDS::NDS* nds;

    float AspectRatio;
    int PriorGameScene;
    int GameScene;
    int HUDState;

    bool DisableEnhancedGraphics = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;

    bool _LastTouchScreenMovementWasByPlugin;

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


    bool _ShouldGrabMouseCursor = false;
    bool _ShouldReleaseMouseCursor = false;
    bool _MouseCursorIsGrabbed = false;

};
}

#endif
