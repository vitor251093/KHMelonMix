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

#include <vector>

#include "Input.h"

#include "Core.h"
#include "Screen.h"
#include "libretro.h"

#include "types.h"

namespace Libretro
{
namespace Input
{

// Bit positions of KEYINPUT/EXTKEYIN as NDS::SetKeyMask() expects them. The
// mask is active low: a bit cleared to 0 means the button is held.
enum
{
    dsKey_A = 0,
    dsKey_B,
    dsKey_Select,
    dsKey_Start,
    dsKey_Right,
    dsKey_Left,
    dsKey_Up,
    dsKey_Down,
    dsKey_R,
    dsKey_L,
    dsKey_X,
    dsKey_Y,
};

struct ButtonMapping
{
    unsigned RetroId;
    unsigned DsBit;
    const char* Name;
};

// The RetroPad face buttons sit where the DS ones do, so the mapping is the
// identity a RetroArch user expects: whatever is bound to "A" presses A.
static const ButtonMapping ButtonMap[] =
{
    { RETRO_DEVICE_ID_JOYPAD_A,      dsKey_A,      "A"      },
    { RETRO_DEVICE_ID_JOYPAD_B,      dsKey_B,      "B"      },
    { RETRO_DEVICE_ID_JOYPAD_X,      dsKey_X,      "X"      },
    { RETRO_DEVICE_ID_JOYPAD_Y,      dsKey_Y,      "Y"      },
    { RETRO_DEVICE_ID_JOYPAD_L,      dsKey_L,      "L"      },
    { RETRO_DEVICE_ID_JOYPAD_R,      dsKey_R,      "R"      },
    { RETRO_DEVICE_ID_JOYPAD_START,  dsKey_Start,  "Start"  },
    { RETRO_DEVICE_ID_JOYPAD_SELECT, dsKey_Select, "Select" },
    { RETRO_DEVICE_ID_JOYPAD_UP,     dsKey_Up,     "Up"     },
    { RETRO_DEVICE_ID_JOYPAD_DOWN,   dsKey_Down,   "Down"   },
    { RETRO_DEVICE_ID_JOYPAD_LEFT,   dsKey_Left,   "Left"   },
    { RETRO_DEVICE_ID_JOYPAD_RIGHT,  dsKey_Right,  "Right"  },
};

// A standard RetroPad has no button left over once the twelve DS ones are
// bound, so the plugin keys (camera, HUD toggle, cutscene skip...) live on a
// second virtual port that the user maps to the same physical pad. Port 1
// carries at most this many of them, one per RetroPad id.
static const unsigned MaxPluginKeys = RETRO_DEVICE_ID_JOYPAD_R3 + 1;

// Idle value of the plugin's touch-key mask. The word packs a 4-bit movement
// strength per direction, in the order the Qt frontend names them:
// CameraRight | CameraLeft << 4 | CameraUp << 8 | CameraDown << 12.
// It is active low because Plugin::_superApplyTouchKeyMaskToTouchControls
// starts from ~TouchKeyMask: the mask travels alongside the DS key mask, which
// the hardware defines as active low, and both are built the same way. Zero
// would therefore mean "drag the stylus at full speed in every direction".
static const melonDS::u32 TouchKeyMaskIdle = 0xFFFF;

// Deflection below this counts as rest. Sticks do not return exactly to zero
// and the plugin moves the stylus every frame the word is not idle, so any
// leftover would creep the camera on its own. 25% of full travel.
static const melonDS::s32 AnalogDeadzone = 0x2000;
static const melonDS::s32 AnalogMaxDeflection = 0x7FFF;

// Largest value a nibble can hold, and the stylus movement in pixels per frame
// it asks for before Plugin::_superApplyTouchKeyMaskToTouchControls scales it
// down by the camera sensitivity: 15 is the fastest, 0 is no movement. The Qt
// frontend feeds this word from (axisValue >> 10), which is already past 15 at
// half deflection, so 15 is also the top speed a pad reaches there.
static const melonDS::s32 AnalogMaxStrength = 0xF;

// Whether one RETRO_DEVICE_ID_JOYPAD_MASK read can replace the twelve
// per-button calls. Answered once, at Init().
static bool SupportsBitmasks = false;

// Absolute stylus position for touchMode_Mouse. RETRO_DEVICE_MOUSE only
// reports movement since the last poll, so the cursor has to live here.
static melonDS::s32 MouseX = 0;
static melonDS::s32 MouseY = 0;

// Previous frame's plugin-key mask, to derive the newly-pressed word.
static melonDS::u32 LastAddonMask = 0;

// The frontend copies the descriptor array but keeps the description pointers,
// which is why the plugin's own labels are stored here rather than temporaries.
static std::vector<retro_input_descriptor> InputDescriptors;

void Init()
{
    SupportsBitmasks = EnvironmentCallback(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, nullptr);

    MouseX = (melonDS::s32)(ScreenWidth / 2);
    MouseY = (melonDS::s32)(ScreenHeight / 2);
    LastAddonMask = 0;
}

void SetInputDescriptors(Plugins::Plugin* plugin)
{
    InputDescriptors.clear();

    for (const ButtonMapping& button : ButtonMap)
        InputDescriptors.push_back({ 0, RETRO_DEVICE_JOYPAD, 0, button.RetroId, button.Name });

    // The right stick drives the plugin's camera, which the games control by
    // dragging the stylus. It is the same binding the Qt frontend recommends.
    InputDescriptors.push_back({ 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
        RETRO_DEVICE_ID_ANALOG_X, "Camera Left/Right" });
    InputDescriptors.push_back({ 0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
        RETRO_DEVICE_ID_ANALOG_Y, "Camera Up/Down" });

    if (plugin != nullptr)
    {
        unsigned keyCount = (unsigned)plugin->customKeyMappingLabels.size();
        if (keyCount > MaxPluginKeys)
            keyCount = MaxPluginKeys;

        for (unsigned i = 0; i < keyCount; i++)
            InputDescriptors.push_back({ 1, RETRO_DEVICE_JOYPAD, 0, i, plugin->customKeyMappingLabels[i] });
    }

    InputDescriptors.push_back({ 0, 0, 0, 0, nullptr });

    EnvironmentCallback(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, InputDescriptors.data());
}

void SetControllerDevice(unsigned port, unsigned device)
{
    // Port 0 is the DS itself and port 1 only exists to carry the plugin keys.
    // Both are plain RetroPads, so anything else is reported and ignored.
    if (device != RETRO_DEVICE_JOYPAD && device != RETRO_DEVICE_NONE)
        Log(RETRO_LOG_WARN, "[Input] Unsupported device %u on port %u, keeping the RetroPad\n", device, port);
}

static melonDS::u32 PollButtons()
{
    melonDS::u32 inputMask = 0xFFF;

    if (SupportsBitmasks)
    {
        // Cast through u16 first: bit 15 would sign-extend the int16_t.
        melonDS::u32 joypad = (melonDS::u16)InputStateCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);

        for (const ButtonMapping& button : ButtonMap)
            if (joypad & (1u << button.RetroId))
                inputMask &= ~(1u << button.DsBit);
    }
    else
    {
        for (const ButtonMapping& button : ButtonMap)
            if (InputStateCallback(0, RETRO_DEVICE_JOYPAD, 0, button.RetroId))
                inputMask &= ~(1u << button.DsBit);
    }

