#include "CartValidator.h"

namespace melonDS
{

u32 daysUsGamecode = 1162300249;
u32 daysEuGamecode = 1346849625;
u32 daysJpGamecode = 1246186329;

u32 recodedUsGamecode = 1161382722;
u32 recodedEuGamecode = 1345932098;
u32 recodedJpGamecode = 1245268802;

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
    return (GameCode == daysUsGamecode || GameCode == daysEuGamecode || GameCode == daysJpGamecode);
}
bool CartValidator::isRecoded()
{
    return (GameCode == recodedUsGamecode || GameCode == recodedEuGamecode || GameCode == recodedJpGamecode);
}
bool CartValidator::isUsaCart()
{
    return (GameCode == daysUsGamecode || GameCode == recodedUsGamecode);
}
bool CartValidator::isEuropeCart()
{
    return (GameCode == daysEuGamecode || GameCode == recodedEuGamecode);
}
bool CartValidator::isJapanCart()
{
    return (GameCode == daysJpGamecode || GameCode == recodedJpGamecode);
}
bool CartValidator::isValid()
{
    return isDays() || isRecoded();
}

};
