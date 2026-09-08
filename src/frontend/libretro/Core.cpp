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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <memory>
#include <string>
#include <vector>

#include "Core.h"
#include "CoreOptions.h"
#include "Input.h"
#include "Screen.h"

#include "version.h"

#include "Args.h"
#include "GPU.h"
#include "GPU2D_Soft.h"
#include "NDS.h"
#include "NDSCart.h"
#include "NDS_Header.h"
#include "Platform.h"
#include "SPI_Firmware.h"
#include "SPU.h"
#include "Savestate.h"
#include "plugins/PluginManager.h"

namespace Libretro
{

CoreConfig Config;

std::string SystemDirectory;
std::string SaveDirectory;

retro_environment_t EnvironmentCallback = nullptr;
retro_video_refresh_t VideoCallback = nullptr;
retro_audio_sample_batch_t AudioBatchCallback = nullptr;
retro_input_poll_t InputPollCallback = nullptr;
retro_input_state_t InputStateCallback = nullptr;

CoreState State;

static retro_audio_sample_t AudioSampleCallback = nullptr;
static retro_log_printf_t LogCallback = nullptr;

// Cached by the first retro_serialize_size call. Recomputing it means running
// a full dry-run savestate, which is far too expensive to do every frame.
static size_t SavestateSize = 0;

// The DS mixes at roughly 32 kHz, so a frame is around 550 stereo samples.
// 2048 frames leaves room for the frontend running the core slower than
// real time without ever truncating a frame's worth of audio.
static constexpr int AudioBufferFrames = 2048;
static melonDS::s16 AudioBuffer[AudioBufferFrames * 2];

// What the core reports, and asks the plugin to render at, when the plugin is
// in its single-screen mode and widescreen rendering is enabled. The Kingdom
// Hearts plugins do not widen the framebuffer; they patch the game's own
// aspect-ratio setting so it draws a wider field of view into the same
// 256x192 image, which the frontend then presents at this ratio.
static constexpr float WidescreenAspectRatio = 16.0f / 9.0f;

void Log(retro_log_level level, const char* fmt, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (LogCallback)
        LogCallback(level, "%s", buffer);
    else
        fprintf(stderr, "[KHMelonMix] %s", buffer);
}

// Reads a BIOS image out of the frontend's system directory. Returns nullptr
// when the file is missing or the wrong size, which leaves the caller free to
// fall back to the bundled FreeBIOS.
template <typename T>
static std::unique_ptr<T> LoadBIOS(const char* filename)
{
    melonDS::Platform::FileHandle* file =
        melonDS::Platform::OpenLocalFile(filename, melonDS::Platform::FileMode::Read);
    if (!file)
        return nullptr;

    std::unique_ptr<T> bios = nullptr;

    if (melonDS::Platform::FileLength(file) == sizeof(T))
    {
        bios = std::make_unique<T>();
        melonDS::Platform::FileRead(bios->data(), sizeof(T), 1, file);
    }
    else
    {
        Log(RETRO_LOG_WARN, "%s has the wrong size, ignoring it\n", filename);
    }

    melonDS::Platform::CloseFile(file);
    return bios;
}

static std::unique_ptr<melonDS::Firmware> LoadFirmware(const char* filename)
{
    melonDS::Platform::FileHandle* file =
        melonDS::Platform::OpenLocalFile(filename, melonDS::Platform::FileMode::Read);
    if (!file)
        return nullptr;

    std::unique_ptr<melonDS::Firmware> firmware = std::make_unique<melonDS::Firmware>(file);
    melonDS::Platform::CloseFile(file);

    if (!firmware->Buffer())
    {
        Log(RETRO_LOG_WARN, "%s is not a valid firmware image, ignoring it\n", filename);
        return nullptr;
    }

    return firmware;
}

static melonDS::Firmware::Language ResolveLanguage()
{
    if (Config.Language != language_Auto)
        return static_cast<melonDS::Firmware::Language>(Config.Language);

    unsigned frontendLanguage = RETRO_LANGUAGE_ENGLISH;
    if (!EnvironmentCallback(RETRO_ENVIRONMENT_GET_LANGUAGE, &frontendLanguage))
        return melonDS::Firmware::English;

    switch (frontendLanguage)
    {
    case RETRO_LANGUAGE_JAPANESE: return melonDS::Firmware::Japanese;
    case RETRO_LANGUAGE_FRENCH:   return melonDS::Firmware::French;
    case RETRO_LANGUAGE_GERMAN:   return melonDS::Firmware::German;
    case RETRO_LANGUAGE_ITALIAN:  return melonDS::Firmware::Italian;
    case RETRO_LANGUAGE_SPANISH:  return melonDS::Firmware::Spanish;
    default:                      return melonDS::Firmware::English;
    }
}

static void ApplyFirmwareLanguage(melonDS::Firmware& firmware)
{
    melonDS::Firmware::UserData& userData = firmware.GetEffectiveUserData();
    userData.Settings &= ~melonDS::Firmware::Reserved;
    userData.Settings |= ResolveLanguage();
    firmware.UpdateChecksums();
}

// The plugins read their settings through three getter callbacks. Their keys
// are all phrased negatively ("DisableX"), so answering false to everything
// yields the plugin's intended defaults: enhanced graphics on, single screen
// on, HD cutscenes on, subtitles on.
static void ApplyPluginConfig(Plugins::Plugin* plugin)
{
    auto getBoolConfig = [](std::string key) -> bool
    {
        if (!Config.PluginWidescreen && key.find(".DisableEnhancedGraphics") != std::string::npos)
            return true;

        return false;
    };

    auto getIntConfig = [](std::string key) -> int
    {
        // Zero means "unset" to the plugin, which then substitutes its own
        // default for camera sensitivity and HUD scale.
        return 0;
    };

    auto getStringConfig = [](std::string key) -> std::string
    {
        return std::string();
    };

    plugin->loadConfigs(getBoolConfig, getIntConfig, getStringConfig);
}

static void ApplyAspectRatio()
{
    if (!State.Plugin)
        return;

    // A zero ratio tells the plugin to leave the game's own setting alone.
    State.Plugin->setAspectRatio(Config.PluginWidescreen ? WidescreenAspectRatio : 0.0f);

    // The libretro core renders with the software renderer, which has no
    // internal upscaling, so the scale factor is always 1.
    State.Plugin->setInternalResolutionScale(1);
}

static void UpdateGeometry()
{
    if (!Screen::RefreshLayout(State.Plugin))
        return;

    retro_game_geometry geometry = {};
    Screen::GetGeometry(&geometry);
    EnvironmentCallback(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
}

static void UnloadCore()
{
    State.Nds = nullptr;
    State.Plugin = nullptr;
    State.RomPath.clear();
    State.SaveDirty = false;
    State.Stopped = false;
    SavestateSize = 0;
}

}

using namespace Libretro;

RETRO_API unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

RETRO_API void retro_set_environment(retro_environment_t cb)
{
    EnvironmentCallback = cb;

    // The core options have to be declared here: retro_set_environment is the
    // only call the frontend makes before it builds its settings menu.
    CoreOptions::SetOptions();

    bool noGame = false;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &noGame);
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb)
{
    VideoCallback = cb;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb)
{
    AudioSampleCallback = cb;
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    AudioBatchCallback = cb;
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb)
{
    InputPollCallback = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb)
{
    InputStateCallback = cb;
}

