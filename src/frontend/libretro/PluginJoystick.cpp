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

#include "plugins/PluginJoystick.h"

// src/plugins/KingdomHeartsHDCollection.cpp calls PluginJoystick::applyMappings,
// but the implementation belongs to the frontend: the Qt one reads the physical
// pad through SDL_GameController and writes the resulting button indices into
// its own config. A libretro core has neither half of that. RetroArch owns the
// bindings, and the plugin's extra buttons reach the core as virtual port 1 in
// Input.cpp, so there is nothing to auto-map and no config to write it to.
//
// The frontend still has to define the symbol, otherwise linking `core` into
// the shared module leaves it undefined.

namespace Plugins
{

void PluginJoystick::applyMappings(std::function<void(std::string, int)> setIntConfig,
    std::map<std::string, std::vector<PluginJoystickInput>> map)
{
}

}
