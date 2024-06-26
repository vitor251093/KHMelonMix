#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "PluginDefault.h"
#include "KHDays_Plugin.h"
#include "KHReCoded_Plugin.h"
#include "CartValidator.h"

namespace Plugins
{

class PluginManager
{
public:
    static Plugin* load() {
        if (CartValidator::isDays()) {
            return new KHDaysPlugin();
        }
        if (CartValidator::isRecoded()) {
            return new KHReCodedPlugin();
        }
        return new PluginDefault();
    }
};
}

#endif
