#ifndef PLUGIN_HarvestMoonDsCute_H
#define PLUGIN_HarvestMoonDsCute_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginHarvestMoonDsCute : public Plugin
{
public:
    PluginHarvestMoonDsCute(u32 gameCode);

    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()        { return GameCode == usGamecode; };
    bool isEuropeCart()     { return GameCode == euGamecode; };
    bool isJapanCart()      { return GameCode == jpGamecode; }

    void loadLocalization();
    std::string localizationFilePath(std::string language) override;
    void onLoadROM() override;
    void onLoadState() override;

    std::string gameFolderName() override;
    std::string assetsRegionSubfolder();

    std::vector<ShapeData2D> renderer_2DShapes() override;
    int renderer_screenLayout() override;
    int renderer_brightnessMode() override;
    bool renderer_showOriginalUI() override;

    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) override;
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) override;

    const char* getGameSceneName() override {
        return "";
    }

    u32 getAspectRatioAddress() override;

private:
    int detectGameScene() override;
};
}

#endif
