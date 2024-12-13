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

        AspectRatio = 0;
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

    const char* getGameSceneName() {
        return "";
    }

    u32 getAspectRatioAddress() {return 0;}
};
}

#endif
