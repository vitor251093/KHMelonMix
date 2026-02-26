#include "KingdomHeartsHDCollection.h"

#include <iostream>
#include <fstream>
#include <regex>

#include "../Platform.h"

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <shlobj.h>
#endif

namespace Plugins
{

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
    t[0x1C] = 0x01000000 | 0x0004; // Qt::Key_Return
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

int KingdomHeartsHDCollection::DecodeSet1ToQt(u32 sc)
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

int KingdomHeartsHDCollection::DecodeSet1ToQt(KHKey* key)
{
    int val = DecodeSet1ToQt(key->main);
    if (val == -1)
    {
        val = DecodeSet1ToQt(key->sub);
    }
    return val;
}

std::filesystem::path KingdomHeartsHDCollection::userDocumentsFolderPath()
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

std::filesystem::path KingdomHeartsHDCollection::path()
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

std::filesystem::path KingdomHeartsHDCollection::steamConfigFolderPathFromDocumentsPath(const std::filesystem::path& documentsFolderPath)
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

std::filesystem::path KingdomHeartsHDCollection::configFolderPath()
{
    std::filesystem::path collectionFolderPath = path();
    if (collectionFolderPath.empty())
    {
        return collectionFolderPath;
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
        std::filesystem::path empty;
        return empty;
    }

    return steamConfigFolderPathFromDocumentsPath(documentsFolderPath);
}

KingdomHeartsHDCollection::KHMareConfig* KingdomHeartsHDCollection::config()
{
    std::filesystem::path _configFolderPath = configFolderPath();
    if (_configFolderPath.empty())
    {
        return nullptr;
    }

    std::filesystem::path configFilePath = _configFolderPath / "config1525.dat";
    Platform::FileHandle* configFileHandle = Platform::OpenFile(configFilePath.string(), Platform::FileMode::ReadText);

    printf("Config file path: %s\n", configFilePath.string().c_str());

    auto* config = new KHMareConfig();
    Platform::FileRead(config, sizeof(KHMareConfig), 1, configFileHandle);
    Platform::CloseFile(configFileHandle);
    return config;
}

void KingdomHeartsHDCollection::createSignalFile()
{
    std::filesystem::path collectionFolderPath = path();
    if (collectionFolderPath.empty())
    {
        return;
    }

    std::filesystem::path signalFilePath = collectionFolderPath / ".melonmix_signal";
    if (!std::filesystem::exists(signalFilePath))
    {
        Platform::FileHandle* signalHandle = Platform::OpenFile(signalFilePath.string(), Platform::FileMode::Append);
        Platform::FileWrite("\n", 1, 1, signalHandle);
        Platform::CloseFile(signalHandle);
    }
}

std::string KingdomHeartsHDCollection::language()
{
    std::filesystem::path collectionFolderPath = path();
    if (collectionFolderPath.empty())
    {
        return "";
    }

    std::filesystem::path steamappsPath = collectionFolderPath.parent_path().parent_path();

    std::filesystem::path manifestPath = steamappsPath / "appmanifest_2552430.acf";

    if (!exists(manifestPath)) {
        return "";
    }

    std::ifstream file(manifestPath);
    if (!file.is_open()) {
        return "";
    }

    std::string line;
    std::string language = "";
    // Regex to find "language" followed by any whitespace and then the value in quotes
    std::regex langRegex("\"language\"\\s+\"([^\"]+)\"");
    std::smatch match;

    bool found = false;
    while (std::getline(file, line)) {
        if (std::regex_search(line, match, langRegex)) {
            // match[1] contains the first captured group (the language name)
            language = match[1];
            found = true;
            break;
        }
    }

    if (!found) {
        return "";
    }

    return language;
}

void KingdomHeartsHDCollection::applyJoystickMappings(std::function<void(std::string, int)> setIntConfig, bool bAsConfirmButton)
{
    std::map<std::string, std::vector<PluginJoystickInput>> map = {
        {"A",                     {bAsConfirmButton ? PLUGIN_GAME_CONTROLLER_BUTTON_B : PLUGIN_GAME_CONTROLLER_BUTTON_A}},
        {"B",                     {bAsConfirmButton ? PLUGIN_GAME_CONTROLLER_BUTTON_A : PLUGIN_GAME_CONTROLLER_BUTTON_B}},
        {"Y",                     {PLUGIN_GAME_CONTROLLER_BUTTON_Y}},
        {"X",                     {PLUGIN_GAME_CONTROLLER_BUTTON_X}},
        // TODO: KH holdToOpenShortcuts
        {"HK_RLockOn",            {PLUGIN_GAME_CONTROLLER_BUTTON_RIGHTSHOULDER}},
        {"HK_LSwitchTarget",      {PLUGIN_GAME_CONTROLLER_LEFT_TRIGGER}},
        {"HK_RSwitchTarget",      {PLUGIN_GAME_CONTROLLER_RIGHT_TRIGGER}},
        {"Up",                    {PLUGIN_GAME_CONTROLLER_LEFT_AXIS_UP}},
        {"Down",                  {PLUGIN_GAME_CONTROLLER_LEFT_AXIS_DOWN}},
        {"Left",                  {PLUGIN_GAME_CONTROLLER_LEFT_AXIS_LEFT}},
        {"Right",                 {PLUGIN_GAME_CONTROLLER_LEFT_AXIS_RIGHT}},
        // TODO: KH holdToWalk
        {"HK_HUDToggle",          {PLUGIN_GAME_CONTROLLER_BUTTON_LEFTSTICK}},
        {"CameraUp",              {PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_UP}},
        {"CameraDown",            {PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_DOWN}},
        {"CameraLeft",            {PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_LEFT}},
        {"CameraRight",           {PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_RIGHT}},
        // TODO: KH resetCamera
        {"HK_CommandMenuUp",      {PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_UP}},
        {"HK_CommandMenuDown",    {PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_DOWN}},
        {"HK_CommandMenuLeft",    {PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_LEFT}},
        {"HK_CommandMenuRight",   {PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_RIGHT}},
        {"Start",                 {PLUGIN_GAME_CONTROLLER_BUTTON_START}},
        {"HK_FullscreenMapToggle",{PLUGIN_GAME_CONTROLLER_BUTTON_TOUCHPAD, PLUGIN_GAME_CONTROLLER_BUTTON_BACK, PLUGIN_GAME_CONTROLLER_BUTTON_GUIDE}},
        {"HK_AttackInteract",     {}},
        {"HK_Jump",               {}},
        {"HK_GuardCombo",         {}},
        {"Select",                {}},
        {"L",                     {}},
        {"R",                     {}}
    };
    PluginJoystick::applyMappings(setIntConfig, map);
}

void KingdomHeartsHDCollection::applyKeyboardAndJoystickMappings(KHMareConfig* config, std::function<void(std::string, int)> setIntConfig)
{
    // TODO: KH We need to load mouse sensitivity (config.mouseSensitivity) to: plugin->tomlUniqueIdentifier() + ".CameraSensitivity"

    // TODO: KH Disabling the keyboard automatic mapping for now because we need mouse control for the camera for that to work 100%
    /*
    setIntConfig("Instance0.Keyboard.A", DecodeSet1ToQt(&config->keyConfiguration.confirm));
    setIntConfig("Instance0.Keyboard.B", DecodeSet1ToQt(&config->keyConfiguration.cancelOrJump));
    setIntConfig("Instance0.Keyboard.Y", DecodeSet1ToQt(&config->keyConfiguration.blockEvadeDodge));
    setIntConfig("Instance0.Keyboard.X", DecodeSet1ToQt(&config->keyConfiguration.useCommand));
    // TODO: KH holdToOpenShortcuts
    setIntConfig("Instance0.Keyboard.HK_RLockOn",       DecodeSet1ToQt(&config->keyConfiguration.toggleLockOn));
    setIntConfig("Instance0.Keyboard.HK_RSwitchTarget", DecodeSet1ToQt(&config->keyConfiguration.changeLockOnTargetOrToggleCursorControls));
    setIntConfig("Instance0.Keyboard.HK_LSwitchTarget", DecodeSet1ToQt(&config->keyConfiguration.toggleGummiShipScoreOrChangeLockOnTarget));
    setIntConfig("Instance0.Keyboard.Up",    DecodeSet1ToQt(&config->keyConfiguration.up));
    setIntConfig("Instance0.Keyboard.Down",  DecodeSet1ToQt(&config->keyConfiguration.down));
    setIntConfig("Instance0.Keyboard.Left",  DecodeSet1ToQt(&config->keyConfiguration.left));
    setIntConfig("Instance0.Keyboard.Right", DecodeSet1ToQt(&config->keyConfiguration.right));
    // TODO: KH holdToWalk
    setIntConfig("Instance0.Keyboard.HK_HUDToggle", DecodeSet1ToQt(&config->keyConfiguration.gummiEditorFlipGummi));
    setIntConfig("Instance0.Keyboard.CameraUp",    DecodeSet1ToQt(config->keyConfiguration.cameraUp));
    setIntConfig("Instance0.Keyboard.CameraDown",  DecodeSet1ToQt(config->keyConfiguration.cameraDown));
    setIntConfig("Instance0.Keyboard.CameraLeft",  DecodeSet1ToQt(config->keyConfiguration.cameraLeft));
    setIntConfig("Instance0.Keyboard.CameraRight", DecodeSet1ToQt(config->keyConfiguration.cameraRight));
    // TODO: KH resetCamera
    setIntConfig("Instance0.Keyboard.HK_CommandMenuUp",    DecodeSet1ToQt(&config->keyConfiguration.cursorUp));
    setIntConfig("Instance0.Keyboard.HK_CommandMenuDown",  DecodeSet1ToQt(&config->keyConfiguration.cursorDown));
    setIntConfig("Instance0.Keyboard.HK_CommandMenuLeft",  DecodeSet1ToQt(&config->keyConfiguration.cursorLeft));
    setIntConfig("Instance0.Keyboard.HK_CommandMenuRight", DecodeSet1ToQt(&config->keyConfiguration.cursorRight));
    setIntConfig("Instance0.Keyboard.Start", DecodeSet1ToQt(&config->keyConfiguration.pause));
    setIntConfig("Instance0.Keyboard.HK_FullscreenMapToggle", DecodeSet1ToQt(&config->keyConfiguration.firstPersonView));

    setIntConfig("Instance0.Keyboard.HK_AttackInteract", -1);
    setIntConfig("Instance0.Keyboard.HK_Jump",       -1);
    setIntConfig("Instance0.Keyboard.HK_GuardCombo", -1);
    setIntConfig("Instance0.Keyboard.Select", -1);
    setIntConfig("Instance0.Keyboard.L", -1);
    setIntConfig("Instance0.Keyboard.R", -1);
    */

    applyJoystickMappings(setIntConfig, config->confirmButton == 1);
}

}