#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "NDS.h"

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
    bool isJapanCartRev1() { return false; }; // TODO: KH Add support to Rev1

    void setNds(melonDS::NDS* Nds) {nds = Nds;};
    void onLoadROM();

    std::string assetsFolder();

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGL_VS_Z();

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    void onLoadState();

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    void applyTouchKeyMask(u32 TouchKeyMask);

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
    void onIngameCutsceneIdentified(CutsceneEntry* cutscene);
    void onTerminateIngameCutscene();
    void onReturnToGameAfterCutscene();
    void onReplacementCutsceneStarted();
    void onReplacementCutsceneEnd();

    const char* getGameSceneName();

    bool shouldRenderFrame();

    void setAspectRatio(float aspectRatio);

    bool refreshGameScene();
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

    std::map<GLuint, GLuint[10]> CompGpuLoc{};
    std::map<u32, GLuint[3]> CompGpu3DLoc{};

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

    int detectGameScene();
    bool setGameScene(int newGameScene);

    u32 getAddressByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address);

    u32 getCutsceneAddress(CutsceneEntry* entry);
    CutsceneEntry* detectCutscene();
    CutsceneEntry* detectSequenceCutscene();
    void refreshCutscene();

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