    return inputMask;
}

static melonDS::u32 PollPluginKeys(Plugins::Plugin* plugin)
{
    unsigned keyCount = (unsigned)plugin->customKeyMappingNames.size();
    if (keyCount == 0)
        return 0;
    if (keyCount > MaxPluginKeys)
        keyCount = MaxPluginKeys;

    // Bit i is customKeyMappingNames[i], which is the indexing the plugins use
    // through Plugin::customKeyIndexByName().
    if (SupportsBitmasks)
    {
        melonDS::u32 joypad = (melonDS::u16)InputStateCallback(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
        return joypad & ((1u << keyCount) - 1);
    }

    melonDS::u32 addonMask = 0;
    for (unsigned i = 0; i < keyCount; i++)
        if (InputStateCallback(1, RETRO_DEVICE_JOYPAD, 0, i))
            addonMask |= 1u << i;

    return addonMask;
}

// Turns one axis deflection into the 0..15 strength a touch-key nibble holds.
static melonDS::u32 AnalogStrength(melonDS::s32 deflection)
{
    if (deflection < 0)
        deflection = -deflection;

    // -0x8000 has no positive counterpart, and the nibble saturates anyway.
    if (deflection > AnalogMaxDeflection)
        deflection = AnalogMaxDeflection;

    if (deflection <= AnalogDeadzone)
        return 0;

    // Ramp from the edge of the dead area rather than from zero, so the camera
    // starts crawling the moment the stick leaves it instead of jumping.
    melonDS::s32 strength = ((deflection - AnalogDeadzone) * AnalogMaxStrength) /
        (AnalogMaxDeflection - AnalogDeadzone);

    return (melonDS::u32)(strength > 0 ? strength : 1);
}

static melonDS::u32 PollTouchKeyMask()
{
    melonDS::s32 axisX = InputStateCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
        RETRO_DEVICE_ID_ANALOG_X);
    melonDS::s32 axisY = InputStateCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
        RETRO_DEVICE_ID_ANALOG_Y);

