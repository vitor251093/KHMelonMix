#include "PluginKingdomHeartsReCoded.h"

#include "PluginKingdomHeartsReCoded_GPU_OpenGL_shaders.h"
#include "PluginKingdomHeartsReCoded_GPU3D_OpenGL_shaders.h"

#include <math.h>

namespace Plugins
{

u32 PluginKingdomHeartsReCoded::usGamecode = 1161382722;
u32 PluginKingdomHeartsReCoded::euGamecode = 1345932098;
u32 PluginKingdomHeartsReCoded::jpGamecode = 1245268802;

#define ASPECT_RATIO_ADDRESS_US 0x0202A810
#define ASPECT_RATIO_ADDRESS_EU 0x0202A824
#define ASPECT_RATIO_ADDRESS_JP 0x0202A728

// 0x00 => intro and main menu
#define IS_MAIN_MENU_US 0x02056b28
#define IS_MAIN_MENU_EU 0x02056b28 // TODO: KH
#define IS_MAIN_MENU_JP 0x02056b28 // TODO: KH

#define PAUSE_SCREEN_ADDRESS_US 0x020569d0 // may also be 0x02056c5c or 0x0205fde4
#define PAUSE_SCREEN_ADDRESS_EU 0x020569d0 // TODO: KH
#define PAUSE_SCREEN_ADDRESS_JP 0x020569d0 // TODO: KH

#define PAUSE_SCREEN_VALUE_TRUE_PAUSE 0x01

// 0x03 => cutscene; 0x01 => not cutscene
#define IS_CUTSCENE_US 0x02056e90
#define IS_CUTSCENE_EU 0x02056e90 // TODO: KH
#define IS_CUTSCENE_JP 0x02056e90 // TODO: KH

// 0x04 => playable (example: ingame); 0x02 => not playable (menus)
#define IS_PLAYABLE_AREA_US 0x0205a8c0
#define IS_PLAYABLE_AREA_EU 0x0205a8c0 // TODO: KH
#define IS_PLAYABLE_AREA_JP 0x0205a8c0 // TODO: KH

#define CUTSCENE_SKIP_START_FRAMES_COUNT 40

#define SWITCH_TARGET_PRESS_FRAME_LIMIT   100
#define SWITCH_TARGET_TIME_BETWEEN_SWITCH 20
#define LOCK_ON_PRESS_FRAME_LIMIT         100

// If you want to undertand that, check GPU2D_Soft.cpp, at the bottom of the SoftRenderer::DrawScanline function
#define PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(b) (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0xF)
#define PARSE_BRIGHTNESS_FOR_BLACK_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : 0)
#define PARSE_BRIGHTNESS_FOR_UNKNOWN_BACKGROUND(b) (b & (1 << 14) ? ((b - 1) & 0xF) : (b & (1 << 15) ? (0xF - ((b - 1) & 0xF)) : 0))

enum
{
    gameScene_Intro,                    // 0
    gameScene_MainMenu,                 // 1
    gameScene_IntroLoadMenu,            // 2
    gameScene_DayCounter,               // 3
    gameScene_Cutscene,                 // 4
    gameScene_InGameWithMap,            // 5
    gameScene_InGameWithoutMap,         // 6 (unused)
    gameScene_InGameMenu,               // 7
    gameScene_InGameSaveMenu,           // 8
    gameScene_InHoloMissionMenu,        // 9
    gameScene_PauseMenu,                // 10
    gameScene_Tutorial,                 // 11
    gameScene_InGameWithCutscene,       // 12
    gameScene_MultiplayerMissionReview, // 13
    gameScene_Shop,                     // 14
    gameScene_LoadingScreen,            // 15
    gameScene_CutsceneWithStaticImages, // 16
    gameScene_Other2D,                  // 17
    gameScene_Other                     // 18
};

PluginKingdomHeartsReCoded::PluginKingdomHeartsReCoded(u32 gameCode)
{
    GameCode = gameCode;

    HUDState = 0;

    PriorGameScene = -1;
    GameScene = -1;
    priorMap = -1;
    Map = 0;
    UIScale = 4;
    AspectRatio = 0;
    ShowMap = true;

    _muchOlderHad3DOnTopScreen = false;
    _muchOlderHad3DOnBottomScreen = false;
    _olderHad3DOnTopScreen = false;
    _olderHad3DOnBottomScreen = false;
    _had3DOnTopScreen = false;
    _had3DOnBottomScreen = false;

    _SkipHdCutsceneCount = 0;
    _PlayingCutsceneBeforeCredits = false;
    _PlayingCredits = false;
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldTerminateIngameCutscene = false;
    _StoppedIngameCutscene = false;
    _ShouldStartReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;
    _ShouldUnmuteAfterCutscene = false;
    _ShouldHideScreenForTransitions = false;
    _CurrentCutscene = nullptr;

    PriorHotkeyMask = 0;
    PriorPriorHotkeyMask = 0;

    LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;
    SwitchTargetPressOnHold = false;
}

std::string PluginKingdomHeartsReCoded::assetsFolder() {
    return "recoded";
}

const char* PluginKingdomHeartsReCoded::gpuOpenGL_FS() {
    return kCompositorFS_KhReCoded;
};

const char* PluginKingdomHeartsReCoded::gpu3DOpenGL_VS_Z() {
    return kRenderVS_Z_KhReCoded;
};

void PluginKingdomHeartsReCoded::gpuOpenGL_FS_initVariables(GLuint CompShader) {
    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "PriorGameScene");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "GameScene");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "KHUIScale");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "TopScreenAspectRatio");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "ShowMap");
    // CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "ShowTarget");
    // CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "ShowMissionGauge");
    // CompGpuLoc[CompShader][7] = glGetUniformLocation(CompShader, "ShowMissionInfo");
}

