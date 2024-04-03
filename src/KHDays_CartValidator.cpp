#include "KHDays_CartValidator.h"

namespace melonDS
{

u32 GameCode = 1162300249;

u32 KHDaysCartValidator::get()
{
    return GameCode;
}
void KHDaysCartValidator::load(u32 gamecode)
{
    GameCode = gamecode;
}
bool KHDaysCartValidator::isDays()
{
    u32 usGamecode = 1162300249;
    u32 euGamecode = 1346849625;
    u32 jpGamecode = 1246186329;
    return (GameCode == usGamecode || GameCode == euGamecode || GameCode == jpGamecode);
}
bool KHDaysCartValidator::isRecoded()
{
    return !isDays() && GameCode != 0; // TODO: Unimplemented
}
bool KHDaysCartValidator::isValid()
{
    return isDays() || isRecoded();
}

};
