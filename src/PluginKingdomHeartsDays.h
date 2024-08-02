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
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };
    bool isJapanCartRev1() { return false; }; // TODO: KH Add support to Rev1

    std::string assetsFolder();

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGL_VS_Z();

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    void onLoadState(melonDS::NDS* nds);

    u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress);
    void applyTouchKeyMask(melonDS::NDS* nds, u32 TouchKeyMask);

    int _StartPressCount;
    int _SkipPressCount;
    bool _ShouldTerminateIngameCutscene;
    bool _ShouldStartReplacementCutscene;
    bool _StartedReplacementCutscene;
    bool _ShouldStopReplacementCutscene;
    bool _ShouldReturnToGameAfterCutscene;
    bool _ShouldHideScreenForTransitions;
    CutsceneEntry* _CurrentCutscene;
    bool ShouldTerminateIngameCutscene() {return _ShouldTerminateIngameCutscene;}
    bool ShouldStartReplacementCutscene() {return _ShouldStartReplacementCutscene;}
    bool StartedReplacementCutscene() {return _StartedReplacementCutscene;}
    bool ShouldStopReplacementCutscene() {
        if (_ShouldStopReplacementCutscene) {
            _ShouldStopReplacementCutscene = false;
            return true;
        }
        return false;
    }
    bool ShouldReturnToGameAfterCutscene() {return _ShouldReturnToGameAfterCutscene;}
    CutsceneEntry* CurrentCutscene() {return _CurrentCutscene;};
    std::string CutsceneFilePath(CutsceneEntry* cutscene);
    void onIngameCutsceneIdentified(melonDS::NDS* nds, CutsceneEntry* cutscene);
    void onTerminateIngameCutscene(melonDS::NDS* nds);
    void onReturnToGameAfterCutscene(melonDS::NDS* nds);
    void onReplacementCutsceneStart(melonDS::NDS* nds);
    void onReplacementCutsceneEnd(melonDS::NDS* nds);

    const char* getGameSceneName();

    bool shouldRenderFrame(melonDS::NDS* nds);

    void setAspectRatio(melonDS::NDS* nds, float aspectRatio);

    bool refreshGameScene(melonDS::NDS* nds);
private:
    bool PausedInGame;
    int HUDState;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    int priorGameScene;
    int GameScene;
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

    int detectGameScene(melonDS::NDS* nds);
    bool setGameScene(melonDS::NDS* nds, int newGameScene);

    u32 getAddressByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address);

    u32 getAddress(CutsceneEntry* entry);
    CutsceneEntry* detectCutscene(melonDS::NDS* nds);
    CutsceneEntry* detectSequenceCutscene(melonDS::NDS* nds);
    void refreshCutscene(melonDS::NDS* nds);

    u32 getCurrentMission(melonDS::NDS* nds);
    u32 getCurrentMap(melonDS::NDS* nds);

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack(melonDS::NDS* nds);
    bool isBottomScreen2DTextureBlack(melonDS::NDS* nds);
    void hudToggle(melonDS::NDS* nds);
    void debugLogs(melonDS::NDS* nds, int gameScene);
};
}

#endif
