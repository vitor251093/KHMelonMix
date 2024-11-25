#include "InputAndroid.h"
#include "../NDS.h"

namespace MelonDSAndroid
{
    u32 keyMask = 0xFFF;

    void touchScreen(u16 x, u16 y)
    {
        NDS::TouchScreen(x, y);
    }

    void releaseScreen()
    {
        NDS::ReleaseScreen();
    }

    void pressKey(u32 key)
    {
        // Special handling for Lid input
        if (key == 16 + 7)
            NDS::SetLidClosed(true);
        else {
            keyMask &= ~(1 << key);
            NDS::SetKeyMask(keyMask);
        }
    }

    void releaseKey(u32 key)
    {
        // Special handling for Lid input
        if (key == 16 + 7)
            NDS::SetLidClosed(false);
        else {
            keyMask |= (1 << key);
            NDS::SetKeyMask(keyMask);
        }
    }
}

