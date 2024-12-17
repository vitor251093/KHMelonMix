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

        hudToggle();
    };

    static bool isCart(u32 gameCode) {return true;};

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) {
        bool shouldContinue = _superApplyHotkeyToInputMask(InputMask, HotkeyMask, HotkeyPress);
        if (!shouldContinue) {
            return;
        }
    }
    void applyTouchKeyMask(u32 TouchKeyMask, u16* touchX, u16* touchY, bool* isTouching) {
        _superApplyTouchKeyMask(TouchKeyMask, 3, true, touchX, touchY, isTouching);
    }

    const char* getGameSceneName() {
        return "";
    }

    u32 getAspectRatioAddress() {return 0;}
};
}

#endif
