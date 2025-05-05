
#ifndef PLUGIN_TEMPLATE_H
#define PLUGIN_TEMPLATE_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;



enum {
    Shape_fromPosition,
    Shape_withSize,
    Shape_placeAtCorner,
    Shape_withMargin,
    Shape_sourceScale,
    Shape_fadeBorderSize,
    Shape_opacity,
    Shape_invertGrayScaleColors,
    Shape_hudScale,
    Shape_build,
    Shape_fromBottomScreen
};

struct ShapeBuilderCall
{
    int callNo;
    float arg1=0.0;
    float arg2=0.0;
    float arg3=0.0;
    float arg4=0.0;
};

int MakeShape(std::vector<ShapeBuilderCall> calls,float aRatio);
int SetShapes(std::vector<int> shapes);
void setLuaGameScene(int gamescene);

class PluginTemplate : public Plugin
{
public:
    PluginTemplate(u32 gameCode);
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return true;};
    bool isUsaCart()        { return GameCode == usGamecode; };
    bool isEuropeCart()     { return GameCode == euGamecode; };
    bool isJapanCart()      { return GameCode == jpGamecode; };

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }
    int renderer_gameSceneState();
    std::vector<ShapeData2D> renderer_2DShapes(int gameScene, int gameSceneState);
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