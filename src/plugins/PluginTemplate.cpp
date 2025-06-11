#include "PluginTemplate.h"

#define ASPECT_RATIO_ADDRESS_US      0
#define ASPECT_RATIO_ADDRESS_EU      0
#define ASPECT_RATIO_ADDRESS_JP      0

namespace Plugins
{

u32 PluginTemplate::usGamecode = 0;
u32 PluginTemplate::euGamecode = 0;
u32 PluginTemplate::jpGamecode = 0;

#define getAnyByCart(usAddress,euAddress,jpAddress) (isUsaCart() ? (usAddress) : (isEuropeCart() ? (euAddress) : (jpAddress)))

PluginTemplate::PluginTemplate(u32 gameCode)
{
    GameCode = gameCode;

    hudToggle();
}

int PluginTemplate::detectGameScene()
{
    if (nds == nullptr)
    {
        return GameScene;
    }

    return 0;
}

std::vector<ShapeData2D> PluginTemplate::renderer_2DShapes(int gameScene, int gameSceneState) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    auto shapes = std::vector<ShapeData2D>();
    int hudScale = UIScale;

    shapes.push_back(ShapeBuilder2D::square()
            //.fromBottomScreen()
            .fromPosition(128, 60)
            .withSize(72, 72)
            .placeAtCorner(corner_TopRight)
            .withMargin(0.0, 30.0, 9.0, 0.0)
            .sourceScale(0.8333)
            .fadeBorderSize(5.0, 5.0, 5.0, 5.0)
            .opacity(0.85)
            .invertGrayScaleColors()
            .hudScale(hudScale)
            .build(aspectRatio));
    
    return shapes;
}

int PluginTemplate::renderer_screenLayout() {
    return screenLayout_Top;
};

int PluginTemplate::renderer_brightnessMode() {
    return brightnessMode_TopScreen;
}

bool PluginTemplate::renderer_showOriginalUI() {
    return false;
}

void PluginTemplate::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    
}
void PluginTemplate::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, CameraSensitivity, true);
}

u32 PluginTemplate::getAspectRatioAddress() {
    return getAnyByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);
}

}