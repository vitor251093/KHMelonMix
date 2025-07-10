#include "PluginManager.h"

#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"
#include "PluginHarvestMoonDsCute.h"
#include "PluginMetroidPrimeHunters.h"
#include "PluginTemplate.h"

#define LOAD_PLUGINS \
    LOAD_PLUGIN(PluginKingdomHeartsDays) \
    LOAD_PLUGIN(PluginKingdomHeartsReCoded) \
    LOAD_PLUGIN(PluginHarvestMoonDsCute) \
    LOAD_PLUGIN(PluginMetroidPrimeHunters) \
    LOAD_PLUGIN(PluginTemplate)

namespace Plugins
{

struct IFactory { 
    virtual bool isCart(int gameCode) = 0;
    virtual Plugin* create(int gameCode) = 0;
};

template< typename Type > struct Factory : public IFactory {
   virtual bool isCart(int gameCode) {
      return Type::isCart(gameCode);
   }
   virtual Type* create(int gameCode) {
      return new Type(gameCode);
   }
};

#define LOAD_PLUGIN(x) new Factory<x>,
IFactory* factories[] = {LOAD_PLUGINS};
#undef LOAD_PLUGIN

Plugin* PluginManager::load(u32 gameCode) {
    Plugin* plugin = nullptr;
    for (IFactory* factory : factories) {
        if (factory->isCart(gameCode)) {
            plugin = factory->create(gameCode);
            break;
        }
    }
    if (plugin == nullptr) {
        plugin = new PluginDefault(gameCode);
    }

    return plugin;
}

}