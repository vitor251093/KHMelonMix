#ifndef PLUGIN_DEFAULT_H
#define PLUGIN_DEFAULT_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginDefault : public Plugin
{
public:
    PluginDefault(u32 gameCode) {
        GameCode = gameCode;

        PriorGameScene = -1;
        GameScene = -1;
        HUDState = -1;
        hudToggle();

        initCutsceneVariables();
        initBgmVariables();
    };

    static bool isCart(u32 gameCode) {return true;};

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) {}
    bool applyTouchKeyMask(u32 TouchKeyMask) {
        nds->SetTouchKeyMask(TouchKeyMask, true);
        return true;
    }

    bool shouldExportTextures() {return false;}
    bool shouldStartInFullscreen() {return false;}

    const char* getGameSceneName() {
        return "";
    }

    bool shouldRenderFrame() {
        return _superShouldRenderFrame();
    }

    void setAspectRatio(float aspectRatio) {}

    std::string replacementCutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    std::string LocalizationFilePath(std::string language) {return "";}
    
    std::string replacementBackgroundMusicFilePath(std::string name) {return "";}
};
}

#endif
