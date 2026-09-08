#include "PluginHarvestMoonDsCute.h"

#define ASPECT_RATIO_ADDRESS_US      0
#define ASPECT_RATIO_ADDRESS_EU      0
#define ASPECT_RATIO_ADDRESS_JP      0

namespace Plugins
{

u32 PluginHarvestMoonDsCute::usGamecode = 1161052737;
u32 PluginHarvestMoonDsCute::euGamecode = 0;
u32 PluginHarvestMoonDsCute::jpGamecode = 0;

#define getAnyByCart(usAddress,euAddress,jpAddress) (isUsaCart() ? (usAddress) : (isEuropeCart() ? (euAddress) : (jpAddress)))

PluginHarvestMoonDsCute::PluginHarvestMoonDsCute(u32 gameCode)
{
    GameCode = gameCode;

    Plugin::hudToggle();
}

void PluginHarvestMoonDsCute::onLoadROM() {
    Plugin::onLoadROM();

    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
}

void PluginHarvestMoonDsCute::onLoadState() {
    Plugin::onLoadState();
}

std::string PluginHarvestMoonDsCute::localizationFilePath(std::string language, bool emptyIfFileNotFound) {
    std::string filename = language + ".csv";
    std::string assetsRegionSubfolderName = assetsRegionSubfolder();
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path fullPath = _assetsFolderPath / "localization" / assetsRegionSubfolderName / filename;
    if (!emptyIfFileNotFound || std::filesystem::exists(fullPath)) {
        return fullPath.u8string();
    }

    return "";
}

std::string PluginHarvestMoonDsCute::gameFolderName() {
    return "hmdscute";
}

std::string PluginHarvestMoonDsCute::assetsRegionSubfolder() {
    return getAnyByCart("us", "eu", "jp");
}

int PluginHarvestMoonDsCute::detectGameScene()
{
    if (nds == nullptr)
    {
        return GameScene;
    }

    return 0;
}

std::vector<ShapeData2D> PluginHarvestMoonDsCute::renderer_topScreen_2DShapes() {
    auto shapes = std::vector<ShapeData2D>();
    return shapes;
}

int PluginHarvestMoonDsCute::renderer_screenLayout() {
    return screenLayout_Top;
};

int PluginHarvestMoonDsCute::renderer_brightnessMode() {
    return brightnessMode_TopScreen;
}

bool PluginHarvestMoonDsCute::renderer_showOriginalUI() {
    return true;
}

void PluginHarvestMoonDsCute::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    
}
void PluginHarvestMoonDsCute::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, CameraSensitivity, true);
}

u32 PluginHarvestMoonDsCute::getAspectRatioAddress() {
    return getAnyByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);
}

}