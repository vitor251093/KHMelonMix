#include "PluginManager.h"

#include "PluginKingdomHeartsDays.h"
#include "PluginKingdomHeartsReCoded.h"

namespace Plugins
{

u32 PluginManager::GameCode = 0;

Plugin* PluginManager::load(u32 gameCode) {
    GameCode = gameCode;
    return get();
}
Plugin* PluginManager::get() {
    if (PluginKingdomHeartsDays::isCart(GameCode)) {
        return new PluginKingdomHeartsDays(GameCode);
    }
    if (PluginKingdomHeartsReCoded::isCart(GameCode)) {
        return new PluginKingdomHeartsReCoded(GameCode);
    }
    return new PluginDefault();
}
u32 PluginManager::getGameCode() {
    return GameCode;
}

}