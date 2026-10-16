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

#ifndef COREOPTIONS_H
#define COREOPTIONS_H

// Declares the khmelonmix_* core options to the frontend and reads them back
// into Libretro::Config. See Core.h for the config struct this fills.

namespace Libretro
{

namespace CoreOptions
{

// Declares the option set to the frontend. Call from retro_set_environment,
// before retro_init, which is the only point where the frontend accepts it.
void SetOptions();

// Reads every option into Libretro::Config. Returns true when at least one
// value changed since the previous call.
bool ReadOptions();

// True when the frontend reports that the user changed something, so
// retro_run can avoid re-reading every variable each frame.
bool HasUpdate();

}

}

#endif // COREOPTIONS_H
