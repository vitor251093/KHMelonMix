/*
    Copyright 2016-2022 melonDS team

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

#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include "types.h"
#include "SaveManager.h"
#include "AREngine.h"

#include <string>
#include <vector>

namespace ROMManager
{

enum SetupResult {
    SUCCESS = 0,
    BIOS9_MISSING,
    BIOS9_BAD,
    BIOS7_MISSING,
    BIOS7_BAD,
    FIRMWARE_MISSING,
    FIRMWARE_BAD,
    FIRMWARE_NOT_BOOTABLE,
    DSI_BIOS9_MISSING,
    DSI_BIOS9_BAD,
    DSI_BIOS7_MISSING,
    DSI_BIOS7_BAD,
    DSI_NAND_MISSING,
    DSI_NAND_BAD
};

extern SaveManager* NDSSave;
extern SaveManager* GBASave;

SetupResult VerifySetup();
void Reset();
bool LoadBIOS();

bool LoadROM(std::string filepath, std::string sramPath, bool reset);
void EjectCart();
bool CartInserted();
std::string CartLabel();

bool LoadGBAROM(std::string filepath, std::string sramPath);
void LoadGBAAddon(int type);
void EjectGBACart();
bool GBACartInserted();
std::string GBACartLabel();

std::string GetSavestateName(int slot);
bool SavestateExists(int slot);
bool LoadState(std::string filename);
bool SaveState(std::string filename);
void UndoStateLoad();

void EnableCheats(bool enable);
ARCodeFile* GetCheatFile();

void ROMIcon(u8 (&data)[512], u16 (&palette)[16], u32* iconRef);
void AnimatedROMIcon(u8 (&data)[8][512], u16 (&palette)[8][16],
                     u16 (&sequence)[64], u32 (&animatedTexRef)[32 * 32 * 64],
                     std::vector<int> &animatedSequenceRef);

}

#endif // ROMMANAGER_H
