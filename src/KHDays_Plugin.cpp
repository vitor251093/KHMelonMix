#include "KHDays_Plugin.h"

#include "GPU3D_OpenGL.h"

#include <math.h>

namespace melonDS
{

int GameScene = -1;
int priorGameScene = -1;

bool _olderHad3DOnTopScreen = false;
bool _olderHad3DOnBottomScreen = false;
bool _had3DOnTopScreen = false;
bool _had3DOnBottomScreen = false;

// If you want to undertand that, check GPU2D_Soft.cpp, at the bottom of the SoftRenderer::DrawScanline function
#define PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(b) (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0xF)
#define PARSE_BRIGHTNESS_FOR_BLACK_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : 0)
#define PARSE_BRIGHTNESS_FOR_UNKNOWN_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0))

u32 KHDaysPlugin::applyCommandMenuInputMask(u32 InputMask, u32 CmdMenuInputMask, u32 PriorCmdMenuInputMask)
{
    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithoutMap || GameScene == gameScene_InGameWithCutscene) {
        // So the arrow keys can be used to control the command menu
        if (CmdMenuInputMask & (1 << 1)) { // D-pad left
            InputMask &= ~(1<<1); // B
        }
        if (CmdMenuInputMask & (1 << 0)) { // D-pad right
            InputMask &= ~(1<<0); // A
        }
        if (CmdMenuInputMask & ((1 << 2) | (1 << 3))) {
            InputMask &= ~(1<<10); // X
            if (CmdMenuInputMask & (1 << 2)) { // D-pad up
                // If you press the up arrow while having the player moving priorly, it may make it go down instead
                InputMask |= (1<<6); // up
                InputMask |= (1<<7); // down
            }
            if (PriorCmdMenuInputMask & (1 << 2)) // Old D-pad up
                InputMask &= ~(1<<6); // up
            if (PriorCmdMenuInputMask & (1 << 3)) // Old D-pad down
                InputMask &= ~(1<<7); // down
        }
    }
    else {
        // So the arrow keys can be used as directionals
        if (CmdMenuInputMask & (1 << 0)) { // D-pad right
            InputMask &= ~(1<<4); // right
        }
        if (CmdMenuInputMask & (1 << 1)) { // D-pad left
            InputMask &= ~(1<<5); // left
        }
        if (CmdMenuInputMask & (1 << 2)) { // D-pad up
            InputMask &= ~(1<<6); // up
        }
        if (CmdMenuInputMask & (1 << 3)) { // D-pad down
            InputMask &= ~(1<<7); // down
        }
    }
    return InputMask;
}

const char* KHDaysPlugin::getNameByGameScene(int newGameScene)
{
    switch (newGameScene) {
        case gameScene_Intro: return "Game scene: Intro";
        case gameScene_MainMenu: return "Game scene: Main menu";
        case gameScene_IntroLoadMenu: return "Game scene: Intro load menu";
        case gameScene_DayCounter: return "Game scene: Day counter";
        case gameScene_Cutscene: return "Game scene: Cutscene";
        case gameScene_InGameWithMap: return "Game scene: Ingame (with minimap)";
        case gameScene_InGameWithoutMap: return "Game scene: Ingame (without minimap)";
        case gameScene_InGameMenu: return "Game scene: Ingame menu";
        case gameScene_InGameSaveMenu: return "Game scene: Ingame save menu";
        case gameScene_InHoloMissionMenu: return "Game scene: Holo mission menu";
        case gameScene_PauseMenu: return "Game scene: Pause menu";
        case gameScene_Tutorial: return "Game scene: Tutorial";
        case gameScene_InGameWithCutscene: return "Game scene: Ingame (with cutscene)";
        case gameScene_MultiplayerMissionReview: return "Game scene: Multiplayer Mission Review";
        case gameScene_Shop: return "Game scene: Shop";
        case gameScene_Other2D: return "Game scene: Unknown (2D)";
        case gameScene_Other: return "Game scene: Unknown (3D)";
        default: return "Game scene: Unknown";
    }
}

