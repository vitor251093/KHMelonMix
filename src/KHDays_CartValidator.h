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

#ifndef KHDAYSCARTVALID_H
#define KHDAYSCARTVALID_H

#include "Utils.h"

namespace melonDS
{

class KHDaysCartValidator
{
public:
    static bool isValid(u32 gamecode)
    {
        // Only Days should be loadable
        Log(LogLevel::Info, "Game code: %u\n", gamecode);
        u32 usGamecode = 1162300249;
        u32 euGamecode = 1346849625;
        u32 jpGamecode = 1246186329;
        return (gamecode == usGamecode || gamecode == euGamecode || gamecode == jpGamecode);
    }
};

}
#endif // KHDAYSCARTVALID_H
