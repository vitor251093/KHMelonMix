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

#include "CoreOptions.h"

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "libretro.h"

#include "Core.h"

namespace Libretro
{

namespace CoreOptions
{

// Keys are shared between the option tables below and ReadOptions(), so a
// typo only has to be caught in one place instead of two.
static const char* KeyScreenLayout = "khmelonmix_screen_layout";
static const char* KeySwapScreens = "khmelonmix_swap_screens";
static const char* KeyScreenGap = "khmelonmix_screen_gap";
static const char* KeyTouchMode = "khmelonmix_touch_mode";
static const char* KeyDirectBoot = "khmelonmix_direct_boot";
static const char* KeyExternalBios = "khmelonmix_external_bios";
static const char* KeyPluginWidescreen = "khmelonmix_plugin_widescreen";
static const char* KeyLanguage = "khmelonmix_language";

// Not const: retro_core_options_v2::categories/definitions are plain
// (non-const) pointers, and this avoids sprinkling const_cast at the call
// site. The frontend is documented to copy the whole tree before returning.
static retro_core_option_v2_category Categories[] =
{
    {
        "video",
        "Video",
        "Screen layout and presentation"
    },
    {
        "system",
        "System",
        "Boot and BIOS settings"
    },
    { nullptr, nullptr, nullptr }
};

static retro_core_option_v2_definition Definitions[] =
{
    {
        KeyScreenLayout,
        "Screen layout",
        "Layout",
        "Selects which DS screen or screens are shown, and how they are arranged when both are visible. \"Plugin decides\" follows what the loaded plugin requests, which is the intended presentation for the Kingdom Hearts games.",
        nullptr,
        "video",
        {
            { "auto", "Plugin decides" },
            { "top", "Top screen only" },
            { "bottom", "Bottom screen only" },
            { "top-bottom", "Top/Bottom" },
            { "left-right", "Left/Right" },
            { nullptr, nullptr },
        },
        "auto"
    },
    {
        KeySwapScreens,
        "Swap screens",
        "Swap Screens",
        "Swaps the position of the top and bottom screen in layouts that show both.",
        nullptr,
        "video",
        {
            { "disabled", nullptr },
            { "enabled", nullptr },
            { nullptr, nullptr },
        },
        "disabled"
    },
    {
        KeyScreenGap,
        "Screen gap",
        "Screen Gap",
        "Gap between the top and bottom screen, in DS pixels. Only has an effect on the Top/Bottom layout.",
        nullptr,
        "video",
        {
            { "0", "No gap" },
            { "5", "5 px" },
            { "16", "16 px" },
            { "32", "32 px" },
            { "48", "48 px" },
            { "64", "64 px" },
            { "72", "72 px" },
            { "88", "88 px" },
            { nullptr, nullptr },
        },
        "0"
    },
    {
        KeyTouchMode,
        "Touch input",
        "Touch Input",
        "Chooses how pointer input is delivered to the DS touch screen. Pointer/Touchscreen maps the pointer directly onto the visible touch screen, Mouse uses relative motion instead, and Disabled ignores touch input entirely.",
        nullptr,
        "video",
        {
            { "pointer", "Pointer/Touchscreen" },
            { "mouse", "Mouse" },
            { "disabled", "Disabled" },
            { nullptr, nullptr },
        },
        "pointer"
    },
    {
        KeyDirectBoot,
        "Boot game directly",
        "Direct Boot",
        "When enabled, skips the DS firmware boot menu and starts the loaded game immediately. When disabled, boots into the firmware menu first, from which the game can be started manually.",
        nullptr,
        "system",
        {
            { "enabled", nullptr },
            { "disabled", nullptr },
            { nullptr, nullptr },
        },
        "enabled"
    },
    {
        KeyExternalBios,
        "Use external BIOS/firmware",
        "External BIOS/Firmware",
        "When enabled, the core looks for bios7.bin, bios9.bin and firmware.bin in the frontend's system directory and uses them instead of its own. When disabled, the core uses its bundled FreeBIOS with generated firmware, which needs no external files.",
        nullptr,
        "system",
        {
            { "disabled", nullptr },
            { "enabled", nullptr },
            { nullptr, nullptr },
        },
        "disabled"
    },
    {
        KeyLanguage,
        "Language",
        "Language",
        "Selects the language used by the Nintendo DS firmware and multi-language games. Auto follows RetroArch's frontend language.",
        nullptr,
        "system",
        {
            { "auto", "Auto" },
            { "japanese", "Japanese" },
            { "english", "English" },
            { "french", "French" },
            { "german", "German" },
            { "italian", "Italian" },
            { "spanish", "Spanish" },
            { nullptr, nullptr },
        },
        "auto"
    },
    {
        KeyPluginWidescreen,
        "Plugin widescreen rendering",
        "Widescreen Rendering",
        "The Kingdom Hearts plugins patch the game's own aspect-ratio setting so it renders in widescreen inside the DS framebuffer. Disabling this restores the game's original 4:3 framing.",
        nullptr,
        "video",
        {
            { "enabled", nullptr },
            { "disabled", nullptr },
            { nullptr, nullptr },
        },
        "enabled"
    },
    {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        {
            { nullptr, nullptr },
        },
        nullptr
    }
};

// Excludes the terminator at the end of Definitions.
constexpr int NumOptions = static_cast<int>((sizeof(Definitions) / sizeof(Definitions[0])) - 1);

// Version 1 fallback (RETRO_ENVIRONMENT_SET_CORE_OPTIONS). Filled lazily from
// Definitions so the value lists only have to be maintained in one place.
// Static storage: SET_CORE_OPTIONS keeps no guarantee that older frontends
// deep-copy the array before returning, so this has to outlive the call.
static retro_core_option_definition LegacyDefinitions[NumOptions + 1];
static bool LegacyDefinitionsBuilt = false;

static void BuildLegacyDefinitions()
{
    if (LegacyDefinitionsBuilt)
        return;

    for (int i = 0; i < NumOptions; i++)
    {
        LegacyDefinitions[i].key = Definitions[i].key;
        LegacyDefinitions[i].desc = Definitions[i].desc;
        LegacyDefinitions[i].info = Definitions[i].info;
        memcpy(LegacyDefinitions[i].values, Definitions[i].values, sizeof(LegacyDefinitions[i].values));
        LegacyDefinitions[i].default_value = Definitions[i].default_value;
    }
    // LegacyDefinitions[NumOptions] is the terminator; static storage already
    // zero-initialises it.

    LegacyDefinitionsBuilt = true;
}

// Version 0 fallback (RETRO_ENVIRONMENT_SET_VARIABLES). The frontend only
// ever sees a flat "desc; value|value|..." string per option, built once and
// kept alive for the process since old frontends have been known to keep raw
// pointers into this data instead of copying it.
static std::vector<std::string> LegacyValueStrings;
static std::vector<retro_variable> LegacyVariables;
static bool LegacyVariablesBuilt = false;

static void BuildLegacyVariables()
{
    if (LegacyVariablesBuilt)
        return;

    // Reserved up front so the push_back loop below never reallocates; the
    // c_str() pointers taken afterwards have to stay valid for good.
    LegacyValueStrings.reserve(NumOptions);

    for (int i = 0; i < NumOptions; i++)
    {
        std::string entry = Definitions[i].desc;
        entry += "; ";

        for (int j = 0; j < RETRO_NUM_CORE_OPTION_VALUES_MAX; j++)
        {
            const retro_core_option_value& value = Definitions[i].values[j];
            if (!value.value)
                break;

            if (j > 0)
                entry += '|';

            entry += value.value;
        }

        LegacyValueStrings.push_back(entry);
    }

    LegacyVariables.reserve(NumOptions + 1);
    for (int i = 0; i < NumOptions; i++)
        LegacyVariables.push_back({ Definitions[i].key, LegacyValueStrings[i].c_str() });
    LegacyVariables.push_back({ nullptr, nullptr });

    LegacyVariablesBuilt = true;
}

void SetOptions()
{
    if (!EnvironmentCallback)
        return;

    unsigned version = 0;
    bool haveVersion = EnvironmentCallback(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version);

    if (haveVersion && (version >= 2))
    {
        retro_core_options_v2 options = { Categories, Definitions };
        EnvironmentCallback(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2, &options);
        return;
    }

    if (haveVersion && (version == 1))
    {
        BuildLegacyDefinitions();
        EnvironmentCallback(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, LegacyDefinitions);
        return;
    }

    // GET_CORE_OPTIONS_VERSION itself failing means the same as it reporting
    // 0: assume the oldest interface.
    BuildLegacyVariables();
    EnvironmentCallback(RETRO_ENVIRONMENT_SET_VARIABLES, LegacyVariables.data());
}

// Returns nullptr both when the frontend has no such key and when the
// environment call is unavailable, which is exactly the case ReadOptions()
// needs to leave the current config value alone.
static const char* GetVariable(const char* key)
{
    if (!EnvironmentCallback)
        return nullptr;

    retro_variable var = { key, nullptr };
    if (!EnvironmentCallback(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        return nullptr;

    return var.value;
}

static bool ReadBool(const char* key, bool currentValue)
{
    const char* value = GetVariable(key);
    if (!value)
        return currentValue;

    if (strcmp(value, "enabled") == 0)
        return true;
    if (strcmp(value, "disabled") == 0)
        return false;

    Log(RETRO_LOG_WARN, "CoreOptions: unexpected value \"%s\" for %s\n", value, key);
    return currentValue;
}

static ScreenLayoutType ReadScreenLayout(ScreenLayoutType currentValue)
{
    const char* value = GetVariable(KeyScreenLayout);
    if (!value)
        return currentValue;

    if (strcmp(value, "auto") == 0)
        return screenLayout_Auto;
    if (strcmp(value, "top") == 0)
        return screenLayout_Top;
    if (strcmp(value, "bottom") == 0)
        return screenLayout_Bottom;
    if (strcmp(value, "top-bottom") == 0)
        return screenLayout_TopBottom;
    if (strcmp(value, "left-right") == 0)
        return screenLayout_LeftRight;

    Log(RETRO_LOG_WARN, "CoreOptions: unexpected value \"%s\" for %s\n", value, KeyScreenLayout);
    return currentValue;
}

static TouchModeType ReadTouchMode(TouchModeType currentValue)
{
    const char* value = GetVariable(KeyTouchMode);
    if (!value)
        return currentValue;

    if (strcmp(value, "pointer") == 0)
        return touchMode_Pointer;
    if (strcmp(value, "mouse") == 0)
        return touchMode_Mouse;
    if (strcmp(value, "disabled") == 0)
        return touchMode_Disabled;

    Log(RETRO_LOG_WARN, "CoreOptions: unexpected value \"%s\" for %s\n", value, KeyTouchMode);
    return currentValue;
}

static melonDS::u32 ReadScreenGap(melonDS::u32 currentValue)
{
    const char* value = GetVariable(KeyScreenGap);
    if (!value)
        return currentValue;

    char* end = nullptr;
    unsigned long parsed = strtoul(value, &end, 10);
    if ((end == value) || (*end != '\0'))
    {
        Log(RETRO_LOG_WARN, "CoreOptions: unexpected value \"%s\" for %s\n", value, KeyScreenGap);
        return currentValue;
    }

    return static_cast<melonDS::u32>(parsed);
}

static LanguageType ReadLanguage(LanguageType currentValue)
{
    const char* value = GetVariable(KeyLanguage);
    if (!value)
        return currentValue;

    if (strcmp(value, "auto") == 0)
        return language_Auto;
    if (strcmp(value, "japanese") == 0)
        return language_Japanese;
    if (strcmp(value, "english") == 0)
        return language_English;
    if (strcmp(value, "french") == 0)
        return language_French;
    if (strcmp(value, "german") == 0)
        return language_German;
    if (strcmp(value, "italian") == 0)
        return language_Italian;
    if (strcmp(value, "spanish") == 0)
        return language_Spanish;

    Log(RETRO_LOG_WARN, "CoreOptions: unexpected value \"%s\" for %s\n", value, KeyLanguage);
    return currentValue;
}

bool ReadOptions()
{
    CoreConfig previous = Config;

    Config.ScreenLayout = ReadScreenLayout(Config.ScreenLayout);
    Config.TouchMode = ReadTouchMode(Config.TouchMode);
    Config.ScreenGap = ReadScreenGap(Config.ScreenGap);
    Config.SwapScreens = ReadBool(KeySwapScreens, Config.SwapScreens);
    Config.DirectBoot = ReadBool(KeyDirectBoot, Config.DirectBoot);
    Config.ExternalBIOS = ReadBool(KeyExternalBios, Config.ExternalBIOS);
    Config.PluginWidescreen = ReadBool(KeyPluginWidescreen, Config.PluginWidescreen);
    Config.Language = ReadLanguage(Config.Language);

    return (previous.ScreenLayout != Config.ScreenLayout)
        || (previous.TouchMode != Config.TouchMode)
        || (previous.ScreenGap != Config.ScreenGap)
        || (previous.SwapScreens != Config.SwapScreens)
        || (previous.DirectBoot != Config.DirectBoot)
        || (previous.ExternalBIOS != Config.ExternalBIOS)
        || (previous.Language != Config.Language)
        || (previous.PluginWidescreen != Config.PluginWidescreen);
}

bool HasUpdate()
{
    if (!EnvironmentCallback)
        return false;

    bool updated = false;
    if (!EnvironmentCallback(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated))
        return false;

    return updated;
}

}

}
