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

struct KHKey
{
    u32 main;
    u32 sub;
};

struct KHKeyboardControls
{
    // The controls below follow IBM PC keyboard scancode Set 1: https://www.vetra.com/scancodes.html
    // Other possible values:
    //   0x1000 => left mouse click
    //   0x1001 => right mouse click
    //   0x1002 => middle button click
    //   0x1003 => ?
    //   0x1004 => ?
    //   0x100A => ?

    KHKey unassigned7;  // 0x3C
    KHKey confirm;      // 0x44
    KHKey cancel;       // 0x4C
    KHKey unassigned2;  // 0x54
    KHKey unassigned1;  // 0x5C
    KHKey unassigned3;  // 0x64
    KHKey unassigned4;  // 0x6C
    KHKey unassigned5;  // 0x74
    KHKey unassigned6;  // 0x7C
    KHKey unassigned8;  // 0x84
    KHKey unassigned13; // 0x8C

    // command menu controls ("Move cursor" in KH Collection settings)
    KHKey unk94;
    KHKey unk9C;
    KHKey unkA4;
    KHKey unkAC;

    KHKey unassigned15; // 0xB4
    KHKey unassigned14; // 0xBC
    KHKey up;           // 0xC4
    KHKey down;         // 0xCC
    KHKey left;         // 0xD4
    KHKey right;        // 0xDC

    // move camera (unassigned 9 to 12)
    u32 unkE4;
    u32 unkE8;
    u32 unkEC;
    u32 unkF0;
};

struct KHMareConfig
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
    u16 resolutionWidth;
    u16 resolutionHeight;
    u8 windowMode; // 0x00 => fullscreen; 0x02 => windowed
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    u8 unk1C;
    u8 unk1D;
    u8 frameRefreshRate; // 0 => 30; 1 => 60; 2 => 120; 3 => unlock
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
    u8 confirmButton; // 0 => A; 1 => B
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u32 mouseSensitivity; // values goes from 1 to 60 (default: 30)
    KHKeyboardControls controls;

    // TODO: KH there is more after that, starting from 0xF4
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

inline KHMareConfig* kingdomHeartsCollectionConfig()
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

    auto* config = new KHMareConfig();
    Platform::FileRead(config, sizeof(KHMareConfig), 1, configFileHandle);
    Platform::CloseFile(configFileHandle);
    return config;
}

}

#endif //MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H