#ifndef PLUGIN_H
#define PLUGIN_H

#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class Plugin
{
public:
    virtual ~Plugin() { };

    bool isDebugEnabled = false;

    virtual u32 applyCommandMenuInputMask(melonDS::NDS* nds, u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask) = 0;

    virtual void hudToggle(melonDS::NDS* nds) = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldSkipFrame(melonDS::NDS* nds) = 0;

    virtual bool refreshGameScene(melonDS::NDS* nds) = 0;

    virtual void setAspectRatio(melonDS::NDS* nds, float aspectRatio) = 0;
};
}

#endif