int KHDaysPlugin::detectGameScene(melonDS::NDS* nds)
{
    // printf("0x021D08B8: %d\n",   nds->ARM7Read8(0x021D08B8));
    // printf("0x0223D38C: %d\n\n", nds->ARM7Read8(0x0223D38C));

    // Also happens during intro, during the start of the mission review, on some menu screens; those seem to use real 2D elements
    bool no3D = nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 0;

    // 3D element mimicking 2D behavior
    bool doesntLook3D = nds->GPU.GPU3D.RenderNumPolygons < 20;

    bool olderHad3DOnTopScreen = _olderHad3DOnTopScreen;
    bool olderHad3DOnBottomScreen = _olderHad3DOnBottomScreen;
    bool had3DOnTopScreen = _had3DOnTopScreen;
    bool had3DOnBottomScreen = _had3DOnBottomScreen;
    bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;
    bool has3DOnBottomScreen = (nds->PowerControl9 >> 9) == 1;
    _olderHad3DOnTopScreen = _had3DOnTopScreen;
    _olderHad3DOnBottomScreen = _had3DOnBottomScreen;
    _had3DOnTopScreen = has3DOnTopScreen;
    _had3DOnBottomScreen = has3DOnBottomScreen;
    bool has3DOnBothScreens = (!olderHad3DOnTopScreen && had3DOnTopScreen && !has3DOnTopScreen) ||
                              (!olderHad3DOnBottomScreen && had3DOnBottomScreen && !has3DOnBottomScreen);

    // The second screen can still look black and not be empty (invisible elements)
    bool noElementsOnBottomScreen = nds->GPU.GPU2D_B.BlendCnt == 0;

    // Scale of brightness, from 0 (black) to 15 (every element is visible)
    u8 topScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_A.MasterBrightness);
    u8 botScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_B.MasterBrightness);

    // Shop has 2D and 3D segments, which is why it's on the top
    bool isShop = (nds->GPU.GPU3D.RenderNumPolygons == 264 && nds->GPU.GPU2D_A.BlendCnt == 0 && 
                   nds->GPU.GPU2D_B.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendAlpha == 16) ||
            (GameScene == gameScene_Shop && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0);
    if (isShop)
    {
        return gameScene_Shop;
    }

    if (has3DOnBothScreens)
    {
        bool isMissionVictory = (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 130) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 2625) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 2114) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 130 && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 322 && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 840 && nds->GPU.GPU2D_B.BlendCnt == 0);
        if (isMissionVictory)
        {
            if (GameScene != gameScene_InGameWithCutscene)
            {
                return gameScene_MultiplayerMissionReview;
            }
        }
        return gameScene_InGameWithCutscene;
    }

    if (doesntLook3D)
    {
        // Intro save menu
        bool isIntroLoadMenu = (nds->GPU.GPU2D_B.BlendCnt == 4164 || nds->GPU.GPU2D_B.BlendCnt == 4161) &&
            (nds->GPU.GPU2D_A.EVA == 0 || nds->GPU.GPU2D_A.EVA == 16) &&
             nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 0 &&
            (nds->GPU.GPU2D_B.EVA < 10 && nds->GPU.GPU2D_B.EVA >= 0) && 
            (nds->GPU.GPU2D_B.EVB >  7 && nds->GPU.GPU2D_B.EVB <= 16) && nds->GPU.GPU2D_B.EVY == 0;
        bool mayBeMainMenu = nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;

        if (isIntroLoadMenu)
        {
            return gameScene_IntroLoadMenu;
        }
        if (GameScene == gameScene_IntroLoadMenu)
        {
            if (mayBeMainMenu)
            {
                return gameScene_MainMenu;
            }
            if (nds->GPU.GPU3D.NumVertices != 8)
            {
                return gameScene_IntroLoadMenu;
            }
        }

        if (has3DOnBottomScreen && GameScene == gameScene_InGameMenu)
        {
            return gameScene_InGameMenu;
        }

        // Mission Mode / Story Mode - Challenges
        bool inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 129 && (nds->GPU.GPU2D_B.BlendCnt >= 143 && nds->GPU.GPU2D_B.BlendCnt <= 207);
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        if (GameScene == gameScene_MainMenu)
        {
            if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 1)
            {
                return gameScene_Cutscene;
            }

            mayBeMainMenu = nds->GPU.GPU3D.NumVertices < 15 && nds->GPU.GPU3D.NumPolygons < 15;
            if (mayBeMainMenu) {
                return gameScene_MainMenu;
            }
        }

        // Day counter
        if (GameScene == gameScene_DayCounter && !no3D)
        {
            return gameScene_DayCounter;
        }
        if (GameScene != gameScene_Intro)
        {
            if (nds->GPU.GPU3D.NumVertices == 8 && nds->GPU.GPU3D.NumPolygons == 2 && nds->GPU.GPU3D.RenderNumPolygons == 2)
            {
                return gameScene_DayCounter;
            }
            if (nds->GPU.GPU3D.NumVertices == 12 && nds->GPU.GPU3D.NumPolygons == 3 && nds->GPU.GPU3D.RenderNumPolygons == 3)
            {
                return gameScene_DayCounter;
            }
        }

        // Main menu
        if (mayBeMainMenu)
        {
            return gameScene_MainMenu;
        }

        // Intro
        if (GameScene == -1 || GameScene == gameScene_Intro)
        {
            mayBeMainMenu = nds->GPU.GPU3D.NumVertices > 0 && nds->GPU.GPU3D.NumPolygons > 0;
            return mayBeMainMenu ? gameScene_MainMenu : gameScene_Intro;
        }

        // Intro cutscene
        if (GameScene == gameScene_Cutscene)
        {
            if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons >= 0 && nds->GPU.GPU3D.RenderNumPolygons <= 3)
            {
                return gameScene_Cutscene;
            }
        }

        // In Game Save Menu
        bool isGameSaveMenu = nds->GPU.GPU2D_A.BlendCnt == 4164 && (nds->GPU.GPU2D_B.EVA == 0 || nds->GPU.GPU2D_B.EVA == 16) && 
             nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0 &&
            (nds->GPU.GPU2D_A.EVA < 10 && nds->GPU.GPU2D_A.EVA >= 2) && 
            (nds->GPU.GPU2D_A.EVB >  7 && nds->GPU.GPU2D_A.EVB <= 14);
        if (isGameSaveMenu) 
        {
            return gameScene_InGameSaveMenu;
        }

        if (has3DOnBottomScreen)
        {
            return gameScene_InGameMenu;
        }

        // Bottom cutscene
        bool isBottomCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
             nds->GPU.GPU2D_A.EVA == 16 && nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 9 &&
             nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (isBottomCutscene)
        {
            return gameScene_Cutscene;
        }

        // Unknown 2D
        return gameScene_Other2D;
    }

    if (has3DOnTopScreen)
    {
        // Tutorial
        if (GameScene == gameScene_Tutorial && topScreenBrightness < 15)
        {
            return gameScene_Tutorial;
        }
        bool inTutorialScreen = topScreenBrightness == 8 && botScreenBrightness == 15;
        if (inTutorialScreen)
        {
            return gameScene_Tutorial;
        }
        bool inTutorialScreenWithoutWarningOnTop = nds->GPU.GPU2D_A.BlendCnt == 193 && nds->GPU.GPU2D_B.BlendCnt == 172 && 
                                                   nds->GPU.GPU2D_B.MasterBrightness == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (inTutorialScreenWithoutWarningOnTop)
        {
            return gameScene_Tutorial;
        }

        bool inGameMenu = (nds->GPU.GPU3D.NumVertices > 940 || nds->GPU.GPU3D.NumVertices == 0) &&
                          nds->GPU.GPU3D.RenderNumPolygons > 340 && nds->GPU.GPU3D.RenderNumPolygons < 370 &&
                          (nds->GPU.GPU2D_A.BlendCnt == 0 || nds->GPU.GPU2D_A.BlendCnt == 2625) && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inGameMenu)
        {
            return gameScene_InGameMenu;
        }

        // After exiting a mission from Mission Mode
        inGameMenu = (nds->GPU.GPU3D.NumVertices > 940 || nds->GPU.GPU3D.NumVertices == 0) &&
                      nds->GPU.GPU3D.RenderNumPolygons > 370 && nds->GPU.GPU3D.RenderNumPolygons < 400 &&
                      (nds->GPU.GPU2D_A.BlendCnt == 0 || nds->GPU.GPU2D_A.BlendCnt == 2625) && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inGameMenu)
        {
            return gameScene_InGameMenu;
        }

        // Story Mode - Normal missions
        bool inHoloMissionMenu = ((nds->GPU.GPU3D.NumVertices == 344 && nds->GPU.GPU3D.NumPolygons == 89 && nds->GPU.GPU3D.RenderNumPolygons == 89) ||
                                  (nds->GPU.GPU3D.NumVertices == 348 && nds->GPU.GPU3D.NumPolygons == 90 && nds->GPU.GPU3D.RenderNumPolygons == 90)) &&
                                 nds->GPU.GPU2D_A.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inHoloMissionMenu || GameScene == gameScene_InHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // Mission Mode / Story Mode - Challenges
        inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 129 && (nds->GPU.GPU2D_B.BlendCnt >= 143 && nds->GPU.GPU2D_B.BlendCnt <= 207);
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // I can't remember
        inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 2625 && nds->GPU.GPU2D_B.BlendCnt == 0;
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
        }

        // Pause Menu
        bool inMissionPauseMenu = nds->GPU.GPU2D_A.EVY == 8;
        if (inMissionPauseMenu)
        {
            return gameScene_PauseMenu;
        }
        else if (GameScene == gameScene_PauseMenu)
        {
            return priorGameScene;
        }

        // Regular gameplay without a map
        if (noElementsOnBottomScreen)
        {
            return gameScene_InGameWithoutMap;
        }

        // Regular gameplay with a map
        return gameScene_InGameWithMap;
    }

    if (GameScene == gameScene_InGameWithMap || has3DOnBottomScreen)
    {
        return gameScene_InGameWithCutscene;
    }
    
    // Unknown
    return gameScene_Other;
}

