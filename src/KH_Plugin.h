#ifndef KH_PLUGIN_H
#define KH_PLUGIN_H

#include "KHDays_Plugin.h"
#include "CartValidator.h"
#include "NDS.h"

namespace melonDS
{

class KHPlugin
{
public:
    static u32 applyCommandMenuInputMask(u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::applyCommandMenuInputMask(InputMask, CmdMenuInputMask, PriorCmdMenuInputMask);
        }
        return InputMask;
    }
    static const char* getNameByGameScene(int newGameScene) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::getNameByGameScene(newGameScene);
        }
        return "";
    }
    static int detectGameScene(melonDS::NDS* nds) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::detectGameScene(nds);
        }
        return -1;
    }
    static bool setGameScene(melonDS::NDS* nds, int newGameScene) {
        if (CartValidator::isDays()) {
            return KHDaysPlugin::setGameScene(nds, newGameScene);
        }
        return false;
    }
    static void debugLogs(melonDS::NDS* nds, int gameScene) {
        if (CartValidator::isDays()) {
            KHDaysPlugin::debugLogs(nds, gameScene);
        }
    }
};
}

#endif