RETRO_API void retro_init(void)
{
    retro_log_callback logging = {};
    if (EnvironmentCallback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        LogCallback = logging.log;

    const char* dir = nullptr;
    if (EnvironmentCallback(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
        SystemDirectory = dir;

    dir = nullptr;
    if (EnvironmentCallback(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir)
        SaveDirectory = dir;

    Screen::Init();
    Input::Init();
}

RETRO_API void retro_deinit(void)
{
    UnloadCore();
    Screen::DeInit();

    LogCallback = nullptr;
    SystemDirectory.clear();
    SaveDirectory.clear();
}

RETRO_API void retro_get_system_info(struct retro_system_info* info)
{
    memset(info, 0, sizeof(*info));

    info->library_name = "KH Melon Mix";
    info->library_version = MELONDS_VERSION;

    // Has to agree with supported_extensions in khmelonmix_libretro.info: that
    // is the list the frontend filters its core suggestions by, and offering a
    // core that then rejects the file is worse than not being offered. ".ids"
    // is a decrypted NDS dump, which NDSCart::ParseROM reads the same way.
    info->valid_extensions = "nds|ids";

    // The ROM is parsed straight out of the buffer the frontend hands over, so
    // the core does not need the file to exist on disk.
    info->need_fullpath = false;
    info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info* info)
{
    memset(info, 0, sizeof(*info));

    Screen::GetGeometry(&info->geometry);

    // Reporting the largest layout as the maximum means switching layouts at
    // runtime only needs SET_GEOMETRY, not a full AV info reset.
    info->geometry.max_width = MaxScreenWidth;
    info->geometry.max_height = MaxScreenHeight;

    info->timing.fps = FramesPerSecond;
    info->timing.sample_rate = SampleRate;
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{
    Input::SetControllerDevice(port, device);
}

RETRO_API bool retro_load_game(const struct retro_game_info* game)
{
    if (!game || !game->data || game->size == 0)
        return false;

    enum retro_pixel_format format = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!EnvironmentCallback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format))
    {
        Log(RETRO_LOG_ERROR, "XRGB8888 is not supported by this frontend\n");
        return false;
    }

    CoreOptions::ReadOptions();

    melonDS::NDSArgs ndsargs = {};
    ndsargs.OutputSampleRate = SampleRate;

    if (Config.ExternalBIOS)
    {
        std::unique_ptr<melonDS::ARM9BIOSImage> arm9 = LoadBIOS<melonDS::ARM9BIOSImage>("bios9.bin");
        std::unique_ptr<melonDS::ARM7BIOSImage> arm7 = LoadBIOS<melonDS::ARM7BIOSImage>("bios7.bin");
        std::unique_ptr<melonDS::Firmware> firmware = LoadFirmware("firmware.bin");

        if (arm9 && arm7 && firmware)
        {
            ndsargs.ARM9BIOS = std::move(arm9);
            ndsargs.ARM7BIOS = std::move(arm7);
            ndsargs.Firmware = std::move(*firmware);
        }
        else
        {
            // Falling back is better than refusing to boot: FreeBIOS runs
            // every retail NDS cart, which is all this core loads.
            Log(RETRO_LOG_WARN,
                "External BIOS/firmware incomplete in the system directory, using FreeBIOS\n");
        }
    }

    std::unique_ptr<melonDS::u8[]> romdata = std::make_unique<melonDS::u8[]>(game->size);
    memcpy(romdata.get(), game->data, game->size);

    melonDS::NDSCart::NDSCartArgs cartargs = {};
    std::unique_ptr<melonDS::NDSCart::CartCommon> cart =
        melonDS::NDSCart::ParseROM(std::move(romdata), (melonDS::u32)game->size, &State, std::move(cartargs));
    if (!cart)
    {
        Log(RETRO_LOG_ERROR, "Failed to parse the ROM\n");
        return false;
    }

    State.RomPath = game->path ? game->path : "";
    State.Plugin = Plugins::PluginManager::load(cart->GetHeader().GameCodeAsU32());

    State.Nds = std::make_unique<melonDS::NDS>(std::move(ndsargs), &State);
    ApplyFirmwareLanguage(State.Nds->GetFirmware());
    State.Nds->SetNDSCart(std::move(cart));
    State.Nds->Reset();

    if (Config.DirectBoot || State.Nds->NeedsDirectBoot())
        State.Nds->SetupDirectBoot(State.RomPath);

    // Reset() leaves the console halted. Without this, RunFrame() skips its
    // whole `while (Running)` body and returns without emulating anything.
    State.Nds->Start();

    if (State.Plugin)
    {
        ApplyPluginConfig(State.Plugin);
        State.Plugin->setNds(State.Nds.get());
        State.Plugin->onLoadROM();

        // The 2D renderer needs the plugin to apply the per-scanline changes
        // the Kingdom Hearts games rely on.
        static_cast<melonDS::GPU2D::SoftRenderer&>(State.Nds->GPU.GetRenderer2D()).setPlugin(State.Plugin);
    }

    ApplyAspectRatio();
    Screen::RefreshLayout(State.Plugin);

    // Re-initialised on every load, not just in retro_init: a second game in
    // the same session would otherwise inherit the previous one's stylus
    // position and plugin-key edge state.
    Input::Init();
    Input::SetInputDescriptors(State.Plugin);

    return true;
}

RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info* info, size_t num_info)
{
    return false;
}

RETRO_API void retro_unload_game(void)
{
    UnloadCore();
}

RETRO_API void retro_reset(void)
{
    if (!State.Nds)
        return;

    State.Nds->Reset();

    if (Config.DirectBoot || State.Nds->NeedsDirectBoot())
        State.Nds->SetupDirectBoot(State.RomPath);

    State.Nds->Start();

    if (State.Plugin)
        State.Plugin->onLoadROM();

    State.Stopped = false;
}

RETRO_API void retro_run(void)
{
    if (!State.Nds)
        return;

    if (CoreOptions::HasUpdate() && CoreOptions::ReadOptions())
        ApplyAspectRatio();

    Input::Poll(*State.Nds, State.Plugin);

    if (State.Plugin)
        State.Plugin->refreshGameScene();

    State.Nds->RunFrame();

    UpdateGeometry();
    Screen::RenderFrame(*State.Nds);

    int available = State.Nds->SPU.GetOutputSize();
    if (available > AudioBufferFrames)
        available = AudioBufferFrames;

    if (available > 0)
    {
        int read = State.Nds->SPU.ReadOutput(AudioBuffer, available);
        if (read > 0)
            AudioBatchCallback(AudioBuffer, (size_t)read);
    }

    if (State.Stopped)
    {
        EnvironmentCallback(RETRO_ENVIRONMENT_SHUTDOWN, nullptr);
        State.Stopped = false;
    }
}

RETRO_API size_t retro_serialize_size(void)
{
    if (!State.Nds)
        return 0;

    if (SavestateSize != 0)
        return SavestateSize;

    melonDS::Savestate probe(melonDS::Savestate::DEFAULT_SIZE);
    if (probe.Error || !State.Nds->DoSavestate(&probe) || probe.Error)
    {
        Log(RETRO_LOG_ERROR, "Failed to measure the savestate size\n");
        return 0;
    }

    // Has to be the exact length, with no headroom: Savestate stores the length
    // it wrote in the state header and refuses to load unless the buffer it is
    // handed is that same size (Savestate.cpp, "expected a length of").
    SavestateSize = probe.Length();
    return SavestateSize;
}

RETRO_API bool retro_serialize(void* data, size_t len)
{
    if (!State.Nds || !data || len == 0)
        return false;

    melonDS::Savestate state(data, (melonDS::u32)len, true);
    if (state.Error)
        return false;

    return State.Nds->DoSavestate(&state) && !state.Error;
}

RETRO_API bool retro_unserialize(const void* data, size_t len)
{
    if (!State.Nds || !data || len == 0)
        return false;

    // Savestate does not modify the buffer when loading, but its constructor
    // takes a non-const pointer because the same class handles both directions.
    melonDS::Savestate state(const_cast<void*>(data), (melonDS::u32)len, false);
    if (state.Error)
        return false;

    if (!State.Nds->DoSavestate(&state) || state.Error)
        return false;

    if (State.Plugin)
        State.Plugin->onLoadState();

    return true;
}

RETRO_API void retro_cheat_reset(void)
{
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char* code)
{
    Log(RETRO_LOG_WARN, "Cheats are not supported by this core\n");
}

RETRO_API unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

RETRO_API void* retro_get_memory_data(unsigned id)
{
    if (id != RETRO_MEMORY_SAVE_RAM || !State.Nds)
        return nullptr;

    return State.Nds->GetNDSSave();
}

RETRO_API size_t retro_get_memory_size(unsigned id)
{
    if (id != RETRO_MEMORY_SAVE_RAM || !State.Nds)
        return 0;

    return State.Nds->GetNDSSaveLength();
}
