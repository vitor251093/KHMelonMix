#ifndef KH_PLUGIN_H
#define KH_PLUGIN_H

#include "KHDays_Plugin.h"
#include "KHReCoded_Plugin.h"
#include "CartValidator.h"
#include "NDS.h"

// References for cheat codes
// https://uk.codejunkies.com/support_downloads/Trainer-Toolkit-for-Nintendo-DS-User-Manual.pdf

namespace melonDS
{

class KHPlugin
{
public:
    static bool isDebugEnabled() {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::isDebugEnabled;
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::isDebugEnabled;
        }
        return false;
    }

    static u32 applyCommandMenuInputMask(melonDS::NDS* nds, u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::applyCommandMenuInputMask(nds, InputMask, CmdMenuInputMask, PriorCmdMenuInputMask);
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::applyCommandMenuInputMask(nds, InputMask, CmdMenuInputMask, PriorCmdMenuInputMask);
        }
        return InputMask;
    }
    static void hudToggle(melonDS::NDS* nds) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::hudToggle(nds);
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::hudToggle(nds);
        }
    }
    static const char* getGameSceneName() {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::getGameSceneName();
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::getGameSceneName();
        }
        return "";
    }
    static bool shouldSkipFrame(melonDS::NDS* nds) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::shouldSkipFrame(nds);
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::shouldSkipFrame(nds);
        }
        return false;
    }
    static bool refreshGameScene(melonDS::NDS* nds) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::refreshGameScene(nds);
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::refreshGameScene(nds);
        }
        return false;
    }
    static void setAspectRatio(melonDS::NDS* nds, float aspectRatio) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::setAspectRatio(nds, aspectRatio);
        }
        if (CartValidator::isRecoded()) {
            return KHReCodedPlugin::setAspectRatio(nds, aspectRatio);
        }
    }
    static void debugLogs(melonDS::NDS* nds, int gameScene) {
        if (CartValidator::isDays()) {
            KHDaysPlugin::debugLogs(nds, gameScene);
        }
        if (CartValidator::isRecoded()) {
            KHReCodedPlugin::debugLogs(nds, gameScene);
        }
    }
};
}

#endif
