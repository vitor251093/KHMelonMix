#ifndef PLUGIN_H
#define PLUGIN_H

#define PLUGIN_ADDON_KEYS_ARRAY_SIZE_LIMIT 100

#define REPLACEMENT_CUTSCENES_ENABLED true
#define REPLACEMENT_BGM_ENABLED true
#define MOUSE_CURSOR_AS_CAMERA_ENABLED false

#define SHOW_GAME_SCENE false
#define DEBUG_MODE_ENABLED false
#define ERROR_LOG_FILE_ENABLED true

#define getPixel(buffer, x, y, layer) buffer[(256*3 + 1)*(y) + (x) + 256*(layer)]

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
#include <filesystem>

#include "../GPU3D.h"
#include "../NDS.h"

#include "../OpenGLSupport.h"

#include "./PluginShapes.h"

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

struct BgmEntry
{
    u8 dsId = 0;
    u8 loadingTableId = 0;
    char DsName[16];
    char Name[40];
};

enum EMidiState : u8 {
    Stopped = 0x00,
    LoadSequence  = 0x01,
    PrePlay  = 0x02,
    Playing = 0x03,
    Stopping = 0x04
};


struct TextureEntryScene
{
    std::string path;
    std::string fullPath;
    u32 time;
};

struct TextureEntry
{
    std::string type;
    u32 totalScenes;
    TextureEntryScene scenes[1000];

    u32 sceneIndex;

    void setPath(std::string _path) {
        type = "static";
        totalScenes = 1;
        sceneIndex = 0;
        scenes[0].path = _path;
    }

    void setFullPath(std::string _path) {
        scenes[0].fullPath = _path;
    }

    void setType(std::string _type) {
        type = _type;
        sceneIndex = 0;
    }

    void setFramePath(int index, std::string _path) {
        totalScenes = totalScenes > (index + 1) ? totalScenes : (index + 1);
        scenes[index].path = _path;
    }

    void setFrameFullPath(int index, std::string _path) {
        totalScenes = totalScenes > (index + 1) ? totalScenes : (index + 1);
        scenes[index].fullPath = _path;
    }

    void setFrameTime(int index, u32 _time) {
        totalScenes = totalScenes > (index + 1) ? totalScenes : (index + 1);
        scenes[index].time = (_time * 60) / 1000;
    }

    TextureEntryScene& getLastScene() {
        return scenes[sceneIndex];
    }

    TextureEntryScene& getNextScene() {
        if (type == "animation") {
            if (totalScenes == ++sceneIndex) {
                sceneIndex = 0;
            }
            return scenes[sceneIndex];
        }
        return scenes[sceneIndex];
    }
};


class Plugin
{
protected:
    melonDS::NDS* nds = nullptr;

public:
    virtual ~Plugin() { };

    u32 GameCode = 0;
    u32 getGameCode() {
        return GameCode;
    };
    static bool isCart(u32 gameCode) {return true;};

    void setNds(melonDS::NDS* Nds) {nds = Nds;};
    virtual void onLoadROM();
    virtual void onLoadState();

    virtual std::string assetsFolder() {return std::to_string(GameCode);}
    std::filesystem::path assetsFolderPath();
    virtual std::string tomlUniqueIdentifier() {return assetsFolder();};

