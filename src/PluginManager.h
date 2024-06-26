#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "PluginDefault.h"
#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"
#include "CartValidator.h"

namespace Plugins
{

class PluginManager
{
public:
    static Plugin* load() {
        if (CartValidator::isDays()) {
            return new PluginKingdomHeartsDays();
        }
        if (CartValidator::isRecoded()) {
            return new PluginKingdomHeartsReCoded();
        }
        return new PluginDefault();
    }
};
}

#endif