void PluginKingdomHeartsReCoded::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1i(CompGpuLoc[CompShader][0], PriorGameScene);
    glUniform1i(CompGpuLoc[CompShader][1], GameScene);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1f(CompGpuLoc[CompShader][3], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][4], ShowMap ? 1 : 0);
    // glUniform1i(CompGpuLoc[CompShader][5], ShowTarget ? 1 : 0);
    // glUniform1i(CompGpuLoc[CompShader][6], ShowMissionGauge ? 1 : 0);
    // glUniform1i(CompGpuLoc[CompShader][7], ShowMissionInfo ? 1 : 0);
}

void PluginKingdomHeartsReCoded::gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags)
{
    CompGpu3DLoc[flags][0] = glGetUniformLocation(prog, "TopScreenAspectRatio");
    CompGpu3DLoc[flags][1] = glGetUniformLocation(prog, "GameScene");
    CompGpu3DLoc[flags][2] = glGetUniformLocation(prog, "KHUIScale");
}

void PluginKingdomHeartsReCoded::gpu3DOpenGL_VS_Z_updateVariables(u32 flags)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1f(CompGpu3DLoc[flags][0], aspectRatio);
    glUniform1i(CompGpu3DLoc[flags][1], GameScene);
    glUniform1i(CompGpu3DLoc[flags][2], UIScale);
}

void PluginKingdomHeartsReCoded::onLoadState()
{

}

void PluginKingdomHeartsReCoded::applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress)
{
    ramSearch(nds, *HotkeyPress);

    if (GameScene == -1)
    {
        return;
    }

    if (_PlayingCredits)
    {
        *InputMask = 0xFFF;
        return;
    }

    if (_RunningReplacementCutscene && (~(*InputMask)) & (1 << 3) && _SkipHdCutsceneCount > 0) { // Start (skip HD cutscene)
        if (!_ShouldTerminateIngameCutscene) { // can only skip after DS cutscene was skipped
            _SkipHdCutsceneCount--;
            _ShouldStopReplacementCutscene = true;
            *InputMask |= (1<<3);
        }
    }

    if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene) {
        *HotkeyPress |= (1<<5); // Fast Forward (skip DS cutscene)
    }

    if ((*HotkeyPress) & (1 << 15)) { // HUD Toggle
        hudToggle();
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithoutMap || GameScene == gameScene_InGameWithCutscene) {
        // So the arrow keys can be used to control the command menu
        if ((*HotkeyMask) & ((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21))) {
            *InputMask &= ~(1<<10); // X
            *InputMask |= (1<<5); // left
            *InputMask |= (1<<4); // right
            *InputMask |= (1<<6); // up
            *InputMask |= (1<<7); // down
            if (PriorPriorHotkeyMask & (1 << 18)) // Old D-pad left
                *InputMask &= ~(1<<5); // left
            if (PriorPriorHotkeyMask & (1 << 19)) // Old D-pad right
                *InputMask &= ~(1<<4); // right
            if (PriorPriorHotkeyMask & (1 << 20)) // Old D-pad up
                *InputMask &= ~(1<<6); // up
            if (PriorPriorHotkeyMask & (1 << 21)) // Old D-pad down
                *InputMask &= ~(1<<7); // down
        }
    }
    else {
        // So the arrow keys can be used as directionals
        if ((*HotkeyMask) & (1 << 18)) { // D-pad left
            *InputMask &= ~(1<<5); // left
        }
        if ((*HotkeyMask) & (1 << 19)) { // D-pad right
            *InputMask &= ~(1<<4); // right
        }
        if ((*HotkeyMask) & (1 << 20)) { // D-pad up
            *InputMask &= ~(1<<6); // up
        }
        if ((*HotkeyMask) & (1 << 21)) { // D-pad down
            *InputMask &= ~(1<<7); // down
        }
    }

    PriorPriorHotkeyMask = PriorHotkeyMask;
    PriorHotkeyMask = (*HotkeyMask);

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;
}

