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

#include <SDL2/SDL.h>
#include <QString>

// Reverse-map a raw joystick button index -> SDL_GameControllerButton
inline SDL_GameControllerButton RawButtonToGCButton(SDL_GameController* gc, int rawIndex)
{
    for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; ++b)
    {
        SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForButton(gc, (SDL_GameControllerButton)b);
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON && bind.value.button == rawIndex)
            return (SDL_GameControllerButton)b;
    }
    return SDL_CONTROLLER_BUTTON_INVALID;
}

inline SDL_GameControllerAxis RawAxisToGCAxis(SDL_GameController* gc, int rawIndex)
{
    for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; ++a)
    {
        SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForAxis(gc, (SDL_GameControllerAxis)a);
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS && bind.value.axis == rawIndex)
            return (SDL_GameControllerAxis)a;
    }
    return SDL_CONTROLLER_AXIS_INVALID;
}

// Small per-type table, only for the 4 face buttons (everything else is
// already consistent enough across pads: shoulders, sticks, dpad, start/back)
inline QString FaceButtonLabel(SDL_GameControllerType type, SDL_GameControllerButton btn)
{
    switch (type)
    {
    case SDL_CONTROLLER_TYPE_PS3:
    case SDL_CONTROLLER_TYPE_PS4:
    case SDL_CONTROLLER_TYPE_PS5:
        switch (btn)
        {
        case SDL_CONTROLLER_BUTTON_A: return QString::fromUtf8("✕");
        case SDL_CONTROLLER_BUTTON_B: return QString::fromUtf8("○");
        case SDL_CONTROLLER_BUTTON_X: return QString::fromUtf8("□");
        case SDL_CONTROLLER_BUTTON_Y: return QString::fromUtf8("△");
        default: break;
        }
        break;

    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
        // Switch face buttons keep Nintendo's letters but SDL's mapping
        // is positional, so A/B and X/Y swap relative to Xbox
        switch (btn)
        {
        case SDL_CONTROLLER_BUTTON_A: return "B";
        case SDL_CONTROLLER_BUTTON_B: return "A";
        case SDL_CONTROLLER_BUTTON_X: return "Y";
        case SDL_CONTROLLER_BUTTON_Y: return "X";
        default: break;
        }
        break;

    default: // Xbox360 / XboxOne / unknown -> fall through to generic names
        break;
    }

    switch (btn)
    {
    case SDL_CONTROLLER_BUTTON_A: return "A";
    case SDL_CONTROLLER_BUTTON_B: return "B";
    case SDL_CONTROLLER_BUTTON_X: return "X";
    case SDL_CONTROLLER_BUTTON_Y: return "Y";
    default: return QString();
    }
}

inline QString GCAxisName(SDL_GameControllerType type, SDL_GameControllerAxis axis)
{
    bool isSwitch = (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO);
    bool isPlaystation = (type == SDL_CONTROLLER_TYPE_PS3 ||
                          type == SDL_CONTROLLER_TYPE_PS4 ||
                          type == SDL_CONTROLLER_TYPE_PS5);

    switch (axis)
    {
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        return isPlaystation ? "L2" : isSwitch ? "ZL" : "LT";
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        return isPlaystation ? "R2" : isSwitch ? "ZR" : "RT";
    case SDL_CONTROLLER_AXIS_LEFTX:
    case SDL_CONTROLLER_AXIS_LEFTY:
        return "Left Stick";
    case SDL_CONTROLLER_AXIS_RIGHTX:
    case SDL_CONTROLLER_AXIS_RIGHTY:
        return "Right Stick";
    default:
        return QString();
    }
}

