#ifndef PLUGIN_TEMPLATE_H
#define PLUGIN_TEMPLATE_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginTemplate : public Plugin
{
public:
    PluginTemplate(u32 gameCode);

    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    //static bool isCart(u32 gameCode) {return true;};
    bool isUsaCart()        { return GameCode == usGamecode; };
    bool isEuropeCart()     { return GameCode == euGamecode; };
    bool isJapanCart()      { return GameCode == jpGamecode; };

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    std::vector<ShapeData2D> renderer_2DShapes();
    std::vector<ShapeData3D> renderer_3DShapes();
    int renderer_screenLayout();
    int renderer_brightnessMode();
    bool renderer_showOriginalUI();

    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);

    const char* getGameSceneName() {
        return "";
    }

    u32 getAspectRatioAddress();

private:
    int detectGameScene();
};
}

#endif
