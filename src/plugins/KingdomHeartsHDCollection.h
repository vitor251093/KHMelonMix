//
// Created by vitor on 1/21/26.
//

#ifndef MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H
#define MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H

#include <functional>
#include <string>
#include <filesystem>
#include <map>

#include "../types.h"

namespace Plugins
{
using namespace melonDS;

class KingdomHeartsHDCollection
{
public:
struct KHKey
{
    u32 main;
    u32 sub;
};

enum Plugin_GameControllerButton
{
    PLUGIN_GAME_CONTROLLER_BUTTON_INVALID = -1,
    PLUGIN_GAME_CONTROLLER_BUTTON_A,
    PLUGIN_GAME_CONTROLLER_BUTTON_B,
    PLUGIN_GAME_CONTROLLER_BUTTON_X,
    PLUGIN_GAME_CONTROLLER_BUTTON_Y,
    PLUGIN_GAME_CONTROLLER_BUTTON_BACK,
    PLUGIN_GAME_CONTROLLER_BUTTON_GUIDE,
    PLUGIN_GAME_CONTROLLER_BUTTON_START,
    PLUGIN_GAME_CONTROLLER_BUTTON_LEFTSTICK,
    PLUGIN_GAME_CONTROLLER_BUTTON_RIGHTSTICK,
    PLUGIN_GAME_CONTROLLER_BUTTON_LEFTSHOULDER,
    PLUGIN_GAME_CONTROLLER_BUTTON_RIGHTSHOULDER,
    PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_UP,
    PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_DOWN,
    PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_LEFT,
    PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_RIGHT,
    PLUGIN_GAME_CONTROLLER_BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE1,  /* Xbox Elite paddle P1 (upper left, facing the back) */
    PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE2,  /* Xbox Elite paddle P3 (upper right, facing the back) */
    PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE3,  /* Xbox Elite paddle P2 (lower left, facing the back) */
    PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE4,  /* Xbox Elite paddle P4 (lower right, facing the back) */
    PLUGIN_GAME_CONTROLLER_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    PLUGIN_GAME_CONTROLLER_LEFT_AXIS_LEFT,
    PLUGIN_GAME_CONTROLLER_LEFT_AXIS_RIGHT,
    PLUGIN_GAME_CONTROLLER_LEFT_AXIS_UP,
    PLUGIN_GAME_CONTROLLER_LEFT_AXIS_DOWN,
    PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_LEFT,
    PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_RIGHT,
    PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_UP,
    PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_DOWN,
    PLUGIN_GAME_CONTROLLER_LEFT_TRIGGER,
    PLUGIN_GAME_CONTROLLER_RIGHT_TRIGGER
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
    u8 confirmButton; // 0 => A/Circle; 1 => B/X
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u32 mouseSensitivity; // values goes from 1 to 60 (default: 30)
    KHKeyboardControls keyConfiguration;

    // TODO: KH there is more after that, starting from 0xF4
};

private:
    static int DecodeSet1ToQt(u32 sc);
    static int DecodeSet1ToQt(KHKey* key);

    static std::filesystem::path userDocumentsFolderPath();
    static std::filesystem::path steamConfigFolderPathFromDocumentsPath(const std::filesystem::path& documentsFolderPath);

    static void applyJoystickMappingsWithMap(std::function<void(std::string, int)> setIntConfig, std::map<std::string, std::vector<Plugin_GameControllerButton>> map);

public:
    static std::filesystem::path path();
    static std::filesystem::path configFolderPath();

    static KHMareConfig* config();

    static void createSignalFile();

    static std::string language();

    static void applyJoystickMappings(std::function<void(std::string, int)> setIntConfig, bool bAsConfirmButton);
    static void applyKeyboardAndJoystickMappings(KHMareConfig* config, std::function<void(std::string, int)> setIntConfig);

};
}

#endif //MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H