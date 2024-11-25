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

#include <stdio.h>
#include <string.h>

#include <string>
#include <utility>

#ifdef ARCHIVE_SUPPORT_ENABLED
#include "ArchiveUtil.h"
#endif
#include "ROMManager.h"
#include "Config.h"
#include "Platform.h"

#include "NDS.h"
#include "DSi.h"
#include "SPI.h"
#include "DSi_I2C.h"


namespace ROMManager
{

int CartType = -1;
int GBACartType = -1;

SaveManager* NDSSave = nullptr;
SaveManager* GBASave = nullptr;

bool SavestateLoaded = false;
std::string PreviousSaveFile = "";


int LastSep(std::string path)
{
    int i = path.length() - 1;
    while (i >= 0)
    {
        if (path[i] == '/' || path[i] == '\\')
            return i;

        i--;
    }

    return -1;
}

SetupResult VerifyDSBIOS()
{
    FILE* f;
    long len;

    f = Platform::OpenLocalFile(Config::BIOS9Path, "rb");
    if (!f) return BIOS9_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len != 0x1000)
    {
        fclose(f);
        return BIOS9_BAD;
    }

    fclose(f);

    f = Platform::OpenLocalFile(Config::BIOS7Path, "rb");
    if (!f) return BIOS7_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len != 0x4000)
    {
        fclose(f);
        return BIOS7_BAD;
    }

    fclose(f);

    return SUCCESS;
}

SetupResult VerifyDSiBIOS()
{
    FILE* f;
    long len;

    // TODO: check the first 32 bytes

    f = Platform::OpenLocalFile(Config::DSiBIOS9Path, "rb");
    if (!f) return DSI_BIOS9_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len != 0x10000)
    {
        fclose(f);
        return DSI_BIOS9_BAD;
    }

    fclose(f);

    f = Platform::OpenLocalFile(Config::DSiBIOS7Path, "rb");
    if (!f) return DSI_BIOS7_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len != 0x10000)
    {
        fclose(f);
        return DSI_BIOS7_BAD;
    }

    fclose(f);

    return SUCCESS;
}

SetupResult VerifyDSFirmware()
{
    FILE* f;
    long len;

    f = Platform::OpenLocalFile(Config::FirmwarePath, "rb");
    if (!f) return FIRMWARE_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len == 0x20000)
    {
        // 128KB firmware, not bootable
        fclose(f);
        // TODO report it somehow? detect in core?
        return SUCCESS;
    }
    else if (len != 0x40000 && len != 0x80000)
    {
        fclose(f);
        return FIRMWARE_BAD;
    }

    fclose(f);

    return SUCCESS;
}

SetupResult VerifyDSiFirmware()
{
    FILE* f;
    long len;

    f = Platform::OpenLocalFile(Config::DSiFirmwarePath, "rb");
    if (!f) return FIRMWARE_MISSING;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len != 0x20000)
    {
        // not 128KB
        // TODO: check whether those work
        fclose(f);
        return FIRMWARE_BAD;
    }

    fclose(f);

    return SUCCESS;
}

SetupResult VerifyDSiNAND()
{
    FILE* f;
    long len;

    f = Platform::OpenLocalFile(Config::DSiNANDPath, "r+b");
    if (!f) return DSI_NAND_MISSING;

    // TODO: some basic checks
    // check that it has the nocash footer, and all

    fclose(f);

    return SUCCESS;
}

SetupResult VerifySetup()
{
    SetupResult res;

    if (Config::ExternalBIOSEnable)
    {
        res = VerifyDSBIOS();
        if (res != SUCCESS) return res;
    }

    if (Config::ConsoleType == 1)
    {
        res = VerifyDSiBIOS();
        if (res != SUCCESS) return res;

        if (Config::ExternalBIOSEnable)
        {
            res = VerifyDSiFirmware();
            if (res != SUCCESS) return res;
        }

        res = VerifyDSiNAND();
        if (res != SUCCESS) return res;
    }
    else
    {
        if (Config::ExternalBIOSEnable)
        {
            res = VerifyDSFirmware();
            if (res != SUCCESS) return res;
        }
    }

    return SUCCESS;
}

