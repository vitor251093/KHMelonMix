#ifndef KHRECODED_PLUGIN_H
#define KHRECODED_PLUGIN_H

#include "Plugin.h"
#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsReCoded : public Plugin
{
public:
    PluginKingdomHeartsReCoded(u32 gameCode);

    u32 GameCode;
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };

    void setNds(melonDS::NDS* Nds) {nds = Nds;};

    std::string assetsFolder();

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGL_VS_Z();

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    void onLoadState();

    u32 applyHotkeyToInputMask(u32 InputMask, u32 HotkeyMask, u32 HotkeyPress);
    void applyTouchKeyMask(u32 TouchKeyMask);

    bool _ShouldTerminateIngameCutscene;
    bool _StoppedIngameCutscene;
    bool _ShouldStartReplacementCutscene;
    bool _StartedReplacementCutscene;
    bool _ShouldStopReplacementCutscene;
    bool _ShouldReturnToGameAfterCutscene;
    CutsceneEntry* _CurrentCutscene;
    bool ShouldTerminateIngameCutscene() {return _ShouldTerminateIngameCutscene;}
    bool StoppedIngameCutscene() {
        if (_StoppedIngameCutscene) {
            _StoppedIngameCutscene = false;
            return true;
        }
        return false;
    }
    bool ShouldStartReplacementCutscene() {return _ShouldStartReplacementCutscene;}
    bool StartedReplacementCutscene() {return _StartedReplacementCutscene;}
    bool ShouldStopReplacementCutscene() {return _ShouldStopReplacementCutscene;}
    bool ShouldReturnToGameAfterCutscene() {return _ShouldReturnToGameAfterCutscene;}
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
    void debugLogs(int gameScene);
private:
    melonDS::NDS* nds;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    int priorGameScene;
    int GameScene;
    int UIScale = 4;
    float AspectRatio;
    bool ShowMap;

    std::map<GLuint, GLuint[10]> CompGpuLoc{};
    std::map<u32, GLuint[3]> CompGpu3DLoc{};

    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    u32 PriorHotkeyMask, PriorPriorHotkeyMask;

    int detectGameScene();
    bool setGameScene(int newGameScene);

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack();
    bool isBottomScreen2DTextureBlack();
    void hudToggle();
};
}

#endif