    // Only the side the stick is pushed towards contributes, otherwise the two
    // nibbles of an axis would cancel each other out inside the plugin.
    // libretro reports Y positive downwards, like the SDL axis the Qt frontend
    // binds CameraUp to in its negative direction. Do not swap these two to
    // match the local names in _superApplyTouchKeyMaskToTouchControls: it calls
    // bits 8-11 "down" because they move the stylus towards the top of the
    // screen, which is what looking up drags.
    melonDS::u32 right = axisX > 0 ? AnalogStrength(axisX) : 0;
    melonDS::u32 left  = axisX < 0 ? AnalogStrength(axisX) : 0;
    melonDS::u32 up    = axisY < 0 ? AnalogStrength(axisY) : 0;
    melonDS::u32 down  = axisY > 0 ? AnalogStrength(axisY) : 0;

    // Clearing bits keeps a frontend with no analog input, which reports 0 on
    // both axes, on the idle word.
    return TouchKeyMaskIdle & ~(right | (left << 4) | (up << 8) | (down << 12));
}

static void PollTouch(melonDS::u16* touchX, melonDS::u16* touchY, bool* isTouching)
{
    switch (Config.TouchMode)
    {
    case touchMode_Pointer:
        if (InputStateCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED))
        {
            int pointerX = InputStateCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
            int pointerY = InputStateCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

            // Fails when the pointer is outside the touchable screen, in which
            // case the stylus stays up instead of jumping to an edge.
            *isTouching = Screen::PointerToTouch(pointerX, pointerY, touchX, touchY);
        }
        break;

    case touchMode_Mouse:
        MouseX += InputStateCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
        MouseY += InputStateCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);

        if (MouseX < 0) MouseX = 0;
        if (MouseY < 0) MouseY = 0;
        if (MouseX > (melonDS::s32)(ScreenWidth - 1)) MouseX = (melonDS::s32)(ScreenWidth - 1);
        if (MouseY > (melonDS::s32)(ScreenHeight - 1)) MouseY = (melonDS::s32)(ScreenHeight - 1);

        if (InputStateCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT))
        {
            *touchX = (melonDS::u16)MouseX;
            *touchY = (melonDS::u16)MouseY;
            *isTouching = true;
        }
        break;

    case touchMode_Disabled:
        break;
    }
}

void Poll(melonDS::NDS& nds, Plugins::Plugin* plugin)
{
    InputPollCallback();

    melonDS::u32 inputMask = PollButtons();

    melonDS::u16 touchX = 0;
    melonDS::u16 touchY = 0;
    bool isTouching = false;
    PollTouch(&touchX, &touchY, &isTouching);

    // Same order as EmuThread::run(): the touch keys move the stylus, then the
    // hotkeys and the plugin keys may override the whole thing.
    if (plugin != nullptr)
    {
        plugin->applyTouchKeyMaskToTouchControls(&touchX, &touchY, &isTouching, PollTouchKeyMask());

        // The core exposes no engine hotkeys - pause, reset and fast forward
        // are the frontend's job - but the plugin dereferences both words.
        melonDS::u32 hotkeyMask = 0;
        melonDS::u32 hotkeyPress = 0;
        plugin->applyHotkeyToInputMaskOrTouchControls(&inputMask, &touchX, &touchY, &isTouching,
            &hotkeyMask, &hotkeyPress);

        melonDS::u32 addonMask = PollPluginKeys(plugin);
        melonDS::u32 addonPress = addonMask & ~LastAddonMask;
        LastAddonMask = addonMask;
        plugin->applyAddonKeysToInputMaskOrTouchControls(&inputMask, &touchX, &touchY, &isTouching,
            &addonMask, &addonPress);
    }

    nds.SetKeyMask(inputMask);

    if (isTouching)
        nds.TouchScreen(touchX, touchY);
    else
        nds.ReleaseScreen();
}

}
}