    virtual const char* gpuOpenGL_FS();
    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader);
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader);

    virtual const char* gpu3DOpenGLClassic_VS_Z();
    virtual void gpu3DOpenGLClassic_VS_Z_initVariables(GLuint CompShader, u32 flags);
    virtual void gpu3DOpenGLClassic_VS_Z_updateVariables(GLuint CompShader, u32 flags);

    virtual void gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32 scaledPositions[10][2], melonDS::Polygon* polygon);

    virtual std::vector<ShapeData2D> renderer_2DShapes(int gameScene, int gameSceneState) { return std::vector<ShapeData2D>(); };
    virtual std::vector<ShapeData3D> renderer_3DShapes(int gameScene, int gameSceneState) { return std::vector<ShapeData3D>(); };
    virtual int renderer_gameSceneState() { return 0; };
    virtual int renderer_screenLayout() { return 0; };
    virtual int renderer_brightnessMode() { return 0; };
    virtual float renderer_forcedAspectRatio() {return AspectRatio;};
    virtual bool renderer_showOriginalUI() { return true; };

    bool togglePause();

    std::vector<const char*> customKeyMappingNames = {};
    std::vector<const char*> customKeyMappingLabels = {};

    bool _superApplyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    virtual void applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    virtual void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* AddonMask, u32* AddonPress) {};

    virtual bool overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching) {return false;}
    void _superApplyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask, u16 sensitivity, bool resetOnEdge);
    virtual void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);
    virtual bool shouldRumble() {return false;}

    bool shouldExportTextures() {
        return ExportTextures;
    }
    bool shouldStartInFullscreen() {
        return FullscreenOnStartup;
    }

    virtual std::string localizationFilePath(std::string language) {return "";}

    virtual std::string textureIndexFilePath();
    virtual std::map<std::string, TextureEntry>& getTexturesIndex();
    virtual TextureEntry& textureById(std::string texture);
    virtual std::string tmpTextureFilePath(std::string texture);

    virtual std::string replacementCutsceneFilePath(CutsceneEntry* cutscene) {return "";}

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

    virtual bool isUnskippableMobiCutscene(CutsceneEntry* cutscene) {return false;}

    void onIngameCutsceneIdentified(CutsceneEntry* cutscene);
    void onTerminateIngameCutscene();
    void onReplacementCutsceneStarted();
    void onReplacementCutsceneEnd();
    void onReturnToGameAfterCutscene();

    inline bool shouldStartBackgroundMusic() { return checkAndResetBool(_ShouldStartReplacementBgmMusic); }
    inline bool shouldStopBackgroundMusic() { return checkAndResetBool(_ShouldStopReplacementBgmMusic); }
    inline bool shouldPauseBackgroundMusic() {
        if (checkAndResetBool(_ShouldPauseReplacementBgmMusic)) {
            _PausedReplacementBgmMusic = true;
            return true;
        }
        return false;
    }
    inline bool shouldResumeBackgroundMusic() {
        if (checkAndResetBool(_ShouldUnpauseReplacementBgmMusic)) {
            _PausedReplacementBgmMusic = false;
            return true;
        }
        return false;
    }
    inline bool shouldUpdateBackgroundMusicVolume() { return checkAndResetBool(_ShouldUpdateReplacementBgmMusicVolume); }

    inline bool checkAndResetBool(bool& val) {
        if (val) {
            val = false;
            return true;
        }
        return false;
    }

    static u16 BGM_INVALID_ID;
    bool isBackgroundMusicPlaying() const { return _CurrentBackgroundMusic != BGM_INVALID_ID; }
    u16 getCurrentBackgroundMusic() const { return _CurrentBackgroundMusic; }
    u16 getBackgroundMusicToStop() const { return _BackgroundMusicToStop; }
    bool shouldForceStopMusic() const { return _ForceStopMusic; }
    u8 getCurrentBgmMusicVolume() const { return _BackgroundMusicVolume; }
    u32 getBgmDelayAtStart() const { return _BackgroundMusicDelayAtStart; }
    bool getStoreBackgroundMusicPosition() const { return _StoreBackgroundMusicPosition; }
    bool getResumeFromPositionBackgroundMusic() const { return _ResumeBackgroundMusicPosition; }

    std::string getReplacementBackgroundMusicFilePath(u16 id);

    virtual bool isBackgroundMusicReplacementImplemented() const { return false; }
    virtual u16 getMidiBgmId() { return 0; }
    virtual u16 getMidiBgmToResumeId() { return BGM_INVALID_ID; }
    virtual u32 getMidiSongTableAddress() { return 0; }
    virtual u8 getMidiBgmState() { return 0; }
    virtual u8 getMidiBgmVolume() { return 0; }
    virtual u16 getSongIdInSongTable(u16 bgmId) { return 0; }
    virtual std::string getBackgroundMusicName(u16 soundtrackId) { return ""; }
    virtual int delayBeforeStartReplacementBackgroundMusic() { return 0; }

    void refreshBackgroundMusic();
    std::string getBackgroundMusicName(u16 soundtrackId) const;
    void muteSongSequence(u16 bgmId);
    void stopBackgroundMusic(bool bImmediateStop);

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
    virtual void setInternalResolutionScale(int scale);

    void _superLoadConfigs(
        std::function<bool(std::string)> getBoolConfig,
        std::function<int(std::string)> getIntConfig,
        std::function<std::string(std::string)> getStringConfig
    );
    virtual void loadConfigs(
        std::function<bool(std::string)> getBoolConfig,
        std::function<int(std::string)> getIntConfig,
        std::function<std::string(std::string)> getStringConfig
    );

    virtual void hudToggle() {}

    virtual void debugLogs(int gameScene) {}

    void errorLog(const char* format, ...);

    u32 LastMainRAM[0xFFFFFF];
    bool MainRAMState[0xFFFFFF];

    void ramSearch(melonDS::NDS* nds, u32 HotkeyPress);
