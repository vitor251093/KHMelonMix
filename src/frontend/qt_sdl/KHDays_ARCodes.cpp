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

#include "KHDays_ARCodes.h"
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

KHDaysARCodes::KHDaysARCodes(const std::string& filename)
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

ARCode KHDaysARCodes::ChangeAspectRatio(std::string codeName, u32 address)
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

bool KHDaysARCodes::Load()
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
    curcat.Name = "KHDaysCheats";
    curcat.Codes.clear();

    if (CartValidator::isUsaCart()) {
        curcat.Codes.push_back(ChangeAspectRatio("Auto Resolution (US)", 0x02023C9C));
    }
    if (CartValidator::isEuropeCart()) {
        curcat.Codes.push_back(ChangeAspectRatio("Auto Resolution (EU)", 0x02023CBC));
    }
    if (CartValidator::isJapanCart()) {
        // TODO: Add auto resolution for Japanese cart
    }

    Categories.push_back(curcat);
    return true;
}

}