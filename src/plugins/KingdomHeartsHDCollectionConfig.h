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
    // Mouse related values:
    //   0x1000 => left mouse click
    //   0x1001 => right mouse click
    //   0x1002 => middle button click
    //   0x1003 => forward mouse button click
    //   0x1004 => mouse scroll up
    //   0x1005 => move mouse up
    //   0x1006 => move mouse down
    //   0x1007 => move mouse left
    //   0x1008 => move mouse right
    //   0x1009 => backward mouse button click
    //   0x100A => mouse scroll down

    KHKey holdToWalk;           // 0x3C / unassigned7
    KHKey confirm;              // 0x44
    KHKey cancelOrJump;         // 0x4C
    KHKey useCommand;           // 0x54 / unassigned2
    KHKey blockEvadeDodge;      // 0x5C / unassigned1
    KHKey holdToOpenShortcuts;  // 0x64 / unassigned3
    KHKey toggleLockOn;         // 0x6C / unassigned4
    KHKey changeLockOnTargetOrToggleCursorControls; // 0x74 / unassigned5
    KHKey toggleGummiShipScoreOrChangeLockOnTarget; // 0x7C / unassigned6
    KHKey gummiEditorFlipGummi; // 0x84 / unassigned8
    KHKey resetCamera;          // 0x8C / unassigned13

    // move cursor
    KHKey cursorUp;    // 0x94
    KHKey cursorDown;  // 0x9C
    KHKey cursorLeft;  // 0xA4
    KHKey cursorRight; // 0xAC

    KHKey firstPersonView; // 0xB4 / unassigned15
    KHKey pause;           // 0xBC / unassigned14
    KHKey up;              // 0xC4
    KHKey down;            // 0xCC
    KHKey left;            // 0xD4
    KHKey right;           // 0xDC

    // move camera
    u32 cameraUp;    // 0xE4 / unassigned9
    u32 cameraDown;  // 0xE8 / unassigned10
    u32 cameraLeft;  // 0xEC / unassigned11
    u32 cameraRight; // 0xF0 / unassigned12
};

struct KHSoundSettings
{
    u16 masterVolume; // values goes from 1 to 10
    u16 bgmVolume; // values goes from 1 to 10
    u16 sfxVolume; // values goes from 1 to 10
    u16 voicesVolume; // values goes from 1 to 10
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
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    u8 windowMode; // 0x00 => fullscreen; 0x02 => windowed
    u8 unk19;
    u16 resolutionWidth;
    u16 resolutionHeight;
    u8 frameRefreshRate; // 0 => 30; 1 => 60; 2 => 120; 3 => unlock
    u8 unk1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 unk26;
    KHSoundSettings sound;
    u8 joystickButtonIcons; // 0 => auto; 1 => xbox; 2 => playstation; 3 => generic
    u8 unk31;
    u8 confirmButton; // 0 => A; 1 => B
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u32 mouseSensitivity; // values goes from 1 to 60 (default: 30)
    KHKeyboardControls keyConfiguration;

    // TODO: KH there is more after that, starting from 0xF4
};

static constexpr std::array<int, 256> Set1ToQtKey = [] {
    std::array<int, 256> t{};

    t[0x29] = '`';
    t[0x02] = '1'; t[0x03] = '2'; t[0x04] = '3'; t[0x05] = '4';
    t[0x06] = '5'; t[0x07] = '6'; t[0x08] = '7'; t[0x09] = '8';
    t[0x0A] = '9'; t[0x0B] = '0';
    t[0x0C] = '-'; t[0x0D] = '=';
    t[0x0E] = 0x01000000 | 0x0003; // Qt::Key_Backspace

    t[0x0F] = 0x01000000 | 0x0002; // Qt::Key_Tab
    t[0x10] = 'Q'; t[0x11] = 'W'; t[0x12] = 'E'; t[0x13] = 'R'; t[0x14] = 'T';
    t[0x15] = 'Y'; t[0x16] = 'U'; t[0x17] = 'I'; t[0x18] = 'O'; t[0x19] = 'P';
    t[0x1A] = '['; t[0x1B] = ']';

    t[0x3A] = 0x01000000 | 0x0024; // Qt::Key_CapsLock
    t[0x1E] = 'A'; t[0x1F] = 'S'; t[0x20] = 'D'; t[0x21] = 'F'; t[0x22] = 'G';
    t[0x23] = 'H'; t[0x24] = 'J'; t[0x25] = 'K'; t[0x26] = 'L';
    t[0x27] = ';'; t[0x28] = '\'';
    t[0x1C] = 0x01000000 | 0x0005; // Qt::Key_Return
    t[0x2A] = 0x01000000 | 0x0020; // Qt::Key_Shift

    t[0x2C] = 'Z'; t[0x2D] = 'X'; t[0x2E] = 'C'; t[0x2F] = 'V'; t[0x30] = 'B';
    t[0x31] = 'N'; t[0x32] = 'M';
    t[0x33] = ','; t[0x34] = '.'; t[0x35] = '/';

    t[0x36] = 0x01000000 | 0x0020; // Qt::Key_Shift
    t[0x1D] = 0x01000000 | 0x0021; // Qt::Key_Control
    t[0x38] = 0x01000000 | 0x0023; // Qt::Key_Alt
    t[0x39] = ' ';                 // Space
    t[0xB8] = 0x01000000 | 0x1103; // right alt
    // right ctrl

    //t[0x??] = 0x01000000 | 0x0006; // insert
    //t[0x??] = 0x01000000 | 0x0007; // delete
    t[0xC8] = 0x01000000 | 0x0012; // left arrow
    //t[0x??] = 0x01000000 | 0x0010; // home
    //t[0x??] = 0x01000000 | 0x0011; // end
    t[0xCD] = 0x01000000 | 0x0013; // up arrow
    t[0xD0] = 0x01000000 | 0x0015; // down arrow
    //t[0x??] = 0x01000000 | 0x0016; // page up
    //t[0x??] = 0x01000000 | 0x0017; // page down
    t[0xCB] = 0x01000000 | 0x0014; // right arrow

    t[0x01] = 0x01000000 | 0x0001; // Qt::Key_Escape
    t[0x3B] = 0x01000000 | 0x0030; // F1
    t[0x3C] = 0x01000000 | 0x0031; // F2
    t[0x3D] = 0x01000000 | 0x0032; // F3
    t[0x3E] = 0x01000000 | 0x0033; // F4
    t[0x3F] = 0x01000000 | 0x0034; // F5
    t[0x40] = 0x01000000 | 0x0035; // F6
    t[0x41] = 0x01000000 | 0x0036; // F7
    t[0x42] = 0x01000000 | 0x0037; // F8
    t[0x43] = 0x01000000 | 0x0038; // F9
    t[0x44] = 0x01000000 | 0x0039; // F10
    t[0x57] = 0x01000000 | 0x003A; // F11
    t[0x58] = 0x01000000 | 0x003B; // F12
    // print screen
    t[0x46] = 0x01000000 | 0x0026; // scroll lock
    //t[0x??] = 0x01000000 | 0x0008; // pause break
    t[0x2B] = '\\';

    return t;
}();

inline int DecodeSet1ToQt(u32 sc)
{
    if (sc > 255)
    {
        return -1;
    }
    int val = Set1ToQtKey[sc];
    if (val == 0)
    {
        return -1;
    }
    return val;
}

inline int DecodeSet1ToQt(KHKey* key)
{
    int val = DecodeSet1ToQt(key->main);
    if (val == -1)
    {
        val = DecodeSet1ToQt(key->sub);
    }
    return val;
}

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