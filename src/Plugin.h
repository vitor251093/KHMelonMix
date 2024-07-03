#ifndef PLUGIN_H
#define PLUGIN_H

#define DEBUG_MODE_ENABLED false

#include "NDS.h"

namespace Plugins
{
using namespace melonDS;

class Plugin
{
public:
    virtual ~Plugin() { };

    bool isDebugEnabled = DEBUG_MODE_ENABLED;

    u32 GameCode = 0;
    static bool isCart(u32 gameCode) {return true;};
    virtual bool isUsaCart()    { return false; };
    virtual bool isEuropeCart() { return false; };
    virtual bool isJapanCart()  { return false; };

    virtual const char* gpuOpenGLFragmentShader() { return nullptr; };
    virtual const char* gpu3DOpenGLVertexShader() { return nullptr; };

    virtual u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) = 0;

    virtual void hudToggle(melonDS::NDS* nds) = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldSkipFrame(melonDS::NDS* nds) = 0;

    virtual bool refreshGameScene(melonDS::NDS* nds) = 0;

    virtual void setAspectRatio(melonDS::NDS* nds, float aspectRatio) = 0;
};
}

#endif
