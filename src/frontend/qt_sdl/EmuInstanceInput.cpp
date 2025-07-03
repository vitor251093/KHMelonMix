/*
    Copyright 2016-2025 melonDS team

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

#include <QKeyEvent>
#include <SDL2/SDL.h>

#include "Platform.h"
#include "SDL_gamecontroller.h"
#include "SDL_sensor.h"
#include "main.h"
#include "Config.h"

using namespace melonDS;

const char* EmuInstance::buttonNames[12] =
{
    "A",
    "B",
    "Select",
    "Start",
    "Right",
    "Left",
    "Up",
    "Down",
    "R",
    "L",
    "X",
    "Y"
};

const char* EmuInstance::hotkeyNames[HK_MAX] =
{
    "HK_Lid",
    "HK_Mic",
    "HK_Pause",
    "HK_Reset",
    "HK_FastForward",
    "HK_FrameLimitToggle",
    "HK_FullscreenToggle",
    "HK_SwapScreens",
    "HK_SwapScreenEmphasis",
    "HK_SolarSensorDecrease",
    "HK_SolarSensorIncrease",
    "HK_FrameStep",
    "HK_PowerButton",
    "HK_VolumeUp",
    "HK_VolumeDown",
    "HK_SlowMo",
    "HK_FastForwardToggle",
    "HK_SlowMoToggle",
    "HK_GuitarGripGreen",
    "HK_GuitarGripRed",
    "HK_GuitarGripYellow",
    "HK_GuitarGripBlue"
};

const char* EmuInstance::touchButtonNames[4] =
{
    "CameraRight",
    "CameraLeft",
    "CameraUp",
    "CameraDown"
};


void EmuInstance::inputInit()
{
    keyInputMask = 0xFFF;
    joyInputMask = 0xFFF;
    inputMask = 0xFFF;

    keyTouchInputMask = 0xFFFF;
    joyTouchInputMask = 0xFFFF;
    touchInputMask = 0xFFFF;

    keyHotkeyMask = 0;
    joyHotkeyMask = 0;
    hotkeyMask = 0;
    lastHotkeyMask = 0;

    keyPluginMask = 0;
    joyPluginMask = 0;
    pluginMask = 0;
    lastPluginMask = 0;

    isTouching = false;
    touchX = 0;
    touchY = 0;

    joystick = nullptr;
    controller = nullptr;
    hasRumble = false;
    hasAccelerometer = false;
    hasGyroscope = false;
    isRumbling = false;
    inputLoadConfig();
}

void EmuInstance::inputDeInit()
{
    closeJoystick();
}

void EmuInstance::inputLoadConfig()
{
    Config::Table keycfg = localCfg.GetTable("Keyboard");
    Config::Table joycfg = localCfg.GetTable("Joystick");

    for (int i = 0; i < 12; i++)
    {
        keyMapping[i] = keycfg.GetInt(buttonNames[i]);
        joyMapping[i] = joycfg.GetInt(buttonNames[i]);
    }

    for (int i = 0; i < HK_MAX; i++)
    {
        hkKeyMapping[i] = keycfg.GetInt(hotkeyNames[i]);
        hkJoyMapping[i] = joycfg.GetInt(hotkeyNames[i]);
    }

    if (plugin != nullptr && plugin->isReady())
    {
        for (int i = 0; i < plugin->customKeyMappingNames.size(); i++)
        {
            const char* name = plugin->customKeyMappingNames[i];
            pluginKeyMapping[i] = keycfg.GetInt(name);
            pluginJoyMapping[i] = joycfg.GetInt(name);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        touchKeyMapping[i] = keycfg.GetInt(touchButtonNames[i]);
        touchJoyMapping[i] = joycfg.GetInt(touchButtonNames[i]);
    }

    setJoystick(localCfg.GetInt("JoystickID"));
}

void EmuInstance::inputRumbleStart(melonDS::u32 len_ms)
{
    if (controller && hasRumble && !isRumbling)
    {
	SDL_GameControllerRumble(controller, 0xFFFF, 0xFFFF, len_ms);
	isRumbling = true;
    }
}

void EmuInstance::inputRumbleStop()
{
    if (controller && hasRumble && isRumbling)
    {
	SDL_GameControllerRumble(controller, 0, 0, 0);
	isRumbling = false;
    }
}

float EmuInstance::inputMotionQuery(melonDS::Platform::MotionQueryType type)
{
    float values[3];
    if (type <= melonDS::Platform::MotionAccelerationZ)
    {
        if (controller && hasAccelerometer)
            if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_ACCEL, values, 3) == 0)
            {
                // Map values from DS console orientation to SDL controller orientation.
                switch (type)
                {
                case melonDS::Platform::MotionAccelerationX:
                    return values[0];
                case melonDS::Platform::MotionAccelerationY:
                    return -values[2];
                case melonDS::Platform::MotionAccelerationZ:
                    return values[1];
                }
            }
    }
    else if (type <= melonDS::Platform::MotionRotationZ)
    {
        if (controller && hasGyroscope)
            if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, values, 3) == 0)
            {
                // Map values from DS console orientation to SDL controller orientation.
                switch (type)
                {
                case melonDS::Platform::MotionRotationX:
                    return values[0];
                case melonDS::Platform::MotionRotationY:
                    return -values[2];
                case melonDS::Platform::MotionRotationZ:
                    return values[1];
                }
            }
    }
    if (type == melonDS::Platform::MotionAccelerationZ)
        return SDL_STANDARD_GRAVITY;
    return 0.0f;
}


void EmuInstance::setJoystick(int id)
{
    joystickID = id;
    openJoystick();
}

void EmuInstance::setAutoJoystickConfig(int a, int b, int select, int start, int right, int left, int up, int down, int r, int l, int x, int y,
                                        int camRight, int camLeft, int camUp, int camDown,
                                        int cmdLeft, int cmdRight, int cmdUp, int cmdDown,
                                        int pause, int fullscreen)
{
    bool shouldNotUpdate = (joyMapping[0] == -1) && (joyMapping[1]  == -1) && (joyMapping[2]  == -1) &&
                           (joyMapping[3] == -1) && (joyMapping[4]  == -1) && (joyMapping[5]  == -1) &&
                           (joyMapping[6] == -1) && (joyMapping[7]  == -1) && (joyMapping[8]  == -1) &&
                           (joyMapping[9] == -1) && (joyMapping[10] == -1) && (joyMapping[11] == -1);
    if (shouldNotUpdate) {
        return;
    }

    joyMapping[0] = a;
    joyMapping[1] = b;
    joyMapping[2] = select;
    joyMapping[3] = start;
    joyMapping[4] = right;
    joyMapping[5] = left;
    joyMapping[6] = up;
    joyMapping[7] = down;
    joyMapping[8] = r;
    joyMapping[9] = l;
    joyMapping[10] = x;
    joyMapping[11] = y;

    touchJoyMapping[0] = camRight;
    touchJoyMapping[1] = camLeft;
    touchJoyMapping[2] = camUp;
    touchJoyMapping[3] = camDown;

    hkJoyMapping[HK_Lid] = -1;
    hkJoyMapping[HK_Mic] = -1;
    hkJoyMapping[HK_Pause] = pause;
    hkJoyMapping[HK_Reset] = -1;
    hkJoyMapping[HK_FastForward] = -1;
    hkJoyMapping[HK_FastForwardToggle] = -1;
    hkJoyMapping[HK_FullscreenToggle] = fullscreen;
    hkJoyMapping[HK_SwapScreens] = -1;
    hkJoyMapping[HK_SwapScreenEmphasis] = -1;
    hkJoyMapping[HK_SolarSensorDecrease] = -1;
    hkJoyMapping[HK_SolarSensorIncrease] = -1;
    hkJoyMapping[HK_FrameStep] = -1;
    hkJoyMapping[HK_PowerButton] = -1;
    hkJoyMapping[HK_VolumeUp] = -1;
    hkJoyMapping[HK_VolumeDown] = -1;

    // hkJoyMapping[HK_HUDToggle] = -1;
    // hkJoyMapping[HK_RLockOn] = -1;
    // hkJoyMapping[HK_LSwitchTarget] = -1;
    // hkJoyMapping[HK_RSwitchTarget] = -1;
    // hkJoyMapping[HK_CommandMenuLeft] = cmdLeft;
    // hkJoyMapping[HK_CommandMenuRight] = cmdRight;
    // hkJoyMapping[HK_CommandMenuUp] = cmdUp;
    // hkJoyMapping[HK_CommandMenuDown] = cmdDown;
}

void EmuInstance::autoMapJoystick()
{
    int JoystickVendorID = SDL_JoystickGetDeviceVendor(joystickID);
    int JoystickDeviceID = SDL_JoystickGetDeviceProduct(joystickID);

    printf("Joystick - Vendor ID %04x - Device ID %04x\n", JoystickVendorID, JoystickDeviceID);
    if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x0268) { // PS3 Controller

    }
    if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x05c4) { // PS4 Controller V1

    }
    if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x09cc) { // PS4 Controller V2
        setAutoJoystickConfig(1, 0, 4, 6, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 86048778, 69271561, 3, 2,
                                0x201FFFF, 0x211FFFF, 0x311FFFF, 0x301FFFF,
                                0x102, 0x108, 0x101, 0x104,
                                69271559, 86048776);
    }
    if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x028e) { // Xbox 360 Controller (Wired)
        setAutoJoystickConfig(1, 0, 6, 7, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 86048773, 35717124, 3, 2,
                                0x301FFFF, 0x311FFFF, 0x411FFFF, 0x401FFFF,
                                0x102, 0x108, 0x101, 0x104,
                                9, 10);
    }
    if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x028f) { // Xbox 360 Controller (Wireless)

    }
    if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x02d1) { // Xbox One Controller

    }
    if (JoystickVendorID == 0x28de) { // Valve controllers
        setAutoJoystickConfig(0, 1, 6, 7, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 5, 4, 3, 2,
                                0x301FFFF, 0x311FFFF, 0x411FFFF, 0x401FFFF,
                                0x102, 0x108, 0x101, 0x104,
                                0x221FFFF, 0x521FFFF);
    }
}

void EmuInstance::openJoystick()
{
    if (controller) SDL_GameControllerClose(controller);

    if (joystick) SDL_JoystickClose(joystick);

    int num = SDL_NumJoysticks();
    if (num < 1)
    {
	controller = nullptr;
        joystick = nullptr;
	hasRumble = false;
    hasAccelerometer = false;
    hasGyroscope = false;
        return;
    }

    if (joystickID >= num)
        joystickID = 0;

    joystick = SDL_JoystickOpen(joystickID);

    if (SDL_IsGameController(joystickID))
    {
	controller = SDL_GameControllerOpen(joystickID);
    }

    if (controller)
    {
	if (SDL_GameControllerHasRumble(controller))
    {
	    hasRumble = true;
    }
	if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL))
    {
	    hasAccelerometer = SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE) == 0;
    }
	if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO))
    {
	    hasGyroscope = SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE) == 0;
    }
    }
}

void EmuInstance::closeJoystick()
{
    if (controller)
    {
	SDL_GameControllerClose(controller);
	controller = nullptr;
	hasRumble = false;
    hasAccelerometer = false;
    hasGyroscope = false;
    }
    if (joystick)
    {
        SDL_JoystickClose(joystick);
        joystick = nullptr;
    }
}


// distinguish between left and right modifier keys (Ctrl, Alt, Shift)
// Qt provides no real cross-platform way to do this, so here we go
// for Windows and Linux we can distinguish via scancodes (but both
// provide different scancodes)
bool isRightModKey(QKeyEvent* event)
{
#ifdef __WIN32__
    quint32 scan = event->nativeScanCode();
    return (scan == 0x11D || scan == 0x138 || scan == 0x36);
#elif __APPLE__
    quint32 scan = event->nativeVirtualKey();
    return (scan == 0x36 || scan == 0x3C || scan == 0x3D || scan == 0x3E);
#else
    quint32 scan = event->nativeScanCode();
    return (scan == 0x69 || scan == 0x6C || scan == 0x3E);
#endif
}

int getEventKeyVal(QKeyEvent* event)
{
    int key = event->key();
    int mod = event->modifiers();
    bool ismod = (key == Qt::Key_Control ||
                  key == Qt::Key_Alt ||
                  key == Qt::Key_AltGr ||
                  key == Qt::Key_Shift ||
                  key == Qt::Key_Meta);

    if (!ismod)
        key |= mod;
    else if (isRightModKey(event))
        key |= (1<<31);

    return key;
}


void EmuInstance::onKeyPress(QKeyEvent* event)
{
    int keyHK = getEventKeyVal(event);
    int keyKP = keyHK;
    if (event->modifiers() != Qt::KeypadModifier)
        keyKP &= ~event->modifiers();

    for (int i = 0; i < 12; i++)
        if (keyKP == keyMapping[i])
            keyInputMask &= ~(1<<i);

    for (int i = 0; i < HK_MAX; i++)
        if (keyHK == hkKeyMapping[i])
            keyHotkeyMask |= (1<<i);
    
    if (plugin != nullptr && plugin->isReady())
    {
        for (int i = 0; i < plugin->customKeyMappingNames.size(); i++)
            if (keyHK == pluginKeyMapping[i])
                keyPluginMask |= (1<<i);
    }
    
    for (int i = 0; i < 4; i++)
        if (keyKP == touchKeyMapping[i])
            keyTouchInputMask &= ~(0xF << (i*4));
}

void EmuInstance::onKeyRelease(QKeyEvent* event)
{
    int keyHK = getEventKeyVal(event);
    int keyKP = keyHK;
    if (event->modifiers() != Qt::KeypadModifier)
        keyKP &= ~event->modifiers();

    for (int i = 0; i < 12; i++)
        if (keyKP == keyMapping[i])
            keyInputMask |= (1<<i);

    for (int i = 0; i < HK_MAX; i++)
        if (keyHK == hkKeyMapping[i])
            keyHotkeyMask &= ~(1<<i);

    if (plugin != nullptr && plugin->isReady())
    {
        for (int i = 0; i < plugin->customKeyMappingNames.size(); i++)
            if (keyHK == pluginKeyMapping[i])
                keyPluginMask &= ~(1<<i);
    }

    for (int i = 0; i < 4; i++)
        if (keyKP == touchKeyMapping[i])
            keyTouchInputMask |= (0xF << (i*4));
}

void EmuInstance::keyReleaseAll()
{
    keyInputMask = 0xFFF;
    keyHotkeyMask = 0;
    keyPluginMask = 0;
}

Sint16 EmuInstance::joystickButtonDown(int val)
{
    if (val == -1) return 0;

    bool hasbtn = ((val & 0xFFFF) != 0xFFFF);

    if (hasbtn)
    {
        if (val & 0x100)
        {
            int hatnum = (val >> 4) & 0xF;
            int hatdir = val & 0xF;
            Uint8 hatval = SDL_JoystickGetHat(joystick, hatnum);

            bool pressed = false;
            if      (hatdir == 0x1) pressed = (hatval & SDL_HAT_UP);
            else if (hatdir == 0x4) pressed = (hatval & SDL_HAT_DOWN);
            else if (hatdir == 0x2) pressed = (hatval & SDL_HAT_RIGHT);
            else if (hatdir == 0x8) pressed = (hatval & SDL_HAT_LEFT);

            if (pressed) return 1;
        }
        else
        {
            int btnnum = val & 0xFFFF;
            Uint8 btnval = SDL_JoystickGetButton(joystick, btnnum);

            if (btnval) return 1;
        }
    }

    if (val & 0x10000)
    {
        int axisnum = (val >> 24) & 0xF;
        int axisdir = (val >> 20) & 0xF;
        Sint16 axisval = SDL_JoystickGetAxis(joystick, axisnum); // from -32768 to 32767

        switch (axisdir)
        {
        case 0: // positive
            if (axisval > 16384) return (axisval >> 10);
            break;

        case 1: // negative
            if (axisval < -16384) return ((~axisval) >> 10);
            break;

        case 2: // trigger
            if (axisval > 0) return 1;
            break;
        }
    }

    return 0;
}

void EmuInstance::inputProcess()
{
    SDL_JoystickUpdate();

    if (joystick)
    {
        if (!SDL_JoystickGetAttached(joystick))
        {
            SDL_JoystickClose(joystick);
            joystick = nullptr;
        }
    }
    if (!joystick && (SDL_NumJoysticks() > 0))
    {
        openJoystick();
    }

    joyInputMask = 0xFFF;
    joyTouchInputMask = 0xFFFF;
    if (joystick)
    {
        for (int i = 0; i < 12; i++)
            if (joystickButtonDown(joyMapping[i]))
                joyInputMask &= ~(1 << i);
        for (int i = 0; i < 4; i++) {
            Sint16 joyValue = joystickButtonDown(touchJoyMapping[i]);
            if (joyValue != 0)
                joyTouchInputMask &= ~(joyValue << (i*4));
        }
    }

    inputMask = keyInputMask & joyInputMask;
    touchInputMask = keyTouchInputMask & joyTouchInputMask;

    joyPluginMask = 0;
    if (joystick)
    {
        if (plugin != nullptr && plugin->isReady())
        {
            for (int i = 0; i < plugin->customKeyMappingNames.size(); i++)
                if (joystickButtonDown(pluginJoyMapping[i]))
                    joyPluginMask |= (1 << i);
        }
    }

    pluginMask = keyPluginMask | joyPluginMask;
    pluginPress = pluginMask & ~lastPluginMask;
    pluginRelease = lastPluginMask & ~pluginMask;
    lastPluginMask = pluginMask;

    joyHotkeyMask = 0;
    if (joystick)
    {
        for (int i = 0; i < HK_MAX; i++)
            if (joystickButtonDown(hkJoyMapping[i]))
                joyHotkeyMask |= (1 << i);
    }

    hotkeyMask = keyHotkeyMask | joyHotkeyMask;
    hotkeyPress = hotkeyMask & ~lastHotkeyMask;
    hotkeyRelease = lastHotkeyMask & ~hotkeyMask;
    lastHotkeyMask = hotkeyMask;
}

void EmuInstance::touchScreen(int x, int y)
{
    touchX = x;
    touchY = y;
    isTouching = true;
}

void EmuInstance::releaseScreen()
{
    isTouching = false;
}