void PluginKingdomHeartsReCoded::applyTouchKeyMask(u32 TouchKeyMask)
{
    if (GameScene == -1)
    {
        return;
    }

    nds->SetTouchKeyMask(TouchKeyMask, true);
}

void PluginKingdomHeartsReCoded::hudToggle()
{
    ShowMap = !ShowMap;
}

const char* PluginKingdomHeartsReCoded::getGameSceneName()
{
    switch (GameScene) {
        case gameScene_Intro: return "Game scene: Intro";
        case gameScene_MainMenu: return "Game scene: Main menu";
        case gameScene_IntroLoadMenu: return "Game scene: Intro load menu";
        case gameScene_DayCounter: return "Game scene: Day counter";
        case gameScene_Cutscene: return "Game scene: Cutscene";
        case gameScene_InGameWithMap: return "Game scene: Ingame (with minimap)";
        case gameScene_InGameMenu: return "Game scene: Ingame menu";
        case gameScene_InGameSaveMenu: return "Game scene: Ingame save menu";
        case gameScene_InHoloMissionMenu: return "Game scene: Holo mission menu";
        case gameScene_PauseMenu: return "Game scene: Pause menu";
        case gameScene_Tutorial: return "Game scene: Tutorial";
        case gameScene_InGameWithCutscene: return "Game scene: Ingame (with cutscene)";
        case gameScene_MultiplayerMissionReview: return "Game scene: Multiplayer Mission Review";
        case gameScene_Shop: return "Game scene: Shop";
        case gameScene_LoadingScreen: return "Game scene: Loading screen";
        case gameScene_CutsceneWithStaticImages: return "Game scene: Cutscene with static images";
        case gameScene_Other2D: return "Game scene: Unknown (2D)";
        case gameScene_Other: return "Game scene: Unknown (3D)";
        default: return "Game scene: Unknown";
    }
}

bool PluginKingdomHeartsReCoded::isBufferBlack(unsigned int* buffer)
{
    if (!buffer) {
        return true;
    }

    // when the result is 'null' (filled with zeros), it's a false positive, so we need to exclude that scenario
    bool newIsNullScreen = true;
    bool newIsBlackScreen = true;
    for (int i = 0; i < 192*256; i++) {
        unsigned int color = buffer[i] & 0xFFFFFF;
        newIsNullScreen = newIsNullScreen && color == 0;
        newIsBlackScreen = newIsBlackScreen &&
                (color == 0 || color == 0x000080 || color == 0x010000 || (buffer[i] & 0xFFFFE0) == 0x018000);
        if (!newIsBlackScreen) {
            break;
        }
    }
    return !newIsNullScreen && newIsBlackScreen;
}

bool PluginKingdomHeartsReCoded::isTopScreen2DTextureBlack()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* topBuffer = nds->GPU.Framebuffer[FrontBuffer][0].get();
    return isBufferBlack(topBuffer);
}

bool PluginKingdomHeartsReCoded::isBottomScreen2DTextureBlack()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* bottomBuffer = nds->GPU.Framebuffer[FrontBuffer][1].get();
    return isBufferBlack(bottomBuffer);
}

bool PluginKingdomHeartsReCoded::shouldRenderFrame()
{
    return true;
}

