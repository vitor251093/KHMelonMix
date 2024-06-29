#include "PluginManager.h"

#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"

namespace Plugins
{

u32 PluginManager::GameCode = 0;

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

IFactory* factories[] = {
  new Factory<PluginKingdomHeartsDays>,
  new Factory<PluginKingdomHeartsReCoded>
};

Plugin* PluginManager::load(u32 gameCode) {
    GameCode = gameCode;
    return get();
}
Plugin* PluginManager::get() {
    for (IFactory* factory : factories) {
        if (factory->isCart(GameCode)) {
            return factory->create(GameCode);
        }
    }
    return new PluginDefault();
}
u32 PluginManager::getGameCode() {
    return GameCode;
}

}