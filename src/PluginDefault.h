#ifndef PLUGIN_DEFAULT_H
#define PLUGIN_DEFAULT_H

#include "Plugin.h"
#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginDefault : public Plugin
{
public:
    PluginDefault(u32 gameCode) {GameCode = gameCode;};

    void setNds(melonDS::NDS* Nds) {}

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void onLoadState() {}

    u32 applyHotkeyToInputMask(u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) {
        return InputMask;
    }
    void applyTouchKeyMask(u32 TouchKeyMask) {}
    const char* getGameSceneName() {
        return "";
    }
    bool shouldRenderFrame() {
        return true;
    }
    bool ShouldTerminateIngameCutscene() {return false;}
    bool StoppedIngameCutscene() {return false;}
    bool ShouldStartReplacementCutscene() {return false;}
    bool StartedReplacementCutscene() {return false;}
    bool ShouldStopReplacementCutscene() {return false;}
    bool ShouldReturnToGameAfterCutscene() {return false;}
    bool ShouldUnmuteAfterCutscene() {return false;}
    CutsceneEntry* CurrentCutscene() {return nullptr;}
    std::string CutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    void onIngameCutsceneIdentified(CutsceneEntry* cutscene) {}
    void onTerminateIngameCutscene() {}
    void onReturnToGameAfterCutscene() {}
    void onReplacementCutsceneStarted() {}
    void onReplacementCutsceneEnd() {}
    bool refreshGameScene() {
        return false;
    }
    void setAspectRatio(float aspectRatio) {}
};
}

#endif
