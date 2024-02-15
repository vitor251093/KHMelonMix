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
    gameScene_Tutorial,           // 11
    gameScene_InGameWithSoraGlitch, // 12
    gameScene_Shop,               // 13
    gameScene_Other2D,            // 14
    gameScene_Other               // 15
};

class KHDaysPlugin
{
public:
    static u32 applyCommandMenuInputMask(u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    static const char* getNameByGameScene(int newGameScene);
    static int detectGameScene(melonDS::NDS* nds);
    static bool setGameScene(melonDS::NDS* nds, int newGameScene);
    static void debugLogs(melonDS::NDS* nds, int gameScene);
};
}

#endif
