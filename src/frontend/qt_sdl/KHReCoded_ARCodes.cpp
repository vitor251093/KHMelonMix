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

#include "KHReCoded_ARCodes.h"
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "Screen.h"
#include "Config.h"
#include <math.h>
#include "CartValidator.h"

namespace melonDS
{
using namespace Platform;

KHReCodedARCodes::KHReCodedARCodes(const std::string& filename)
{
    Filename = filename;

    Error = false;

    float aspectTop = (Config::WindowWidth * 1.f) / Config::WindowHeight;
    for (auto ratio : aspectRatios)
    {
        if (ratio.id == Config::ScreenAspectTop)
            aspectTop = ratio.ratio * 4.0/3;
    }
    if (aspectTop == 0) {
        aspectTop = 16.0 / 9;
    }
    ScreenAspect = aspectTop;

    Categories.clear();

    if (!Load())
        Error = true;
}

ARCode KHReCodedARCodes::ChangeAspectRatio(std::string codeName, u32 address)
{
    // Also known as the "Widescreen hack"
    // TODO: Shouldn't be applied while the menu is opened

    // if (mem32[0x02023C9C] == 0x00001555) {
    //     mem32[0x02023C9C] = aspectRatioKey;
    // }

    int aspectRatioKey = (int)round(0x1000 * ScreenAspect);

    ARCode curcode;
    curcode.Name = codeName;
    curcode.Enabled = true;
    curcode.Code.clear();
    curcode.Code.push_back(0x50000000 | address); curcode.Code.push_back(0x00001555);
    curcode.Code.push_back(             address); curcode.Code.push_back(aspectRatioKey);
    curcode.Code.push_back(0xD2000000);           curcode.Code.push_back(0x00000000);
    return curcode;
}

ARCode KHReCodedARCodes::AlwaysEnableXAndDPadToControlCommandMenu(std::string codeName, u32 address)
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
    curcode2.Code.push_back( 0x20000000 | address);        curcode2.Code.push_back(0x40);
    curcode2.Code.push_back( 0xD2000000);                  curcode2.Code.push_back(0x00000000);
    curcode2.Code.push_back( 0xD2000000);                  curcode2.Code.push_back(0x00000000);
    return curcode2;
}

bool KHReCodedARCodes::Load()
{
    FileHandle* f = OpenFile(Filename, FileMode::ReadText);
    if (f) {
        CloseFile(f);
        return ARCodeFile::Load();
    }

    // References
    // https://uk.codejunkies.com/support_downloads/Trainer-Toolkit-for-Nintendo-DS-User-Manual.pdf

    Categories.clear();

    ARCodeCat curcat;
    curcat.Name = "KHRecodedCheats";
    curcat.Codes.clear();

    if (CartValidator::isUsaCart()) {
        curcat.Codes.push_back(ChangeAspectRatio("Auto Resolution (US)", 0x0202A810));
        // curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (US)", 0x02194CC3));
    }
    if (CartValidator::isEuropeCart()) {
        curcat.Codes.push_back(ChangeAspectRatio("Auto Resolution (EU)", 0x0202A824));
        // curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (EU)", 0x02195AA3));
    }
    if (CartValidator::isJapanCart()) {
        curcat.Codes.push_back(ChangeAspectRatio("Auto Resolution (JP)", 0x0202A728));
        // curcat.Codes.push_back(AlwaysEnableXAndDPadToControlCommandMenu("Always X + D-Pad (JP)", 0x02193E23));
    }

    Categories.push_back(curcat);
    return true;
}

}