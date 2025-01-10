#ifndef PLUGIN_METROID_PRIME_HUNTERS_H
#define PLUGIN_METROID_PRIME_HUNTERS_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginMetroidPrimeHunters : public Plugin
{
public:
    PluginMetroidPrimeHunters(u32 gameCode);

    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()        { return GameCode == usGamecode; };
    bool isEuropeCart()     { return GameCode == euGamecode; };
    bool isEuropeRev1Cart() { return GameCode == euGamecode && nds != nullptr && nds->GetNDSCart() != nullptr && nds->GetNDSCart()->GetROMLength() == 67108864; };
    bool isJapanCart()      { return GameCode == jpGamecode; };

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void applyCustomKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);

    const char* getGameSceneName() {
        return "";
    }

    int detectGameScene() {return 0;}

    u32 getAspectRatioAddress();

    u32 getU32ByCart(u32 usAddress, u32 euAddress, u32 euRev1Address, u32 jpAddress);
    std::string getStringByCart(std::string usAddress, std::string euAddress, std::string euRev1Address, std::string jpAddress);
    bool getBoolByCart(bool usAddress, bool euAddress, bool euRev1Address, bool jpAddress);
};
}

#endif
