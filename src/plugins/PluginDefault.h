#ifndef PLUGIN_DEFAULT_H
#define PLUGIN_DEFAULT_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginDefault : public Plugin
{
public:
    PluginDefault(u32 gameCode) {
        GameCode = gameCode;

        hudToggle();
    };

    static bool isCart(u32 gameCode) {return true;};

    const char* getGameSceneName() {
        return "";
    }

    u32 getAspectRatioAddress() {return 0;}
};
}

#endif
