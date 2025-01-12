#include "PluginMetroidPrimeHunters.h"

#define ASPECT_RATIO_ADDRESS_US      0
#define ASPECT_RATIO_ADDRESS_EU      0
#define ASPECT_RATIO_ADDRESS_EU_REV1 0x02111380
#define ASPECT_RATIO_ADDRESS_JP      0

namespace Plugins
{

u32 PluginMetroidPrimeHunters::usGamecode = 0;
u32 PluginMetroidPrimeHunters::euGamecode = 1346915649;
u32 PluginMetroidPrimeHunters::jpGamecode = 0;

PluginMetroidPrimeHunters::PluginMetroidPrimeHunters(u32 gameCode)
{
    GameCode = gameCode;

    hudToggle();
}

void PluginMetroidPrimeHunters::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    
}
void PluginMetroidPrimeHunters::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, 1, true);
}

u32 PluginMetroidPrimeHunters::getAspectRatioAddress() {
    return getU32ByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_EU_REV1, ASPECT_RATIO_ADDRESS_JP);
}

u32 PluginMetroidPrimeHunters::getU32ByCart(u32 usAddress, u32 euAddress, u32 euRev1Address, u32 jpAddress)
{
    u32 value = 0;
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeRev1Cart()) {
        value = euRev1Address;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

std::string PluginMetroidPrimeHunters::getStringByCart(std::string usAddress, std::string euAddress, std::string euRev1Address, std::string jpAddress)
{
    std::string value = "";
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeRev1Cart()) {
        value = euRev1Address;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

bool PluginMetroidPrimeHunters::getBoolByCart(bool usAddress, bool euAddress, bool euRev1Address, bool jpAddress)
{
    bool value = false;
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeRev1Cart()) {
        value = euRev1Address;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

}