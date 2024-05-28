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

#include <QKeyEvent>
#include <SDL2/SDL.h>

#include "Input.h"
#include "Config.h"

using namespace melonDS;

namespace Input
{

int JoystickID;
int JoystickVendorID;
int JoystickDeviceID;
SDL_Joystick* Joystick = nullptr;

u32 KeyInputMask, JoyInputMask;
u32 KeyTouchInputMask, JoyTouchInputMask;
u32 KeyHotkeyMask, JoyHotkeyMask;
u32 KeyCmdMenuInputMask, JoyCmdMenuInputMask;
u32 HotkeyMask, LastHotkeyMask;
u32 HotkeyPress, HotkeyRelease;

u32 InputMask, TouchInputMask;
u32 CmdMenuInputMask, PriorCmdMenuInputMask, PriorPriorCmdMenuInputMask;


void Init()
{
    KeyInputMask = 0xFFF;
    JoyInputMask = 0xFFF;
    KeyTouchInputMask = 0xFFFF;
    JoyTouchInputMask = 0xFFFF;
    InputMask = 0xFFF;
    TouchInputMask = 0xFFFF;

    KeyHotkeyMask = 0;
    JoyHotkeyMask = 0;
    HotkeyMask = 0;
    LastHotkeyMask = 0;

    KeyCmdMenuInputMask = 0;
    JoyCmdMenuInputMask = 0;
    CmdMenuInputMask = 0;
    PriorCmdMenuInputMask = 0;
    PriorPriorCmdMenuInputMask = 0;
}


void SetAutoJoystickConfig(int a, int b, int select, int start, int right, int left, int up, int down, int r, int l, int x, int y,
                           int camRight, int camLeft, int camUp, int camDown,
                           int cmdLeft, int cmdRight, int cmdUp, int cmdDown,
                           int pause, int fullscreen)
{
    bool shouldUpdate = (Config::JoyMapping[0] == -1) && (Config::JoyMapping[1]  && -1) || (Config::JoyMapping[2]  && -1) ||
                        (Config::JoyMapping[3] == -1) && (Config::JoyMapping[4]  && -1) || (Config::JoyMapping[5]  && -1) ||
                        (Config::JoyMapping[6] == -1) && (Config::JoyMapping[7]  && -1) || (Config::JoyMapping[8]  && -1) ||
                        (Config::JoyMapping[9] == -1) && (Config::JoyMapping[10] && -1) || (Config::JoyMapping[11] && -1);
    if (!shouldUpdate) {
        return;
    }

    Config::JoyMapping[0] = a;
    Config::JoyMapping[1] = b;
    Config::JoyMapping[2] = select;
    Config::JoyMapping[3] = start;
    Config::JoyMapping[4] = right;
    Config::JoyMapping[5] = left;
    Config::JoyMapping[6] = up;
    Config::JoyMapping[7] = down;
    Config::JoyMapping[8] = r;
    Config::JoyMapping[9] = l;
    Config::JoyMapping[10] = x;
    Config::JoyMapping[11] = y;

    Config::TouchJoyMapping[0] = camRight;
    Config::TouchJoyMapping[1] = camLeft;
    Config::TouchJoyMapping[2] = camUp;
    Config::TouchJoyMapping[3] = camDown;

    Config::CmdMenuJoyMapping[0] = cmdLeft;
    Config::CmdMenuJoyMapping[1] = cmdRight;
    Config::CmdMenuJoyMapping[2] = cmdUp;
    Config::CmdMenuJoyMapping[3] = cmdDown;

    Config::HKJoyMapping[HK_Lid] = -1;
    Config::HKJoyMapping[HK_Mic] = -1;
    Config::HKJoyMapping[HK_Pause] = pause;
    Config::HKJoyMapping[HK_Reset] = -1;
    Config::HKJoyMapping[HK_FastForward] = -1;
    Config::HKJoyMapping[HK_FastForwardToggle] = -1;
    Config::HKJoyMapping[HK_FullscreenToggle] = fullscreen;
    Config::HKJoyMapping[HK_SwapScreens] = -1;
    Config::HKJoyMapping[HK_SwapScreenEmphasis] = -1;
    Config::HKJoyMapping[HK_SolarSensorDecrease] = -1;
    Config::HKJoyMapping[HK_SolarSensorIncrease] = -1;
    Config::HKJoyMapping[HK_FrameStep] = -1;
    Config::HKJoyMapping[HK_PowerButton] = -1;
    Config::HKJoyMapping[HK_VolumeUp] = -1;
    Config::HKJoyMapping[HK_VolumeDown] = -1;
    Config::HKJoyMapping[HK_HUDToggle] = -1;
}
void OpenJoystick(bool autoMapping)
{
    if (Joystick) SDL_JoystickClose(Joystick);

    int num = SDL_NumJoysticks();
    if (num < 1)
    {
        Joystick = nullptr;
        return;
    }

    if (JoystickID >= num)
        JoystickID = 0;

    Joystick = SDL_JoystickOpen(JoystickID);

    JoystickVendorID = SDL_JoystickGetDeviceVendor(JoystickID);
    JoystickDeviceID = SDL_JoystickGetDeviceProduct(JoystickID);

    printf("Joystick - Vendor ID %04x - Device ID %04x\n", JoystickVendorID, JoystickDeviceID);
    if (autoMapping)
    {
        if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x0268) { // PS3 Controller

        }
        if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x05c4) { // PS4 Controller V1

        }
        if (JoystickVendorID == 0x054c && JoystickDeviceID == 0x09cc) { // PS4 Controller V2
            SetAutoJoystickConfig(1, 0, 4, 6, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 86048778, 69271561, 3, 2,
                                  0x201FFFF, 0x211FFFF, 0x311FFFF, 0x301FFFF,
                                  0x102, 0x108, 0x101, 0x104,
                                  69271559, 86048776);
        }
        if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x028e) { // Xbox 360 Controller (Wired)
            SetAutoJoystickConfig(1, 0, 6, 7, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 86048773, 35717124, 3, 2,
                                  0x301FFFF, 0x311FFFF, 0x411FFFF, 0x401FFFF,
                                  0x102, 0x108, 0x101, 0x104,
                                  9, 10);
        }
        if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x028f) { // Xbox 360 Controller (Wireless)

        }
        if (JoystickVendorID == 0x045e && JoystickDeviceID == 0x02d1) { // Xbox One Controller

        }
        if (JoystickVendorID == 0x28de) { // Valve controllers
            SetAutoJoystickConfig(0, 1, 6, 7, 0x001FFFF, 0x011FFFF, 0x111FFFF, 0x101FFFF, 5, 4, 3, 2,
                                  0x301FFFF, 0x311FFFF, 0x411FFFF, 0x401FFFF,
                                  0x102, 0x108, 0x101, 0x104,
                                  0x221FFFF, 0x521FFFF);
        }
    }
}

