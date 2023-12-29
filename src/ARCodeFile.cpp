/*
    Copyright 2016-2023 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <string.h>
#include "ARCodeFile.h"
#include "Platform.h"
#include <math.h>

namespace melonDS
{
using namespace Platform;

// TODO: import codes from other sources (usrcheat.dat, ...)
// TODO: more user-friendly error reporting


ARCodeFile::ARCodeFile(float screenAspect)
{
    Error = false;
    ScreenAspect = screenAspect;
    if (ScreenAspect == 0) {
        ScreenAspect = 16.0 / 9;
    }

    Categories.clear();

    if (!Load())
        Error = true;
}

ARCodeFile::~ARCodeFile()
{
    Categories.clear();
}

ARCode ARCodeFile::AlwaysEnableXAndDPadToControlCommandMenu(std::string codeName, u32 address)
{
    // Example:
    // if (mem16[0x02194CC2] < 0x4300) {
    //     if (mem16[0x02194CC2] > 0x41FF) {
    //         mem8[0x02194CC3] = 0x40;
    //     }
    // }

    ARCode curcode2;
    curcode2.Name = codeName;
    curcode2.Enabled = true;
    curcode2.Code.clear();
    curcode2.Code.push_back((0x70000000 | address) - 0x1); curcode2.Code.push_back(0x4300);
    curcode2.Code.push_back((0x80000000 | address) - 0x1); curcode2.Code.push_back(0x41FF);
    curcode2.Code.push_back(0x20000000 | address); curcode2.Code.push_back(0x40);
    curcode2.Code.push_back(0xD2000000); curcode2.Code.push_back(0x00000000);
    curcode2.Code.push_back(0xD2000000); curcode2.Code.push_back(0x00000000);
    return curcode2;
}

bool ARCodeFile::Load()
{
    // References
    // https://uk.codejunkies.com/support_downloads/Trainer-Toolkit-for-Nintendo-DS-User-Manual.pdf

    int aspectRatioKey = (int)round(0x1000 * ScreenAspect);

    Categories.clear();

    ARCodeCat curcat;
    curcat.Name = "KHDaysCheats";
    curcat.Codes.clear();

    // Also known as the "Widescreen hack"
    // TODO: Shouldn't be applied while the menu is opened
    ARCode curcode;
    curcode.Name = "Auto Resolution (US)";
    curcode.Enabled = true;
    curcode.Code.clear();
    curcode.Code.push_back(0x52023C9C); curcode.Code.push_back(0x00001555);     // if (mem32[0x02023C9C] == 0x00001555) {
    curcode.Code.push_back(0x02023C9C); curcode.Code.push_back(aspectRatioKey); //     mem32[0x02023C9C] = aspectRatioKey;
    curcode.Code.push_back(0xD2000000); curcode.Code.push_back(0x00000000);     // }
    curcat.Codes.push_back(curcode);

    curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (US)",      0x02194CC3));
    curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (EU)",      0x02195AA3));
    curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (JP)",      0x02193E23));
    curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (JP Rev1)", 0x02193DA3));

    Categories.push_back(curcat);
    return true;
}

bool ARCodeFile::Save()
{
    return false;
}

}