void SetBatteryLevels()
{
    if (NDS::ConsoleType == 1)
    {
        DSi_BPTWL::SetBatteryLevel(Config::DSiBatteryLevel);
        DSi_BPTWL::SetBatteryCharging(Config::DSiBatteryCharging);
    }
    else
    {
        SPI_Powerman::SetBatteryLevelOkay(Config::DSBatteryLevelOkay);
    }
}

void Reset()
{
    NDS::SetConsoleType(Config::ConsoleType);
    if (Config::ConsoleType == 1) EjectGBACart();
    NDS::Reset();
    SetBatteryLevels();

    if (Config::DirectBoot || NDS::NeedsDirectBoot())
    {
        // TODO: Use proper file name
        NDS::SetupDirectBoot("");
    }
}


bool LoadBIOS()
{
    NDS::SetConsoleType(Config::ConsoleType);

    if (NDS::NeedsDirectBoot())
        return false;

    /*if (NDSSave) delete NDSSave;
    NDSSave = nullptr;

    CartType = -1;
    BaseROMDir = "";
    BaseROMName = "";
    BaseAssetName = "";*/

    NDS::Reset();
    SetBatteryLevels();
    return true;
}


bool LoadROM(std::string filepath, std::string sramPath, bool reset)
{
    if (filepath.empty()) return false;

    u8* filedata;
    u32 filelen;

    FILE* f = Platform::OpenFile(filepath, "rb", true);
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len > 0x40000000)
    {
        fclose(f);
        return false;
    }

    fseek(f, 0, SEEK_SET);
    filedata = new u8[len];
    size_t nread = fread(filedata, (size_t)len, 1, f);
    if (nread != 1)
    {
        fclose(f);
        delete[] filedata;
        return false;
    }

    fclose(f);
    filelen = (u32)len;

    if (NDSSave) delete NDSSave;
    NDSSave = nullptr;

    if (reset)
    {
        NDS::SetConsoleType(Config::ConsoleType);
        NDS::EjectCart();
        NDS::Reset();
        SetBatteryLevels();
    }

    u32 savelen = 0;
    u8* savedata = nullptr;

    FILE* sav = Platform::OpenFile(sramPath, "rb", true);
    if (sav)
    {
        fseek(sav, 0, SEEK_END);
        savelen = (u32)ftell(sav);

        if (savelen > 0)
        {
            fseek(sav, 0, SEEK_SET);
            savedata = new u8[savelen];
            fread(savedata, savelen, 1, sav);
        }
        fclose(sav);
    }

    bool res = NDS::LoadCart(filedata, filelen, savedata, savelen);
    if (res && reset)
    {
        if (Config::DirectBoot || NDS::NeedsDirectBoot())
        {
            // TODO: Send proper file name
            NDS::SetupDirectBoot("");
        }
    }

    if (res)
    {
        CartType = 0;
        NDSSave = new SaveManager(sramPath);
    }

    if (savedata) delete[] savedata;
    delete[] filedata;
    return res;
}

void EjectCart()
{
    if (NDSSave) delete NDSSave;
    NDSSave = nullptr;

    NDS::EjectCart();

    CartType = -1;
}

bool CartInserted()
{
    return CartType != -1;
}