int PluginKingdomHeartsReCoded::detectGameScene()
{
    if (nds == nullptr)
    {
        return GameScene;
    }

    bool wasSaveLoaded = isSaveLoaded();
    bool muchOlderHad3DOnTopScreen = _muchOlderHad3DOnTopScreen;
    bool muchOlderHad3DOnBottomScreen = _muchOlderHad3DOnBottomScreen;
    bool olderHad3DOnTopScreen = _olderHad3DOnTopScreen;
    bool olderHad3DOnBottomScreen = _olderHad3DOnBottomScreen;
    bool had3DOnTopScreen = _had3DOnTopScreen;
    bool had3DOnBottomScreen = _had3DOnBottomScreen;
    bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;
    bool has3DOnBottomScreen = (nds->PowerControl9 >> 9) == 1;
    _muchOlderHad3DOnTopScreen = _olderHad3DOnTopScreen;
    _muchOlderHad3DOnBottomScreen = _olderHad3DOnBottomScreen;
    _olderHad3DOnTopScreen = _had3DOnTopScreen;
    _olderHad3DOnBottomScreen = _had3DOnBottomScreen;
    _had3DOnTopScreen = has3DOnTopScreen;
    _had3DOnBottomScreen = has3DOnBottomScreen;
    bool has3DOnBothScreens = (muchOlderHad3DOnTopScreen || olderHad3DOnTopScreen || had3DOnTopScreen || has3DOnTopScreen) &&
                              (muchOlderHad3DOnBottomScreen || olderHad3DOnBottomScreen || had3DOnBottomScreen || has3DOnBottomScreen);

    bool isMainMenuOrIntroOrLoadMenu = nds->ARM7Read8(getAddressByCart(IS_MAIN_MENU_US, IS_MAIN_MENU_EU, IS_MAIN_MENU_JP)) == 0x00;
    bool isPauseScreen = nds->ARM7Read8(getAddressByCart(PAUSE_SCREEN_ADDRESS_US, PAUSE_SCREEN_ADDRESS_EU, PAUSE_SCREEN_ADDRESS_JP)) == PAUSE_SCREEN_VALUE_TRUE_PAUSE;
    bool isCutscene = nds->ARM7Read8(getAddressByCart(IS_CUTSCENE_US, IS_CUTSCENE_EU, IS_CUTSCENE_JP)) == 0x03;
    bool isUnplayableArea = nds->ARM7Read8(getAddressByCart(IS_PLAYABLE_AREA_US, IS_PLAYABLE_AREA_EU, IS_PLAYABLE_AREA_JP)) == 0x02;
    
    // Scale of brightness, from 0 (black) to 15 (every element is visible)
    u8 topScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_A.MasterBrightness);
    u8 botScreenBrightness = PARSE_BRIGHTNESS_FOR_WHITE_BACKGROUND(nds->GPU.GPU2D_B.MasterBrightness);

    if (isCutscene)
    {
        return gameScene_Cutscene;
    }

    if (isMainMenuOrIntroOrLoadMenu)
    {
        // Intro
        if (GameScene == -1 || GameScene == gameScene_Intro)
        {
            if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0)
            {
                return gameScene_Intro;
            }
        }

        // Intro save menu
        bool isIntroLoadMenu = (nds->GPU.GPU2D_B.BlendCnt == 4164 || nds->GPU.GPU2D_B.BlendCnt == 4161) &&
            (nds->GPU.GPU2D_A.EVA == 0 || nds->GPU.GPU2D_A.EVA == 16) &&
             nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 0 &&
            (nds->GPU.GPU2D_B.EVA < 10 && nds->GPU.GPU2D_B.EVA >= 0) && 
            (nds->GPU.GPU2D_B.EVB >  7 && nds->GPU.GPU2D_B.EVB <= 16) && nds->GPU.GPU2D_B.EVY == 0;

        if (isIntroLoadMenu)
        {
            return gameScene_IntroLoadMenu;
        }
        if (GameScene == gameScene_IntroLoadMenu)
        {
            if (nds->GPU.GPU3D.NumVertices != 8)
            {
                return gameScene_IntroLoadMenu;
            }
        }

        return gameScene_MainMenu;
    }

    if (has3DOnBothScreens)
    {
        return gameScene_InGameWithCutscene;
    }
    else if (has3DOnBottomScreen)
    {
        if (isUnplayableArea)
        {
            return gameScene_InGameMenu;
        }

        if (nds->GPU.GPU3D.RenderNumPolygons < 20)
        {
            if (nds->GPU.GPU2D_B.BlendCnt == 143 && nds->GPU.GPU2D_B.BlendAlpha == 16)
            {
                return gameScene_LoadingScreen;
            }
        }

        // Unknown
        return gameScene_Other;
    }
    else if (!has3DOnTopScreen)
    {
        // Unknown 2D
        return gameScene_Other2D;
    }

    // Shop has 2D and 3D segments, which is why it's on the top
    bool isShop = (nds->GPU.GPU3D.RenderNumPolygons == 264 && nds->GPU.GPU2D_A.BlendCnt == 0 && 
                   nds->GPU.GPU2D_B.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendAlpha == 16) ||
            (GameScene == gameScene_Shop && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0);
    if (isShop)
    {
        return gameScene_Shop;
    }

    if (isUnplayableArea)
    {
        return gameScene_InGameMenu;
    }

    if (isPauseScreen)
    {
        return gameScene_PauseMenu;
    }
    else if (GameScene == gameScene_PauseMenu)
    {
        return PriorGameScene;
    }

    // Regular gameplay
    return gameScene_InGameWithMap;
}

