#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "PluginDefault.h"

namespace Plugins
{

class PluginManager
{
public:
    static Plugin* load(u32 gameCode);
};
}

#endif
