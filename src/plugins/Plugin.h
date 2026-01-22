#include "PluginLogs.h"

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

    int DefaultCameraSensitivity = 3;

    bool shouldInvalidateConfigs = false;
    void invalidateConfigs() {
        shouldInvalidateConfigs = true;
    }

    void setNds(melonDS::NDS* Nds) {nds = Nds;};
    virtual void onLoadROM();
    virtual void onLoadState();

    std::filesystem::path myDocumentsFolderPath();

    std::filesystem::path _AssetsFolderPath;
    virtual std::string gameFolderName() {return std::to_string(GameCode);}
    std::filesystem::path gameAssetsFolderPath();
    virtual std::string tomlUniqueIdentifier() {return gameFolderName();};

    virtual const char* gpuOpenGL_FS();
    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader);
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader);

    virtual bool gpuOpenGL_applyChangesToPolygonVertex(int resolutionScale, s32 scaledPositions[10][2], melonDS::Polygon* polygon, ShapeData3D shape, int vertexIndex);
    virtual bool gpuOpenGL_applyChangesToPolygon(int resolutionScale, s32 scaledPositions[10][2], melonDS::Polygon* polygon);

    void buildShapes();
    virtual void renderer_beforeBuildingShapes() { };
    virtual std::vector<ShapeData2D> renderer_2DShapes() { return std::vector<ShapeData2D>(); };
    virtual std::vector<ShapeData3D> renderer_3DShapes() { return std::vector<ShapeData3D>(); };
    virtual void renderer_afterBuildingShapes() { };
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

    bool muteBGMs = false;
    static u16 BGM_INVALID_ID;
    bool isBackgroundMusicPlaying() const { return _CurrentBackgroundMusic != BGM_INVALID_ID; }
    u16 getCurrentBackgroundMusic() const { return _CurrentBackgroundMusic; }
    const std::string& getCurrentBackgroundMusicFilePath() const { return _CurrentBackgroundMusicFilepath; }
    u16 getBackgroundMusicToStop() const { return _BackgroundMusicToStop; }
    u32 getBackgroundMusicFadeOutToApply() const { return _BgmFadeOutDurationMs; }
    u8 getCurrentBgmMusicVolume() const { return _BackgroundMusicVolume; }
    u32 getBgmDelayAtStart() const { return _BackgroundMusicDelayAtStart; }
    bool getStoreBackgroundMusicPosition() const { return _StoreBackgroundMusicPosition; }
    bool getResumeFromPositionBackgroundMusic() const { return _ResumeBackgroundMusicPosition; }
    std::vector<std::string> audioPackNames();

    std::string getReplacementBackgroundMusicFilePath(u16 id);

    virtual bool isBackgroundMusicReplacementImplemented() const { return false; }
    virtual u16 getMidiBgmId() { return 0; }
    virtual u16 getMidiBgmToResumeId() { return BGM_INVALID_ID; }
    virtual u32 getMidiSequenceAddress(u16 bgmId) { return 0; }
    virtual u16 getMidiSequenceSize(u16 bgmId) { return 0; }
    virtual u32 getStreamBgmAddress() { return 0; }
    virtual u16 getStreamBgmCustomIdFromDsId(u8 dsId, u32 numSamples) { return BGM_INVALID_ID; }
    virtual u8 getMidiBgmState() { return 0; }
    virtual u8 getMidiBgmVolume() { return 0; }
    virtual u32 getBgmFadeOutDuration() { return 0; }
    virtual std::string getBackgroundMusicName(u16 soundtrackId) { return ""; }
    virtual int delayBeforeStartReplacementBackgroundMusic(u16 bgmId) { return 0; }

    void loadBgmRedirections();
    void refreshBackgroundMusic();
    std::string getBackgroundMusicName(u16 soundtrackId) const;
    void startBackgroundMusic(u16 bgmId, u8 bgmState);
    void stopBackgroundMusic(u16 fadeOutDuration);

    void refreshStreamedMusic();
    virtual void onStreamBgmReplacementStarted() {}
    virtual void muteStreamedMusic();
    void stopReplacementStreamBgm(u32 fadeOutDuration);

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

    void replacementTexturesToggle() {
        DisableReplacementTextures = !DisableReplacementTextures;
        texturesIndex.clear();
    }
    bool areReplacementTexturesDisabled() {return DisableReplacementTextures;}

    virtual void debugLogs(int gameScene) {}

    static void errorLog(const char* format, ...);

    u32 LastMainRAM[0xFFFFFF];
    bool MainRAMState[0xFFFFFF];

    void ramSearch(melonDS::NDS* nds, u32 HotkeyPress);
protected:
    std::map<GLuint, GLuint[20]> CompGpuLoc{};
    std::map<GLuint, GLuint> CompUboLoc{};

    std::map<u32, std::map<u32, GLuint[20]>> CompGpu3DLoc{};
    std::map<u32, std::map<u32, int[20]>> CompGpu3DLastValues{};
    std::map<u32, GLuint> CompUbo3DLoc{};
    bool CompUbo3DLocInit = false;

    std::vector<ShapeData2D> current2DShapes;
    std::vector<ShapeData3D> current3DShapes;

    int InternalResolutionScale = 1;
    float AspectRatio = 0;
    int PriorGameScene = -1;
    int GameScene = -1;
    int GameSceneState = -1;
    int HUDState = -1;
    int UIScale = 4;

    int CameraSensitivity = 0;
    bool EnhancedGraphics = true;
    bool SingleScreenMode = true;
    bool DisableReplacementTextures = false;
    bool FastForwardLoadingScreens = false;
    bool DaysDisableHisMemories = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;
    std::string SelectedAudioPack = "";

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
    bool _PendingReplacmentBgmMusicStart = false;
    bool _ShouldStartReplacementBgmMusic = false;
    bool _ShouldStopReplacementBgmMusic = false;
    bool _ShouldUpdateReplacementBgmMusicVolume = false;
    u16 _CurrentBackgroundMusic = BGM_INVALID_ID;
    std::string _CurrentBackgroundMusicFilepath;
    u32 _BackgroundMusicDelayAtStart = 0;
    u16 _BackgroundMusicToStop = 0;
    u8 _SoundtrackState = 0;
    u32 _BgmFadeOutDurationMs = 0;
    u8 _BackgroundMusicVolume = 0x7f;
    bool _StoreBackgroundMusicPosition = false;
    bool _ResumeBackgroundMusicPosition = false;
    std::map<std::string, std::string> _BgmRedirectors;

    u8 _BgmStreamState = 0;
    bool _BgmStreamMuted = false;
    u32 _CurrentStreamAddress = 0;
    bool _CurrentBgmIsStream = false;

    bool _ShouldGrabMouseCursor = false;
    bool _ShouldReleaseMouseCursor = false;
    bool _MouseCursorIsGrabbed = false;

public:
    bool isReady() { return GameCode != 0 && nds != nullptr && nds->NDSCartSlot.GetCart() != nullptr; };
};
}

#endif