void PluginKingdomHeartsReCoded::setAspectRatio(float aspectRatio)
{
    if (GameScene != -1)
    {
        int aspectRatioKey = (int)round(0x1000 * aspectRatio);

        u32 aspectRatioMenuAddress = getAddressByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);

        if (nds->ARM7Read32(aspectRatioMenuAddress) == 0x00001555) {
            nds->ARM7Write32(aspectRatioMenuAddress, aspectRatioKey);
        }
    }

    AspectRatio = aspectRatio;
}

bool PluginKingdomHeartsReCoded::setGameScene(int newGameScene)
{
    bool updated = false;
    if (GameScene != newGameScene) 
    {
        updated = true;

        // Game scene
        PriorGameScene = GameScene;
        GameScene = newGameScene;
    }

    return updated;
}

u32 PluginKingdomHeartsReCoded::getCutsceneAddress(CutsceneEntry* entry)
{
    return getAddressByCart(entry->usAddress, entry->euAddress, entry->jpAddress);
}

u32 PluginKingdomHeartsReCoded::getAddressByCart(u32 usAddress, u32 euAddress, u32 jpAddress)
{
    u32 cutsceneAddress = 0;
    if (isUsaCart()) {
        cutsceneAddress = usAddress;
    }
    if (isEuropeCart()) {
        cutsceneAddress = euAddress;
    }
    if (isJapanCart()) {
        cutsceneAddress = jpAddress;
    }
    return cutsceneAddress;
}

CutsceneEntry* PluginKingdomHeartsReCoded::detectCutscene()
{
    return nullptr;
}

CutsceneEntry* PluginKingdomHeartsReCoded::detectSequenceCutscene()
{
    return nullptr;
}

void PluginKingdomHeartsReCoded::refreshCutscene()
{
#if !REPLACEMENT_CUTSCENES_ENABLED
    return;
#endif

    bool isCutsceneScene = GameScene == gameScene_Cutscene;
    CutsceneEntry* cutscene = detectCutscene();
    bool wasSaveLoaded = isSaveLoaded();

    // Avoiding bug with the credits
    // if (cutscene != nullptr && wasSaveLoaded && strcmp(cutscene->DsName, "843") == 0) {
    //     cutscene = nullptr;
    // }
    
    if (cutscene != nullptr) {
        onIngameCutsceneIdentified(cutscene);
    }

    // Natural progression for all cutscenes
    if (_ShouldTerminateIngameCutscene && !_RunningReplacementCutscene && isCutsceneScene) {
        _ShouldStartReplacementCutscene = true;
    }

    if (wasSaveLoaded) { // In game cutscenes (starting from Day 7)
        
        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && (!isCutsceneScene || _PlayingCutsceneBeforeCredits)) {
            onTerminateIngameCutscene();
        }

        if (_ShouldReturnToGameAfterCutscene && (!isCutsceneScene || _PlayingCredits || _PlayingCutsceneBeforeCredits)) {
            onReturnToGameAfterCutscene();
        }
    }
    else { // Intro when waiting on the title screen, theater, and cutscenes before Day 7

        // Intro when waiting on the title screen and cutscenes before Day 7
        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && !isCutsceneScene) {
            onTerminateIngameCutscene();
        }

        // Theater only
        // if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && isCutsceneScene) {
        //     u8 world = nds->ARM7Read8(getAddressByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
        //     u8 map = nds->ARM7Read8(getAddressByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
        //     u32 fullMap = world;
        //     fullMap = (fullMap << 4*2) | map;
        //     if (fullMap != 128) {
        //         onTerminateIngameCutscene();
        //     }
        // }

        if (_ShouldReturnToGameAfterCutscene) {
            onReturnToGameAfterCutscene();
        }
    }
}

