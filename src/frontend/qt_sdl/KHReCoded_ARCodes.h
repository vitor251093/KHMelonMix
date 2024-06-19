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

#ifndef KHRECODEDARCODES_H
#define KHRECODEDARCODES_H

#include "ARCodeFile.h"
#include <string>
#include <list>
#include <vector>
#include "types.h"

namespace melonDS
{

class KHReCodedARCodes: public ARCodeFile
{
public:
    KHReCodedARCodes(const std::string& filename);

    bool Load() override;


private:
    float ScreenAspect;

    ARCode ChangeAspectRatio(std::string codeName, u32 address);
    ARCode AlwaysEnableXAndDPadToControlCommandMenu(std::string codeName, u32 address);
};

}
#endif // KHRECODEDARCODES_H
