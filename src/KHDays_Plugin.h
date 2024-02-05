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
    gameScene_TopCutscene,        // 5
    gameScene_BottomCutscene,     // 6
    gameScene_InGameWithMap,      // 7
    gameScene_InGameWithoutMap,   // 8
    gameScene_InGameMenu,         // 9
    gameScene_InGameSaveMenu,     // 10
    gameScene_InHoloMissionMenu,  // 11
    gameScene_PauseMenu,          // 12
    gameScene_PauseMenuWithGauge, // 13
    gameScene_Tutorial,           // 14
    gameScene_RoxasThoughts,      // 15
    gameScene_Shop,               // 16
    gameScene_BlackScreen,        // 17
    gameScene_Other2D,            // 18
    gameScene_Other               // 19
};

class KHDaysPlugin
{
public:
    static u32 applyCommandMenuInputMask(u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask);
    static void fetchScreenStatus(melonDS::NDS* nds, int frontbuf);
    static int getSizeByGameScene(int newGameScene);
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