inline QString GCButtonName(SDL_GameController* gc, SDL_GameControllerButton btn)
{
    if (btn == SDL_CONTROLLER_BUTTON_INVALID) return "Unknown";

    SDL_GameControllerType type = SDL_GameControllerGetType(gc);

    if (btn == SDL_CONTROLLER_BUTTON_A || btn == SDL_CONTROLLER_BUTTON_B ||
        btn == SDL_CONTROLLER_BUTTON_X || btn == SDL_CONTROLLER_BUTTON_Y)
    {
        return FaceButtonLabel(type, btn);
    }

    if (btn == SDL_CONTROLLER_BUTTON_LEFTSHOULDER || btn == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
    {
        bool isPlaystation = (type == SDL_CONTROLLER_TYPE_PS3 ||
                              type == SDL_CONTROLLER_TYPE_PS4 ||
                              type == SDL_CONTROLLER_TYPE_PS5);
        bool isSwitch = (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO);

        if (btn == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
            return isPlaystation ? "L1" : isSwitch ? "L" : "LB";
        else
            return isPlaystation ? "R1" : isSwitch ? "R" : "RB";
    }

    if (btn == SDL_CONTROLLER_BUTTON_BACK || btn == SDL_CONTROLLER_BUTTON_START ||
        btn == SDL_CONTROLLER_BUTTON_GUIDE)
    {
        switch (type)
        {
        case SDL_CONTROLLER_TYPE_XBOXONE:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "View";
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Menu";
            return "Xbox Button";

        case SDL_CONTROLLER_TYPE_XBOX360:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "Back";
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Start";
            return "Guide";

        case SDL_CONTROLLER_TYPE_PS3:
        case SDL_CONTROLLER_TYPE_PS4:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "Select";
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Options";
            return "PS Button";

        case SDL_CONTROLLER_TYPE_PS5:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "Create"; // PS5 renamed Share -> Create
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Options";
            return "PS Button";

        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "Minus";
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Plus";
            return "Home";

        default:
            if (btn == SDL_CONTROLLER_BUTTON_BACK)  return "Back";
            if (btn == SDL_CONTROLLER_BUTTON_START) return "Start";
            return "Guide";
        }
    }

    static const QMap<QString, QString> pretty = {
        {"leftstick", "Left Stick"}, {"rightstick", "Right Stick"},
        {"dpup", "D-Pad Up"}, {"dpdown", "D-Pad Down"},
        {"dpleft", "D-Pad Left"}, {"dpright", "D-Pad Right"},
        {"back", "Back"}, {"start", "Start"}, {"guide", "Guide"},
        {"touchpad", "Touchpad"}
    };
    QString raw = SDL_GameControllerGetStringForButton(btn);
    return pretty.value(raw, raw.toUpper());
}

inline QString JoyMappingName(SDL_GameController* gc, int id)
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
            int rawIndex = id & 0xFFFF;
            SDL_GameControllerButton btn =
                gc ? RawButtonToGCButton(gc, rawIndex) : SDL_CONTROLLER_BUTTON_INVALID;
            str = (btn != SDL_CONTROLLER_BUTTON_INVALID)
                    ? GCButtonName(gc, btn)
                    : QString("Button %1").arg(rawIndex + 1);
        }
    }
    if (id & 0x10000)
    {
        int rawAxisIndex = (id >> 24) & 0xF;
        int direction = (id >> 20) & 0xF; // 0 = +, 1 = -, 2 = trigger
        if (hasbtn) str += " / ";

        SDL_GameControllerAxis axis =
            gc ? RawAxisToGCAxis(gc, rawAxisIndex) : SDL_CONTROLLER_AXIS_INVALID;

        if (axis != SDL_CONTROLLER_AXIS_INVALID)
        {
            QString name = GCAxisName(SDL_GameControllerGetType(gc), axis);
            str += (direction == 2) ? name // triggers have no +/- sign
                                     : QString("%1 %2").arg(name, direction == 0 ? "+" : "-");
        }
        else // no gc, or this raw axis isn't in the current mapping
        {
            int axisnum = rawAxisIndex + 1;
            switch (direction)
            {
            case 0: str += QString("Axis %1 +").arg(axisnum);  break;
            case 1: str += QString("Axis %1 -").arg(axisnum);  break;
            case 2: str += QString("Trigger %1").arg(axisnum); break;
            }
        }
    }
    return str.isEmpty() ? "None" : str;
}

inline QString JoyMappingName(int id)
{
    return JoyMappingName(nullptr, id);
}

#endif // BINDINGNAME_H
