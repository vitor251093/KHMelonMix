#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "NDS.h"

namespace melonDS
{
enum
{
    gameScene_Intro,              // 0
    gameScene_MainMenu,           // 1
    gameScene_IntroLoadMenu,      // 2
    gameScene_DayCounter,         // 3
    gameScene_Cutscene,           // 4
    gameScene_InGameWithMap,      // 5
    gameScene_InGameWithoutMap,   // 6
    gameScene_InGameMenu,         // 7
    gameScene_InGameSaveMenu,     // 8
    gameScene_InHoloMissionMenu,  // 9
    gameScene_PauseMenu,          // 10
    gameScene_PauseMenuWithGauge, // 11
    gameScene_Tutorial,           // 12
    gameScene_RoxasThoughts,      // 13
    gameScene_Shop,               // 14
    gameScene_BlackScreen,        // 15
    gameScene_Other2D,            // 16
    gameScene_Other               // 17
};

class KHDaysPlugin
{
public:
    static u32 applyCommandMenuInputMask(u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    static void fetchScreenStatus(melonDS::NDS* nds, int frontbuf);
    static const char* getNameByGameScene(int newGameScene);
    static float* getBackgroundColorByGameScene(melonDS::NDS* nds, int newGameScene);
    static int detectGameScene(melonDS::NDS* nds);
    static bool setGameScene(melonDS::NDS* nds, int newGameScene);
    static void debugLogs(melonDS::NDS* nds, int gameScene);
private:
    static bool isBufferBlack(unsigned int* buffer);
};
}

#endif