bool LoadGBAROM(std::string filepath, std::string sramPath)
{
    if (Config::ConsoleType == 1) return false;
    if (filepath.empty()) return false;

    u8* filedata;
    u32 filelen;

    FILE* f = Platform::OpenFile(filepath, "rb", true);
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len > 0x40000000)
    {
        fclose(f);
        return false;
    }

    fseek(f, 0, SEEK_SET);
    filedata = new u8[len];
    size_t nread = fread(filedata, (size_t)len, 1, f);
    if (nread != 1)
    {
        fclose(f);
        delete[] filedata;
        return false;
    }

    fclose(f);
    filelen = (u32)len;

    if (GBASave) delete GBASave;
    GBASave = nullptr;

    u32 savelen = 0;
    u8* savedata = nullptr;

    FILE* sav = Platform::OpenFile(sramPath, "rb", true);
    if (sav)
    {
        fseek(sav, 0, SEEK_END);
        savelen = (u32)ftell(sav);

        if (savelen > 0)
        {
            fseek(sav, 0, SEEK_SET);
            savedata = new u8[savelen];
            fread(savedata, savelen, 1, sav);
        }
        fclose(sav);
    }

    bool res = NDS::LoadGBACart(filedata, filelen, savedata, savelen);

    if (res)
    {
        GBACartType = 0;
        GBASave = new SaveManager(sramPath);
    }

    if (savedata) delete[] savedata;
    delete[] filedata;
    return res;
}

void LoadGBAAddon(int type)
{
    if (Config::ConsoleType == 1) return;

    if (GBASave) delete GBASave;
    GBASave = nullptr;

    NDS::LoadGBAAddon(type);

    GBACartType = type;
}

void EjectGBACart()
{
    if (GBASave) delete GBASave;
    GBASave = nullptr;

    NDS::EjectGBACart();

    GBACartType = -1;
}

bool GBACartInserted()
{
    return GBACartType != -1;
}

void ROMIcon(u8 (&data)[512], u16 (&palette)[16], u32* iconRef)
{
    int index = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 8; k++)
            {
                for (int l = 0; l < 8; l++)
                {
                    u8 pal_index = index % 2 ?  data[index/2] >> 4 : data[index/2] & 0x0F;
                    u8 b = ((palette[pal_index] >> 0)  & 0x1F) * 255 / 31;
                    u8 g = ((palette[pal_index] >> 5)  & 0x1F) * 255 / 31;
                    u8 r = ((palette[pal_index] >> 10) & 0x1F) * 255 / 31;
                    u8 a = pal_index ? 255: 0;
                    u32* row = &iconRef[256 * i + 32 * k + 8 * j];
                    row[l] = (a << 24) | (r << 16) | (g << 8) | b;
                    index++;
                }
            }
        }
    }
}

#define SEQ_FLIPV(i) ((i & 0b1000000000000000) >> 15)
#define SEQ_FLIPH(i) ((i & 0b0100000000000000) >> 14)
#define SEQ_PAL(i) ((i & 0b0011100000000000) >> 11)
#define SEQ_BMP(i) ((i & 0b0000011100000000) >> 8)
#define SEQ_DUR(i) ((i & 0b0000000011111111) >> 0)

void AnimatedROMIcon(u8 (&data)[8][512], u16 (&palette)[8][16], u16 (&sequence)[64], u32 (&animatedTexRef)[32 * 32 * 64], std::vector<int> &animatedSequenceRef)
{
    for (int i = 0; i < 64; i++)
    {
        if (!sequence[i])
            break;
        u32* frame = &animatedTexRef[32 * 32 * i];
        ROMIcon(data[SEQ_BMP(sequence[i])], palette[SEQ_PAL(sequence[i])], frame);

        if (SEQ_FLIPH(sequence[i]))
        {
            for (int x = 0; x < 32; x++)
            {
                for (int y = 0; y < 32/2; y++)
                {
                    std::swap(frame[x * 32 + y], frame[x * 32 + (32 - 1 - y)]);
                }
            }
        }
        if (SEQ_FLIPV(sequence[i]))
        {
            for (int x = 0; x < 32/2; x++)
            {
                for (int y = 0; y < 32; y++)
                {
                    std::swap(frame[x * 32 + y], frame[(32 - 1 - x) * 32 + y]);
                }
            }
        }

        for (int j = 0; j < SEQ_DUR(sequence[i]); j++)
            animatedSequenceRef.push_back(i);
    }
}

}
