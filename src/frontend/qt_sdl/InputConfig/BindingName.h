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

#ifndef BINDINGNAME_H
#define BINDINGNAME_H

#include <QString>

// Human-readable label for a joystick binding id, as encoded by the rebind capture code
// (button index, hat = 0x100 | dir | hat<<4, axis = 0xFFFF | 0x10000 | type<<20 | axis<<24).
// Single source of truth for decoding that layout — shared by the input-config dialog
// (JoyMapButton) and the in-game settings overlay (SettingsView) so the two can't drift.
// NOTE: the matching *encoder* lives in the capture paths (MapButton.h, SettingsView::
// pollBindCapture) and the runtime decoder in EmuInstance::joystickButtonDown; keep them in
// sync with the bit layout above.
inline QString JoyMappingName(int id)
{
    if (id == -1) return "None";

    bool hasbtn = ((id & 0xFFFF) != 0xFFFF);
    QString str;

    if (hasbtn)
    {
        if (id & 0x100)
        {
            int hatnum = ((id >> 4) & 0xF) + 1;
            switch (id & 0xF)
            {
            case 0x1: str = QString("Hat %1 Up").arg(hatnum);    break;
            case 0x2: str = QString("Hat %1 Right").arg(hatnum); break;
            case 0x4: str = QString("Hat %1 Down").arg(hatnum);  break;
            case 0x8: str = QString("Hat %1 Left").arg(hatnum);  break;
            default:  str = QString("Hat %1").arg(hatnum);       break;
            }
        }
        else
        {
            str = QString("Button %1").arg((id & 0xFFFF) + 1);
        }
    }

    if (id & 0x10000)
    {
        int axisnum = ((id >> 24) & 0xF) + 1;
        if (hasbtn) str += " / ";
        switch ((id >> 20) & 0xF)
        {
        case 0: str += QString("Axis %1 +").arg(axisnum);  break;
        case 1: str += QString("Axis %1 -").arg(axisnum);  break;
        case 2: str += QString("Trigger %1").arg(axisnum); break;
        }
    }

    return str.isEmpty() ? "None" : str;
}

#endif // BINDINGNAME_H
