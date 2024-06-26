#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsDays : public Plugin
{
public:
    PluginKingdomHeartsDays();

    bool isDebugEnabled;

    u32 applyCommandMenuInputMask(melonDS::NDS* nds, u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    void hudToggle(melonDS::NDS* nds);
    const char* getGameSceneName();
    bool shouldSkipFrame(melonDS::NDS* nds);
    void setAspectRatio(melonDS::NDS* nds, float aspectRatio);
    bool refreshGameScene(melonDS::NDS* nds);
    void debugLogs(melonDS::NDS* nds, int gameScene);
private:
    int GameScene;
    int priorGameScene;
    int HUDState;
    bool ShowMap;
    bool ShowTarget;
    bool ShowMissionGauge;

    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    bool _hasVisible3DOnBottomScreen;

    int detectGameScene(melonDS::NDS* nds);
    bool setGameScene(melonDS::NDS* nds, int newGameScene);

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack(melonDS::NDS* nds);
    bool isBottomScreen2DTextureBlack(melonDS::NDS* nds);
    void hudRefresh(melonDS::NDS* nds);
};
}

#endif