std::string PluginKingdomHeartsReCoded::CutsceneFilePath(CutsceneEntry* cutscene) {
    std::string filename = "hd" + std::string(cutscene->DsName) + ".mp4";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (!std::filesystem::exists(fullPath)) {
        // TODO: KH try to load the cutscene from EPIC\Mare\MOVIE\?\en
        return "";
    }
    return fullPath.string();
}

void PluginKingdomHeartsReCoded::onIngameCutsceneIdentified(CutsceneEntry* cutscene) {
    if (_CurrentCutscene != nullptr && _CurrentCutscene->usAddress == cutscene->usAddress) {
        return;
    }

    // Workaround so those two cutscenes are played in sequence ingame,
    // without playing the first cutscene again
    bool wasSaveLoaded = isSaveLoaded();
    if (wasSaveLoaded) {
        // for (int seqIndex = 0; seqIndex < SequentialCutscenesSize; seqIndex++) {
        //     if (_CurrentCutscene != nullptr && strcmp(_CurrentCutscene->DsName, SequentialCutscenes[seqIndex][1]) == 0 &&
        //                                        strcmp(cutscene->DsName,         SequentialCutscenes[seqIndex][0]) == 0) {
        //         return;
        //     }
        // }
    }

    std::string path = CutsceneFilePath(cutscene);
    if (path == "") {
        return;
    }

    printf("Preparing to load cutscene: %s\n", cutscene->Name);
    log("Cutscene detected");

    _SkipHdCutsceneCount = 1;
    _CurrentCutscene = cutscene;
    _ShouldTerminateIngameCutscene = true;
    // _PlayingCredits = wasSaveLoaded && strcmp(cutscene->DsName, "843") == 0;
    // _PlayingCutsceneBeforeCredits = wasSaveLoaded && strcmp(cutscene->DsName, "842") == 0;
}
void PluginKingdomHeartsReCoded::onTerminateIngameCutscene() {
    if (_CurrentCutscene == nullptr) {
        return;
    }
    log("Ingame cutscene terminated");
    _ShouldTerminateIngameCutscene = false;
    _StoppedIngameCutscene = true;

    if (_PlayingCutsceneBeforeCredits || _PlayingCredits) {
        _StoppedIngameCutscene = false;
    }
}
void PluginKingdomHeartsReCoded::onReplacementCutsceneStarted() {
    log("Cutscene started");
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = true;
    _RunningReplacementCutscene = true;
}

void PluginKingdomHeartsReCoded::onReplacementCutsceneEnd() {
    log("Replacement cutscene ended");
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = true;

    CutsceneEntry* sequence = detectSequenceCutscene();
    _ShouldHideScreenForTransitions = sequence != nullptr;
}
void PluginKingdomHeartsReCoded::onReturnToGameAfterCutscene() {
    log("Returning to the game");
    _PlayingCredits = false;
    _PlayingCutsceneBeforeCredits = false;
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;
    _ShouldUnmuteAfterCutscene = true;

    // Ugly workaround to play one cutscene after another one, because both are skipped with a single "Start" click
    bool newCutsceneWillPlay = false;
    CutsceneEntry* sequence = detectSequenceCutscene();
    if (sequence != nullptr) {
        onIngameCutsceneIdentified(sequence);
        _ShouldStartReplacementCutscene = true;
        _ShouldUnmuteAfterCutscene = false;
        newCutsceneWillPlay = true;
    }

    if (!newCutsceneWillPlay) {
        _CurrentCutscene = nullptr;

        // u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
        // u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
        // nds->ARM7Write32(cutsceneAddress, 0x0);
        // nds->ARM7Write32(cutsceneAddress2, 0x0);
    }
}