void CloseJoystick()
{
    if (Joystick)
    {
        SDL_JoystickClose(Joystick);
        Joystick = nullptr;
    }
}


int GetEventKeyVal(QKeyEvent* event)
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
    else if (Input::IsRightModKey(event))
        key |= (1<<31);

    return key;
}

void KeyPress(QKeyEvent* event)
{
    int keyHK = GetEventKeyVal(event);
    int keyKP = keyHK;
    if (event->modifiers() != Qt::KeypadModifier)
        keyKP &= ~event->modifiers();

    for (int i = 0; i < 12; i++)
        if (keyKP == Config::KeyMapping[i])
            KeyInputMask &= ~(1<<i);

    for (int i = 0; i < 4; i++)
        if (keyKP == Config::TouchKeyMapping[i])
            KeyTouchInputMask &= ~(0xF << (i*4));

    for (int i = 0; i < HK_MAX; i++)
        if (keyHK == Config::HKKeyMapping[i])
            KeyHotkeyMask |= (1<<i);

    for (int i = 0; i < 4; i++)
        if (keyKP == Config::CmdMenuKeyMapping[i])
            KeyCmdMenuInputMask |= (1<<i);
}

void KeyRelease(QKeyEvent* event)
{
    int keyHK = GetEventKeyVal(event);
    int keyKP = keyHK;
    if (event->modifiers() != Qt::KeypadModifier)
        keyKP &= ~event->modifiers();

    for (int i = 0; i < 12; i++)
        if (keyKP == Config::KeyMapping[i])
            KeyInputMask |= (1<<i);

    for (int i = 0; i < 4; i++)
        if (keyKP == Config::TouchKeyMapping[i])
            KeyTouchInputMask |= (0xF << (i*4));

    for (int i = 0; i < HK_MAX; i++)
        if (keyHK == Config::HKKeyMapping[i])
            KeyHotkeyMask &= ~(1<<i);

    for (int i = 0; i < 4; i++)
        if (keyKP == Config::CmdMenuKeyMapping[i])
            KeyCmdMenuInputMask &= ~(1<<i);
}