protected:
    std::map<GLuint, GLuint[20]> CompGpuLoc{};
    std::map<GLuint, int[20]> CompGpuLastValues{};
    std::map<GLuint, GLuint> CompUboLoc{};

    std::map<u32, std::map<u32, GLuint[20]>> CompGpu3DLoc{};
    std::map<u32, std::map<u32, int[20]>> CompGpu3DLastValues{};
    std::map<u32, GLuint> CompUbo3DLoc{};
    bool CompUbo3DLocInit = false;

    int InternalResolutionScale = 1;
    float AspectRatio = 0;
    int PriorGameScene = -1;
    int GameScene = -1;
    int HUDState = -1;
    int UIScale = 4;

    bool DisableEnhancedGraphics = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;

    bool _LastTouchScreenMovementWasByPlugin = false;

    std::map<std::string, TextureEntry> texturesIndex;

    int _StartPressCount = 0;
    int _ReplayLimitCount = 0;
    bool _CanSkipHdCutscene = false;
    bool _SkipDsCutscene = false;
    bool _IsUnskippableCutscene = false;
    bool _ShouldTerminateIngameCutscene = false;
    bool _StoppedIngameCutscene = false;
    bool _ShouldStartReplacementCutscene = false;
    bool _StartedReplacementCutscene = false;
    bool _RunningReplacementCutscene = false;
    bool _PausedReplacementCutscene = false;
    bool _ShouldPauseReplacementCutscene = false;
    bool _ShouldUnpauseReplacementCutscene = false;
    bool _ShouldStopReplacementCutscene = false;
    bool _ShouldReturnToGameAfterCutscene = false;
    bool _ShouldUnmuteAfterCutscene = false;
    bool _ShouldHideScreenForTransitions = false;
    CutsceneEntry* _CurrentCutscene = nullptr;
    CutsceneEntry* _NextCutscene = nullptr;
    CutsceneEntry* _LastCutscene = nullptr;


    bool _PausedReplacementBgmMusic = false;
    bool _ShouldPauseReplacementBgmMusic = false;
    bool _ShouldUnpauseReplacementBgmMusic = false;
    bool _ShouldStartReplacementBgmMusic = false;
    bool _ShouldStopReplacementBgmMusic = false;
    bool _ShouldUpdateReplacementBgmMusicVolume = false;
    u16 _CurrentBackgroundMusic = BGM_INVALID_ID;
    u32 _BackgroundMusicDelayAtStart = 0;
    u16 _BackgroundMusicToStop = 0;
    u8 _SoundtrackState = 0;
    bool _ForceStopMusic = false;
    bool _MuteSeqBgm = false;
    u8 _BackgroundMusicVolume = 0x7f;
    bool _StoreBackgroundMusicPosition = false;
    bool _ResumeBackgroundMusicPosition = false;

    bool _ShouldGrabMouseCursor = false;
    bool _ShouldReleaseMouseCursor = false;
    bool _MouseCursorIsGrabbed = false;

public:
    bool isReady() { return GameCode != 0 && nds != nullptr && nds->NDSCartSlot.GetCart() != nullptr; };
};
}

#endif
