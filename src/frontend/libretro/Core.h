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

#ifndef CORE_H
#define CORE_H

#include <memory>
#include <string>

#include "libretro.h"

#include "types.h"
#include "NDS.h"
#include "plugins/Plugin.h"

// Shared state of the libretro frontend. Every translation unit in this
// directory compiles against this header; it is the only thing they share.

namespace Libretro
{

// The DS renders 256x192 per screen. The widest layout puts both screens side
// by side, the tallest stacks them, so no layout can exceed these bounds. They
// are what retro_get_system_av_info reports as the maximum geometry, which
// lets the layout change at runtime with SET_GEOMETRY instead of forcing the
// frontend through a full AV info reset.
constexpr melonDS::u32 ScreenWidth = 256;
constexpr melonDS::u32 ScreenHeight = 192;
constexpr melonDS::u32 MaxScreenWidth = ScreenWidth * 2;

// The stacked layout can insert a gap between the two screens, so the tallest
// frame is both screens plus the widest gap the core options offer. The
// frontend sizes its video buffer from the maximum geometry and SET_GEOMETRY
// cannot raise it later, so the headroom has to be declared up front.
constexpr melonDS::u32 MaxScreenGap = 128;
constexpr melonDS::u32 MaxScreenHeight = ScreenHeight * 2 + MaxScreenGap;

// Derived from the ARM7 clock (33513982 Hz): one video frame is 560190 cycles,
// and the SPU emits one sample every 1024 cycles. Passing the sample rate to
// NDSArgs::OutputSampleRate too means melonDS' resampler runs at unity.
constexpr double FramesPerSecond = 33513982.0 / 560190.0;
constexpr double SampleRate = 33513982.0 / 1024.0;

enum ScreenLayoutType
{
    // Follow whatever the loaded plugin asks for. The Kingdom Hearts plugins
    // pick a single screen and drive the other one off-screen, so this is the
    // only layout that gives the intended presentation.
    screenLayout_Auto = 0,
    screenLayout_Top,
    screenLayout_Bottom,
    screenLayout_TopBottom,
    screenLayout_LeftRight,
};

enum TouchModeType
{
    touchMode_Pointer = 0,
    touchMode_Mouse,
    touchMode_Disabled,
};

enum LanguageType
{
    language_Auto = -1,
    language_Japanese = 0,
    language_English = 1,
    language_French = 2,
    language_German = 3,
    language_Italian = 4,
    language_Spanish = 5,
};

struct CoreConfig
{
    ScreenLayoutType ScreenLayout = screenLayout_Auto;
    TouchModeType TouchMode = touchMode_Pointer;
    LanguageType Language = language_Auto;

    // Gap between the two screens, in DS pixels, for the stacked layouts.
    melonDS::u32 ScreenGap = 0;
    bool SwapScreens = false;

    bool DirectBoot = true;
    bool ExternalBIOS = false;

    // Whether the plugin is allowed to patch the game's own aspect ratio
    // setting in RAM, which is how the widescreen rendering is obtained.
    bool PluginWidescreen = true;
};

extern CoreConfig Config;

// Filled in retro_set_environment / retro_init. Both may stay empty if the
// frontend does not provide the directory.
extern std::string SystemDirectory;
extern std::string SaveDirectory;

extern retro_environment_t EnvironmentCallback;
extern retro_video_refresh_t VideoCallback;
extern retro_audio_sample_batch_t AudioBatchCallback;
extern retro_input_poll_t InputPollCallback;
extern retro_input_state_t InputStateCallback;

// Logs through the frontend's log interface when it offers one, and to stderr
// otherwise. Never null, so callers do not have to check.
void Log(retro_log_level level, const char* fmt, ...);

struct CoreState
{
    std::unique_ptr<melonDS::NDS> Nds = nullptr;

    // Owned by PluginManager, which hands out a plugin per game code. Null
    // until a ROM is loaded.
    Plugins::Plugin* Plugin = nullptr;

    // Set by Platform::WriteNDSSave. RetroArch owns the .srm file and polls
    // retro_get_memory_data, so the core only has to remember that the buffer
    // moved on.
    bool SaveDirty = false;

    // Set by Platform::SignalStop so retro_run can ask the frontend to shut
    // down instead of running a dead emulator.
    bool Stopped = false;

    std::string RomPath;
};

extern CoreState State;

}

#endif // CORE_H