void KeyReleaseAll()
{
    KeyInputMask = 0xFFF;
    KeyHotkeyMask = 0;
}

Sint16 JoystickButtonDown(int val)
{
    if (val == -1) return 0;

    bool hasbtn = ((val & 0xFFFF) != 0xFFFF);

    if (hasbtn)
    {
        if (val & 0x100)
        {
            int hatnum = (val >> 4) & 0xF;
            int hatdir = val & 0xF;
            Uint8 hatval = SDL_JoystickGetHat(Joystick, hatnum);

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
            Uint8 btnval = SDL_JoystickGetButton(Joystick, btnnum);

            if (btnval) return 1;
        }
    }

    if (val & 0x10000)
    {
        int axisnum = (val >> 24) & 0xF;
        int axisdir = (val >> 20) & 0xF;
        Sint16 axisval = SDL_JoystickGetAxis(Joystick, axisnum); // from -32768 to 32767

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

void Process()
{
    SDL_JoystickUpdate();

    if (Joystick)
    {
        if (!SDL_JoystickGetAttached(Joystick))
        {
            SDL_JoystickClose(Joystick);
            Joystick = NULL;
        }
    }
    if (!Joystick && (SDL_NumJoysticks() > 0))
    {
        JoystickID = Config::JoystickID;
        OpenJoystick(false);
    }

    JoyInputMask = 0xFFF;
    JoyTouchInputMask = 0xFFFF;
    if (Joystick)
    {
        for (int i = 0; i < 12; i++)
            if (JoystickButtonDown(Config::JoyMapping[i]) != 0)
                JoyInputMask &= ~(1<<i);
        for (int i = 0; i < 4; i++) {
            Sint16 joyValue = JoystickButtonDown(Config::TouchJoyMapping[i]);
            if (joyValue != 0)
                JoyTouchInputMask &= ~(joyValue << (i*4));
        }
    }

    InputMask = KeyInputMask & JoyInputMask;
    TouchInputMask = KeyTouchInputMask & JoyTouchInputMask;

    JoyHotkeyMask = 0;
    JoyCmdMenuInputMask = 0;
    if (Joystick)
    {
        for (int i = 0; i < HK_MAX; i++)
            if (JoystickButtonDown(Config::HKJoyMapping[i]) != 0)
                JoyHotkeyMask |= (1<<i);
        for (int i = 0; i < 4; i++)
            if (JoystickButtonDown(Config::CmdMenuJoyMapping[i]) != 0)
                JoyCmdMenuInputMask |= (1<<i);
    }

    HotkeyMask = KeyHotkeyMask | JoyHotkeyMask;
    HotkeyPress = HotkeyMask & ~LastHotkeyMask;
    HotkeyRelease = LastHotkeyMask & ~HotkeyMask;
    LastHotkeyMask = HotkeyMask;

    PriorPriorCmdMenuInputMask = PriorCmdMenuInputMask;
    PriorCmdMenuInputMask = CmdMenuInputMask;
    CmdMenuInputMask = KeyCmdMenuInputMask | JoyCmdMenuInputMask;
}


bool HotkeyDown(int id)     { return HotkeyMask    & (1<<id); }
bool HotkeyPressed(int id)  { return HotkeyPress   & (1<<id); }
bool HotkeyReleased(int id) { return HotkeyRelease & (1<<id); }


// distinguish between left and right modifier keys (Ctrl, Alt, Shift)
// Qt provides no real cross-platform way to do this, so here we go
// for Windows and Linux we can distinguish via scancodes (but both
// provide different scancodes)
#ifdef __WIN32__
bool IsRightModKey(QKeyEvent* event)
{
    quint32 scan = event->nativeScanCode();
    return (scan == 0x11D || scan == 0x138 || scan == 0x36);
}
#elif __APPLE__
bool IsRightModKey(QKeyEvent* event)
{
    quint32 scan = event->nativeVirtualKey();
    return (scan == 0x36 || scan == 0x3C || scan == 0x3D || scan == 0x3E);
}
#else
bool IsRightModKey(QKeyEvent* event)
{
    quint32 scan = event->nativeScanCode();
    return (scan == 0x69 || scan == 0x6C || scan == 0x3E);
}
#endif

}
