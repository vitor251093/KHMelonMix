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
    PluginDefault() {};

    u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) {
        return InputMask;
    }
    void applyTouchScreenMask(melonDS::NDS* nds, u32 TouchMask)
    {
    }
    const char* getGameSceneName() {
        return "";
    }
    bool shouldSkipFrame(melonDS::NDS* nds) {
        return false;
    }
    bool refreshGameScene(melonDS::NDS* nds) {
        return false;
    }
    void setAspectRatio(melonDS::NDS* nds, float aspectRatio) {
    }
};
}

#endif
