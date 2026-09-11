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

#ifndef SCREEN_H
#define SCREEN_H

#include "libretro.h"

#include "types.h"
#include "NDS.h"
#include "plugins/Plugin.h"

// Video output of the libretro frontend. It owns the screen layout, composes
// the two DS framebuffers into the single frame libretro expects, and is the
// only place that knows where each DS screen ends up inside that frame, which
// is what makes pointer input mappable back to touchscreen coordinates.

namespace Libretro
{
namespace Screen
{

// Allocates the composition buffer and installs a valid default layout, so a
// frame can be rendered before the first RefreshLayout call.
void Init();
void DeInit();

// Recomputes the active layout from Libretro::Config and, when the layout is
// screenLayout_Auto, from the plugin. A null plugin falls back to the top
// screen alone. Returns true when the resulting geometry differs from the
// previous one.
//
// This only computes: issuing RETRO_ENVIRONMENT_SET_GEOMETRY is left to the
// caller, because that environment call is only legal from inside retro_run
// and the core has to keep track of when it may run it.
bool RefreshLayout(Plugins::Plugin* plugin);

// Fills a retro_game_geometry with the current base size and aspect ratio, plus
// the maximum size the layouts can reach. SET_GEOMETRY ignores the maximum,
// retro_get_system_av_info needs it, so both callers can use this.
void GetGeometry(retro_game_geometry* geometry);

// Composes the DS framebuffers into the internal buffer and hands it to
// Libretro::VideoCallback.
void RenderFrame(melonDS::NDS& nds);

// Maps a frontend pointer position (RETRO_DEVICE_ID_POINTER_X/Y, which range
// over -0x7FFF..0x7FFF across the whole video frame) to DS touchscreen
// coordinates. Returns false when the point falls outside the touchable
// screen, in which case *touchX and *touchY are untouched.
bool PointerToTouch(int pointerX, int pointerY, melonDS::u16* touchX, melonDS::u16* touchY);

}
}

#endif // SCREEN_H
