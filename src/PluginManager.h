#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "PluginDefault.h"

namespace Plugins
{

class PluginManager
{
public:
    static Plugin* load(u32 gameCode);
    static Plugin* get();
    static u32 getGameCode();

private:
    static u32 GameCode;
    static std::map<u32, std::unique_ptr<Plugin>> PluginsCache;
};
}

#endif
