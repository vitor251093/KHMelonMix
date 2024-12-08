#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsDays : public Plugin
{
public:
    PluginKingdomHeartsDays(u32 gameCode);

    u32 GameCode;
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    u32 getGameCode() { return GameCode; };
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };
    bool isJapanCartRev1() { return GameCode == jpGamecode && nds != nullptr && nds->GetNDSCart() != nullptr && nds->GetNDSCart()->GetROMLength() == 268435456; };

    void setNds(melonDS::NDS* Nds) {nds = Nds;};
    void loadLocalization();
    void onLoadROM();

    std::string assetsFolder();
    std::string assetsRegionSubfolder();

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGLClassic_VS_Z();
    void gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32* x, s32* y, s32 z, s32* rgb);

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    void onLoadState();
    bool togglePause();

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    bool applyTouchKeyMask(u32 TouchKeyMask);

    int _FastForwardPressCount;
    int _StartPressCount;
    int _ReplayLimitCount;
    bool _CanSkipHdCutscene;
    bool _SkipDsCutscene;
    bool _PlayingCutsceneBeforeCredits;
    bool _PlayingCredits;
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
    std::string CutsceneFilePath(CutsceneEntry* cutscene);
    std::string LocalizationFilePath(std::string language);
    std::filesystem::path patchCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    void onIngameCutsceneIdentified(CutsceneEntry* cutscene);
    void onTerminateIngameCutscene();
    void onReturnToGameAfterCutscene();
    void onReplacementCutsceneStarted();
    void onReplacementCutsceneEnd();

    bool _ShouldStartReplacementBgmMusic;
    bool _ShouldStopReplacementBgmMusic;
    u16 _CurrentBackgroundMusic;
    bool ShouldStartReplacementBgmMusic() {
        if (_ShouldStartReplacementBgmMusic) {
            _ShouldStartReplacementBgmMusic = false;
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
    std::string BackgroundMusicFilePath(std::string name);

    const char* getGameSceneName();

    bool shouldRenderFrame();

    void setAspectRatio(float aspectRatio);

    bool refreshGameScene();

    void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        KH_15_25_Remix_Location = getStringConfig("Kingdom_Hearts_HD_1_5_2_5_Remix_Location");

        KHDaysUSLanguage = getStringConfig("KHDays_US.Language");
        KHDaysEULanguage = getStringConfig("KHDays_EU.Language");
        KHDaysJPLanguage = getStringConfig("KHDays_JP.Language");
        KHDaysJPRev1Language = getStringConfig("KHDays_JPRev1.Language");

        KHDaysUSDisableEnhancedGraphics = getBoolConfig("KHDays_US.DisableEnhancedGraphics");
        KHDaysEUDisableEnhancedGraphics = getBoolConfig("KHDays_EU.DisableEnhancedGraphics");
        KHDaysJPDisableEnhancedGraphics = getBoolConfig("KHDays_JP.DisableEnhancedGraphics");
        KHDaysJPRev1DisableEnhancedGraphics = getBoolConfig("KHDays_JPRev1.DisableEnhancedGraphics");
    }
private:
    melonDS::NDS* nds;

    bool PausedInGame;
    int HUDState;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    int PriorGameScene;
    int GameScene;
    u32 priorMap;
    u32 Map;
    int UIScale = 4;
    float AspectRatio;
    bool ShowMap;
    bool ShowTarget;
    bool ShowMissionGauge;
    bool ShowMissionInfo;
    bool HideAllHUD;

    std::map<GLuint, GLuint[11]> CompGpuLoc{};
    std::map<u32, GLuint[4]> CompGpu3DLoc{};

    bool _muchOlderHad3DOnTopScreen;
    bool _muchOlderHad3DOnBottomScreen;
    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    bool _hasVisible3DOnBottomScreen;
    bool _ignore3DOnBottomScreen;
    bool _priorIgnore3DOnBottomScreen;
    bool _priorPriorIgnore3DOnBottomScreen;

    u32 PriorHotkeyMask, PriorPriorHotkeyMask;
    u32 LastLockOnPress, LastSwitchTargetPress;
    bool SwitchTargetPressOnHold;

    std::array<CutsceneEntry, 46> Cutscenes;

    std::string KH_15_25_Remix_Location = "";
    std::string KHDaysUSLanguage = "";
    std::string KHDaysEULanguage = "";
    std::string KHDaysJPLanguage = "";
    std::string KHDaysJPRev1Language = "";
    bool KHDaysUSDisableEnhancedGraphics = false;
    bool KHDaysEUDisableEnhancedGraphics = false;
    bool KHDaysJPDisableEnhancedGraphics = false;
    bool KHDaysJPRev1DisableEnhancedGraphics = false;

    int detectGameScene();
    bool setGameScene(int newGameScene);

    u32 getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address);
    std::string getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress, std::string jpRev1Address);
    bool getBoolByCart(bool usAddress, bool euAddress, bool jpAddress, bool jpRev1Address);

    u32 getCutsceneAddress(CutsceneEntry* entry);
    CutsceneEntry* detectTopScreenCutscene();
    CutsceneEntry* detectBottomScreenCutscene();
    CutsceneEntry* detectCutscene();
    CutsceneEntry* detectSequenceCutscene();
    void refreshCutscene();

    u16 detectBackgroundMusic();
    void refreshBackgroundMusic();

    u32 getCurrentMission();
    u32 getCurrentMainMenuView();
    u32 getCurrentMap();
    bool isSaveLoaded();

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack();
    bool isBottomScreen2DTextureBlack();
    void hudToggle();
    void debugLogs(int gameScene);
};
}

#endif
