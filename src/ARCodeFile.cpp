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

bool ARCodeFile::Load()
{
    int aspectRatioKey = (int)round(0x1000 * ScreenAspect);

    Categories.clear();

    ARCodeCat curcat;
    curcat.Name = "KHDaysCheats";
    curcat.Codes.clear();

    // Also known as the "Widescreen hack"
    ARCode curcode;
    curcode.Name = "Auto Resolution";
    curcode.Enabled = true;
    curcode.Code[0] = 0x52023C9C; curcode.Code[1] = 0x00001555;
    curcode.Code[2] = 0x02023C9C; curcode.Code[3] = aspectRatioKey;
    curcode.Code[4] = 0xD2000000; curcode.Code[5] = 0x00000000;
    curcat.Codes.push_back(curcode);

    Categories.push_back(curcat);
    return true;
}

bool ARCodeFile::Save()
{
    return false;
}

}