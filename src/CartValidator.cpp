#include "CartValidator.h"

namespace melonDS
{

u32 GameCode = 1162300249;

u32 CartValidator::get()
{
    return GameCode;
}
void CartValidator::load(u32 gamecode)
{
    GameCode = gamecode;
}
bool CartValidator::isDays()
{
    u32 usGamecode = 1162300249;
    u32 euGamecode = 1346849625;
    u32 jpGamecode = 1246186329;
    return (GameCode == usGamecode || GameCode == euGamecode || GameCode == jpGamecode);
}
bool CartValidator::isRecoded()
{
    u32 usGamecode = 1161382722;
    u32 euGamecode = 1345932098;
    u32 jpGamecode = 1245268802;
    return (GameCode == usGamecode || GameCode == euGamecode || GameCode == jpGamecode);
}
bool CartValidator::isValid()
{
    return isDays() || isRecoded();
}

};
