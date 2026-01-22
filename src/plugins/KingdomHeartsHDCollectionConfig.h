//
// Created by vitor on 1/21/26.
//

#ifndef MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H
#define MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H

#include <filesystem>
#include "../types.h"

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <shlobj.h>
#endif

namespace Plugins
{
using namespace melonDS;

struct HDCollectionConfig
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    u8 windowMode; // 0x00 => fullscreen; 0x02 => windowed
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u16 masterVolume; // values goes from 1 to 10
    u16 bgmVolume; // values goes from 1 to 10
    u16 sfxVolume; // values goes from 1 to 10
    u16 voicesVolume; // values goes from 1 to 10
    u8 joystickLayout; // 0 => auto; 1 => xbox; 2 => playstation; 3 => generic
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 unk3B;
    u8 unk3C;
    u8 unk3D;
    u8 unk3E;
    u8 unk3F;
    u8 unk40;
    u8 unk41;
    u8 unk42;
    u8 unk43;
    // The controls below follow IBM PC keyboard scancode Set 1: https://www.vetra.com/scancodes.html
    // Other possible values:
    //   0x1000 => left mouse click
    //   0x1001 => right mouse click
    u32 controls_confirm;
    u32 controls_confirm_sub;
    u32 controls_cancel;
    u32 controls_cancel_sub;
    u32 controls_unassigned2;
    u32 controls_unassigned2_sub;
    u32 controls_unassigned1;
    u32 controls_unassigned1_sub;
    u32 controls_unassigned3;
    u32 controls_unassigned3_sub;
    u32 controls_unassigned4;
    u32 controls_unassigned4_sub;
    u32 controls_unassigned5;
    u32 controls_unassigned5_sub;
    u32 controls_unassigned6;
    u32 controls_unassigned6_sub;
    u8 unk8x[12];
    u8 unk9x[16];
    u8 unkAx[16];
    u8 unkB0;
    u8 unkB1;
    u8 unkB2;
    u8 unkB3;
    u32 controls_unassigned_last1;
    u32 controls_unassigned_last1_sub;
    u32 controls_unassigned_last2;
    u32 controls_unassigned_last2_sub;
    u32 controls_up;
    u32 controls_up_sub;
    u32 controls_down;
    u32 controls_down_sub;
    u32 controls_left;
    u32 controls_left_sub;
    u32 controls_right;
    u32 controls_right_sub;

    // TODO: KH there is more after that, starting from 0xE4
};

inline std::filesystem::path myDocumentsFolderPath()
{
#ifdef _WIN32
    wchar_t Folder[1024];
    HRESULT hr = SHGetFolderPathW(0, CSIDL_MYDOCUMENTS, 0, 0, Folder);
    if (SUCCEEDED(hr))
    {
        char str[1024];
        wcstombs(str, Folder, 1023);
        return std::filesystem::u8path(std::string(str));
    }

    std::filesystem::path empty;
    return empty;
#else
    const char* homeDir = std::getenv("HOME");

    if (homeDir == nullptr) {
        std::filesystem::path empty;
        return empty;
    }

    return std::filesystem::u8path(std::string(homeDir)) / "Documents";
#endif
}

inline std::filesystem::path kingdomHeartsCollectionFolderPath()
{
    std::filesystem::path collectionFolderPath;

    const char* assetsPathEnv = std::getenv("KINGDOM_HEARTS_HD_1_5_2_5_REMIX_LOCATION");
    if (assetsPathEnv != nullptr)
    {
        collectionFolderPath = std::filesystem::u8path(std::string(assetsPathEnv));
    }
    else
    {
        std::filesystem::path currentFolder = std::filesystem::current_path();
        if (currentFolder.filename().string() == "KINGDOM HEARTS -HD 1.5+2.5 ReMIX-")
        {
            collectionFolderPath = currentFolder;
        }
    }

    if (collectionFolderPath.empty())
    {
        return collectionFolderPath;
    }

    // TODO: KH That seems way too convoluted
    if (collectionFolderPath.string()[collectionFolderPath.string().size()-1] == '/')
    {
        collectionFolderPath = collectionFolderPath.parent_path();
    }

    if (!std::filesystem::exists(collectionFolderPath))
    {
        std::filesystem::path empty;
        return empty;
    }

    return collectionFolderPath;
}

inline std::filesystem::path kingdomHeartsCollectionSteamConfigFolderPathFromDocuments(const std::filesystem::path& documentsFolderPath)
{
    std::filesystem::path empty;

    std::filesystem::path saveDatasFolderPath = documentsFolderPath /
        "My Games" / "KINGDOM HEARTS HD 1.5+2.5 ReMIX" / "Steam";
    if (!std::filesystem::exists(saveDatasFolderPath))
    {
        return empty;
    }

    std::vector<std::string> saveDataFolderNameList = Platform::ContentsOfFolder(saveDatasFolderPath.string(), true, false);
    if (saveDataFolderNameList.empty())
    {
        return empty;
    }

    return saveDatasFolderPath / saveDataFolderNameList[0];
}

inline HDCollectionConfig* kingdomHeartsCollectionConfig()
{
    std::filesystem::path collectionFolderPath = kingdomHeartsCollectionFolderPath();
    if (collectionFolderPath.empty())
    {
        return nullptr;
    }

#ifdef _WIN32
    std::filesystem::path documentsFolderPath = myDocumentsFolderPath();
#else
    std::filesystem::path documentsFolderPath = collectionFolderPath.parent_path().parent_path() /
        "compatdata" / "2552430" / "pfx" /
        "drive_c" / "users" / "steamuser" / "Documents";
#endif
    if (!std::filesystem::exists(documentsFolderPath))
    {
        return nullptr;
    }

    std::filesystem::path configFolderPath = kingdomHeartsCollectionSteamConfigFolderPathFromDocuments(documentsFolderPath);
    if (configFolderPath.empty())
    {
        return nullptr;
    }

    std::filesystem::path configFilePath = configFolderPath / "config1525.dat";
    Platform::FileHandle* configFileHandle = Platform::OpenFile(configFilePath.string(), Platform::FileMode::ReadText);

    printf("Config file path: %s\n", configFilePath.string().c_str());

    auto* config = new HDCollectionConfig();
    Platform::FileRead(config, sizeof(HDCollectionConfig), 1, configFileHandle);
    Platform::CloseFile(configFileHandle);
    return config;
}

}

#endif //MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H