bool KHDaysPlugin::setGameScene(melonDS::NDS* nds, int newGameScene)
{
    bool updated = false;
    if (GameScene != newGameScene) 
    {
        updated = true;

        // Game scene
        priorGameScene = GameScene;
        GameScene = newGameScene;
    }

    // Updating GameScene inside shader
    static_cast<GLRenderer&>(nds->GPU.GetRenderer3D()).GetCompositor().SetGameScene(newGameScene);
    return updated;
}

void KHDaysPlugin::debugLogs(melonDS::NDS* nds, int gameScene)
{
    printf("Game scene: %d\n", gameScene);
    printf("NDS->GPU.GPU3D.NumVertices: %d\n",        nds->GPU.GPU3D.NumVertices);
    printf("NDS->GPU.GPU3D.NumPolygons: %d\n",        nds->GPU.GPU3D.NumPolygons);
    printf("NDS->GPU.GPU3D.RenderNumPolygons: %d\n",  nds->GPU.GPU3D.RenderNumPolygons);
    printf("NDS->PowerControl9: %d\n",                nds->PowerControl9);
    printf("NDS->GPU.GPU2D_A.BlendCnt: %d\n",         nds->GPU.GPU2D_A.BlendCnt);
    printf("NDS->GPU.GPU2D_A.BlendAlpha: %d\n",       nds->GPU.GPU2D_A.BlendAlpha);
    printf("NDS->GPU.GPU2D_A.EVA: %d\n",              nds->GPU.GPU2D_A.EVA);
    printf("NDS->GPU.GPU2D_A.EVB: %d\n",              nds->GPU.GPU2D_A.EVB);
    printf("NDS->GPU.GPU2D_A.EVY: %d\n",              nds->GPU.GPU2D_A.EVY);
    printf("NDS->GPU.GPU2D_A.MasterBrightness: %d\n", nds->GPU.GPU2D_A.MasterBrightness);
    printf("NDS->GPU.GPU2D_B.BlendCnt: %d\n",         nds->GPU.GPU2D_B.BlendCnt);
    printf("NDS->GPU.GPU2D_B.BlendAlpha: %d\n",       nds->GPU.GPU2D_B.BlendAlpha);
    printf("NDS->GPU.GPU2D_B.EVA: %d\n",              nds->GPU.GPU2D_B.EVA);
    printf("NDS->GPU.GPU2D_B.EVB: %d\n",              nds->GPU.GPU2D_B.EVB);
    printf("NDS->GPU.GPU2D_B.EVY: %d\n",              nds->GPU.GPU2D_B.EVY);
    printf("NDS->GPU.GPU2D_B.MasterBrightness: %d\n", nds->GPU.GPU2D_B.MasterBrightness);
    printf("\n");
}


}