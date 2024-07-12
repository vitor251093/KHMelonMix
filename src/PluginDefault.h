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

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) {
        return InputMask;
    }
    void applyTouchKeyMask(melonDS::NDS* nds, u32 TouchKeyMask) {}
    const char* getGameSceneName() {
        return "";
    }
    bool shouldRenderFrame(melonDS::NDS* nds) {
        return true;
    }
    bool ShouldStartIngameCutscene() {return false;}
    bool ShouldStartReplacementCutscene() {return false;}
    bool StartedReplacementCutscene() {return false;}
    bool ShouldStopIngameCutscene() {return false;}
    CutsceneEntry* CurrentCutscene() {return nullptr;}
    std::string CutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    void onIngameCutsceneIdentified(melonDS::NDS* nds, CutsceneEntry* cutscene) {}
    void onIngameCutsceneStart(melonDS::NDS* nds) {}
    void onIngameCutsceneEnd(melonDS::NDS* nds) {}
    void onReplacementCutsceneStart(melonDS::NDS* nds) {}
    void onReplacementCutsceneEnd(melonDS::NDS* nds) {}
    bool refreshGameScene(melonDS::NDS* nds) {
        return false;
    }
    void setAspectRatio(melonDS::NDS* nds, float aspectRatio) {}
};
}

#endif