bool PluginKingdomHeartsReCoded::refreshGameScene()
{
    int newGameScene = detectGameScene();
    
    debugLogs(newGameScene);

    bool updated = setGameScene(newGameScene);

    refreshCutscene();

    return updated;
}

u32 PluginKingdomHeartsReCoded::getCurrentMission()
{
    return 0;
    // return nds->ARM7Read8(getAddressByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
}

u32 PluginKingdomHeartsReCoded::getCurrentMap()
{
    return 0;

    // if (GameScene == -1)
    // {
    //     return 0;
    // }

    // u8 world = nds->ARM7Read8(getAddressByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    // u8 map = nds->ARM7Read8(getAddressByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
    // u32 fullMap = world;
    // fullMap = (fullMap << 4*2) | map;

    // if (Map != fullMap) {
    //     priorMap = Map;
    //     Map = fullMap;
    // }

    // if (Map == 128) { // cutscene
    //     return priorMap;
    // }

    // return Map;
}

bool PluginKingdomHeartsReCoded::isSaveLoaded()
{
    return getCurrentMap() != 0;
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80000000 ? '1' : '0'), \
  ((byte) & 0x40000000 ? '1' : '0'), \
  ((byte) & 0x20000000 ? '1' : '0'), \
  ((byte) & 0x10000000 ? '1' : '0'), \
  ((byte) & 0x08000000 ? '1' : '0'), \
  ((byte) & 0x04000000 ? '1' : '0'), \
  ((byte) & 0x02000000 ? '1' : '0'), \
  ((byte) & 0x01000000 ? '1' : '0'), \
  ((byte) & 0x00800000 ? '1' : '0'), \
  ((byte) & 0x00400000 ? '1' : '0'), \
  ((byte) & 0x00200000 ? '1' : '0'), \
  ((byte) & 0x00100000 ? '1' : '0'), \
  ((byte) & 0x00080000 ? '1' : '0'), \
  ((byte) & 0x00040000 ? '1' : '0'), \
  ((byte) & 0x00020000 ? '1' : '0'), \
  ((byte) & 0x00010000 ? '1' : '0'), \
  ((byte) & 0x00008000 ? '1' : '0'), \
  ((byte) & 0x00004000 ? '1' : '0'), \
  ((byte) & 0x00002000 ? '1' : '0'), \
  ((byte) & 0x00001000 ? '1' : '0'), \
  ((byte) & 0x00000800 ? '1' : '0'), \
  ((byte) & 0x00000400 ? '1' : '0'), \
  ((byte) & 0x00000200 ? '1' : '0'), \
  ((byte) & 0x00000100 ? '1' : '0'), \
  ((byte) & 0x00000080 ? '1' : '0'), \
  ((byte) & 0x00000040 ? '1' : '0'), \
  ((byte) & 0x00000020 ? '1' : '0'), \
  ((byte) & 0x00000010 ? '1' : '0'), \
  ((byte) & 0x00000008 ? '1' : '0'), \
  ((byte) & 0x00000004 ? '1' : '0'), \
  ((byte) & 0x00000002 ? '1' : '0'), \
  ((byte) & 0x00000001 ? '1' : '0') 

#define PRINT_AS_8_BIT_HEX(ADDRESS) printf("0x%08x: 0x%02x\n", ADDRESS, nds->ARM7Read8(ADDRESS))
#define PRINT_AS_8_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read8(ADDRESS)))

#define PRINT_AS_16_BIT_HEX(ADDRESS) printf("0x%08x: 0x%04x\n", ADDRESS, nds->ARM7Read16(ADDRESS))
#define PRINT_AS_16_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read16(ADDRESS)))

#define PRINT_AS_32_BIT_HEX(ADDRESS) printf("0x%08x: 0x%08x\n", ADDRESS, nds->ARM7Read32(ADDRESS))
#define PRINT_AS_32_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read32(ADDRESS)))

void PluginKingdomHeartsReCoded::debugLogs(int gameScene)
{
    // PRINT_AS_8_BIT_HEX(0x0205e704);
    // PRINT_AS_8_BIT_HEX(0x0205e908);
    // printf("\n");

    if (!DEBUG_MODE_ENABLED) {
        return;
    }

    printf("Game scene: %d\n", gameScene);
    printf("Current map: %d\n", getCurrentMap());
    printf("Is save loaded: %d\n", isSaveLoaded() ? 1 : 0);
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