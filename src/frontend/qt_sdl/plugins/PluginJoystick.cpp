//
// Created by vitor on 2/26/26.
//

#include "../../../plugins/PluginJoystick.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <SDL2/SDL.h>

namespace Plugins
{

inline int GetAxisBinding(SDL_GameController* controller, SDL_GameControllerAxis axis, bool isNegative) {
    SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForAxis(controller, axis);

    if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS) {
        int physicalIndex = bind.value.axis;

        bool isTrigger = (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

        if (isTrigger) {
            return 0xFFFF | 0x10000 | (2 << 20) | (physicalIndex << 24);
        }

        int direction = isNegative ? 0x1 : 0x0;
        return 0xFFFF | 0x10000 | (direction << 20) | (physicalIndex << 24);
    }
    return -1;
}

inline int GetButtonBinding(SDL_GameController* controller, SDL_GameControllerButton button) {
    SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(controller, button);

    if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT) {
        int hatIndex = bind.value.hat.hat;
        int hatMask  = bind.value.hat.hat_mask;
        return 0x100 | hatMask | (hatIndex << 4);
    }

    if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
        return bind.value.button;
    }

    return -1;
}

inline int GetBinding(SDL_GameController* controller, std::vector<PluginJoystickInput> preferences) {
    int value = -1;
    for (auto button : preferences)
    {
        switch (button)
        {
            case PLUGIN_GAME_CONTROLLER_BUTTON_A:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_A); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_B:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_B); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_X:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_X); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_Y:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_Y); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_BACK:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_BACK); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_GUIDE:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_GUIDE); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_START:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_START); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_LEFTSTICK:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_RIGHTSTICK:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_LEFTSHOULDER:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_RIGHTSHOULDER:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_UP:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_DPAD_UP); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_DOWN:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_LEFT:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_DPAD_RIGHT:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_MISC1:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_MISC1); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE1:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_PADDLE1); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE2:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_PADDLE2); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE3:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_PADDLE3); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_PADDLE4:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_PADDLE4); break;
            case PLUGIN_GAME_CONTROLLER_BUTTON_TOUCHPAD:
                value = GetButtonBinding(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD); break;
            case PLUGIN_GAME_CONTROLLER_LEFT_AXIS_LEFT:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_LEFTX, true); break;
            case PLUGIN_GAME_CONTROLLER_LEFT_AXIS_RIGHT:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_LEFTX, false); break;
            case PLUGIN_GAME_CONTROLLER_LEFT_AXIS_UP:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_LEFTY, true); break;
            case PLUGIN_GAME_CONTROLLER_LEFT_AXIS_DOWN:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_LEFTY, false); break;
            case PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_LEFT:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_RIGHTX, true); break;
            case PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_RIGHT:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_RIGHTX, false); break;
            case PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_UP:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_RIGHTY, true); break;
            case PLUGIN_GAME_CONTROLLER_RIGHT_AXIS_DOWN:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_RIGHTY, false); break;
            case PLUGIN_GAME_CONTROLLER_LEFT_TRIGGER:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT, true); break;
            case PLUGIN_GAME_CONTROLLER_RIGHT_TRIGGER:
                value = GetAxisBinding(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, true); break;
            default:
                break;
        }

        if (value != -1)
        {
            return value;
        }
    }
    return -1;
}

void PluginJoystick::applyMappings(std::function<void(std::string, int)> setIntConfig, std::map<std::string, std::vector<PluginJoystickInput>> map)
{
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (!controller) continue;

            u16 vendor = SDL_GameControllerGetVendor(controller);
            u16 product = SDL_GameControllerGetProduct(controller);
            u32 controllerID = (int)((vendor << 16) | product);

            std::string prefix = "Instance0.Joystick." + std::to_string(controllerID) + ".";

            for (const auto& [keyName, preferences] : map)
            {
                setIntConfig(prefix + keyName, GetBinding(controller, preferences));
            }

            SDL_GameControllerClose(controller);
        }
    }
}
}