#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "NDS.h"

#include "OpenGLSupport.h"

namespace melonDS
{
class KHDaysPlugin
{
public:
    static u32 applyCommandMenuInputMask(melonDS::NDS* nds, u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    static void hudToggle(melonDS::NDS* nds);

    static const char* getNameByGameScene(int newGameScene);
    static int detectGameScene(melonDS::NDS* nds);
    static bool setGameScene(melonDS::NDS* nds, int newGameScene);

    static bool shouldSkipFrame(melonDS::NDS* nds);
    static void extraRenderer(melonDS::NDS* nds, int ScreenW, int ScreenH);

    static void debugLogs(melonDS::NDS* nds, int gameScene);
private:
    static int GameScene;
    static int priorGameScene;
    static int HUDState;
    static bool ShowMap;
    static bool ShowTarget;
    static bool ShowMissionGauge;

    static bool _olderHad3DOnTopScreen;
    static bool _olderHad3DOnBottomScreen;
    static bool _had3DOnTopScreen;
    static bool _had3DOnBottomScreen;

    static bool _hasVisible3DOnBottomScreen;

    static GLuint mainMenuBgImageTextureId;
    static int mainMenuBgImageHeight;
    static int mainMenuBgImageWidth;
    static int mainMenuBgImageChannels;

    static GLuint mainMenuHeartImageTextureId;
    static int mainMenuHeartImageHeight;
    static int mainMenuHeartImageWidth;
    static int mainMenuHeartImageChannels;

    static bool isBufferBlack(unsigned int* buffer);
    static bool isTopScreen2DTextureBlack(melonDS::NDS* nds);
    static bool isBottomScreen2DTextureBlack(melonDS::NDS* nds);
    static void hudRefresh(melonDS::NDS* nds);

    static GLuint loadImageAsOpenGLTexture(const char* path, int* width, int* height, int* channels);
};
}

#endif
