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

#ifndef INPUT_H
#define INPUT_H

#include "NDS.h"
#include "plugins/Plugin.h"

// Reads the RetroPad, its right stick and the pointer, hands the result to the
// loaded plugin so it can rewrite it, and applies whatever comes back to the
// emulated DS. The right stick reaches the games as stylus drags, which is how
// they take camera input.

namespace Libretro
{
namespace Input
{

void Init();

// Publishes retro_input_descriptor entries for the physical DS buttons, the
// camera stick and, when the plugin defines extra logical buttons, one per
// plugin key. Call it again whenever the loaded plugin changes; those labels
// come from the plugin.
void SetInputDescriptors(Plugins::Plugin* plugin);

void SetControllerDevice(unsigned port, unsigned device);

// Polls the frontend, lets the plugin rewrite the result, and applies it to
// the emulator. Call once per frame, before NDS::RunFrame.
void Poll(melonDS::NDS& nds, Plugins::Plugin* plugin);

}
}

#endif // INPUT_H
