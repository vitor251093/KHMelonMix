#include "PluginManager.h"

#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"

#define LOAD_PLUGINS \
    LOAD_PLUGIN(PluginKingdomHeartsDays) \
    LOAD_PLUGIN(PluginKingdomHeartsReCoded)

namespace Plugins
{

EmuInstance* PluginManager::emuInstance = nullptr;
u32 PluginManager::GameCode = 0;
std::map<u32, std::unique_ptr<Plugin>> PluginManager::PluginsCache = {};

struct IFactory { 
    virtual bool isCart(int gameCode) = 0;
    virtual Plugin* create(EmuInstance* instance, int gameCode) = 0;
};

template< typename Type > struct Factory : public IFactory {
   virtual bool isCart(int gameCode) {
      return Type::isCart(gameCode);
   }
   virtual Type* create(EmuInstance* instance, int gameCode) {
      return new Type(instance, gameCode);
   }
};

#define LOAD_PLUGIN(x) new Factory<x>,
IFactory* factories[] = {LOAD_PLUGINS};
#undef LOAD_PLUGIN

Plugin* PluginManager::load(EmuInstance* instance, u32 gameCode) {
    GameCode = gameCode;
    if (instance == nullptr) {
        return nullptr;
    }

    emuInstance = instance;
    return get();
}
Plugin* PluginManager::get() {
    if (auto search = PluginsCache.find(GameCode); search != PluginsCache.end()) {
        return search->second.get();
    }
    for (IFactory* factory : factories) {
        if (factory->isCart(GameCode)) {
            PluginsCache[GameCode] = std::unique_ptr<Plugin>(factory->create(emuInstance, GameCode));
            return PluginsCache.at(GameCode).get();
        }
    }
    PluginsCache[GameCode] = std::unique_ptr<Plugin>(new PluginDefault(emuInstance, GameCode));
    return PluginsCache.at(GameCode).get();
}
u32 PluginManager::getGameCode() {
    return GameCode;
}

}