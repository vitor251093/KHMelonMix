#include "PluginManager.h"

#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"

#define LOAD_PLUGINS \
    LOAD_PLUGIN(PluginKingdomHeartsDays) \
    LOAD_PLUGIN(PluginKingdomHeartsReCoded)

namespace Plugins
{

Plugin* PluginManager::LastPlugin = nullptr;

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
    LastPlugin = nullptr;
    for (IFactory* factory : factories) {
        if (factory->isCart(gameCode)) {
            LastPlugin = factory->create(gameCode);
            break;
        }
    }
    if (LastPlugin == nullptr) {
        LastPlugin = new PluginDefault(gameCode);
    }

    return get();
}
Plugin* PluginManager::get() {
    if (LastPlugin == nullptr) {
        LastPlugin = new PluginDefault(0);
    }
    return LastPlugin;
}

}