#include "PluginKingdomHeartsDays.h"

#include "PluginKingdomHeartsDays_GPU_OpenGL_shaders.h"
#include "PluginKingdomHeartsDays_GPU3D_OpenGL_shaders.h"

#include <math.h>

namespace Plugins
{

u32 PluginKingdomHeartsDays::usGamecode = 1162300249;
u32 PluginKingdomHeartsDays::euGamecode = 1346849625;
u32 PluginKingdomHeartsDays::jpGamecode = 1246186329;

#define ASPECT_RATIO_ADDRESS_US      0x02023C9C
#define ASPECT_RATIO_ADDRESS_EU      0x02023CBC
#define ASPECT_RATIO_ADDRESS_JP      0x02023C9C
#define ASPECT_RATIO_ADDRESS_JP_REV1 0x02023C9C

#define CUTSCENE_ADDRESS_US      0x02093A4C
#define CUTSCENE_ADDRESS_EU      0x02093A4C
#define CUTSCENE_ADDRESS_JP      0x02093A4C
#define CUTSCENE_ADDRESS_JP_REV1 0x02093A4C

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US      0x02194CC3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU      0x02195AA3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP      0x02193E23
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1 0x02193DA3

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
    gameScene_InGameWithoutMap,         // 6
    gameScene_InGameMenu,               // 7
    gameScene_InGameSaveMenu,           // 8
    gameScene_InHoloMissionMenu,        // 9
    gameScene_PauseMenu,                // 10
    gameScene_Tutorial,                 // 11
    gameScene_InGameWithCutscene,       // 12
    gameScene_MultiplayerMissionReview, // 13
    gameScene_Shop,                     // 14
    gameScene_LoadingScreen,            // 15
    gameScene_Other2D,                  // 16
    gameScene_Other                     // 17
};

CutsceneEntry Cutscenes[] =
{
     // still couldn't find a proper way to detect the opening from the Theater
    {"802.mods",    "802_opening",                       0x088b2e00, 0x088b2e00, 0x088b2e00},
    {"803.mods",    "803_meet_xion",                     0x0987ec00, 0x0987ec00, 0x0987ec00},
    {"804.mods",    "804_roxas_recusant_sigil",          0x09ae9400, 0x09ae9400, 0x09ae9400},
    {"805.mods",    "805_the_dark_margin",               0x09b80600, 0x09b80600, 0x09b80600},
    {"806.mods",    "806_sora_entering_pod",             0x09e83800, 0x09e83800, 0x09e83800},
    {"808.mods",    "808_sunset_memory",                 0x09f24c00, 0x09f24c00, 0x09f24c00},
    {"809.mods",    "809_xions_defeat",                  0x09f79400, 0x09f79400, 0x09f79400},
    {"810.mods",    "810_the_main_in_black_reflects",    0x09ff8000, 0x09ff8000, 0x09ff8000},
    {"813.mods",    "813_xions_defeat",                  0x0a13f600, 0x0a13f600, 0x0a13f600},
    {"814.mods",    "814_sora_walk",                     0x0a677c00, 0x0a677c00, 0x0a677c00},
    {"815.mods",    "815_sora_release_kairi",            0x0a6e4200, 0x0a6e4200, 0x0a6e4200},
    {"816.mods",    "816_kairi_memories",                0x0a7a9200, 0x0a7a9200, 0x0a7a9200},
    {"817.mods",    "817_namine_and_diz",                0x0a857600, 0x0a857600, 0x0a857600},
    {"818.mods",    "818_why_the_sun_sets_red",          0x0ab4be00, 0x0ab4be00, 0x0ab4be00},
    {"819.mods",    "819_sora_wakes_up",                 0x0afeac00, 0x0afeac00, 0x0afeac00},
    {"821.mods",    "821_snarl_of_memories",             0x0b043e00, 0x0b043e00, 0x0b043e00},
    {"822.mods",    "822_riku_takes_care_of_xion",       0x0b514600, 0x0b514600, 0x0b514600},
    {"823.mods",    "823_roxas_passes_by",               0x0b5b5e00, 0x0b5b5e00, 0x0b5b5e00},
    {"824.mods",    "824_xions_dream",                   0x0b65a200, 0x0b65a200, 0x0b65a200},
    {"825.mods",    "825_xions_capture",                 0x0b8a7a00, 0x0b8a7a00, 0x0b8a7a00},
    {"826.mods",    "826_hollow_bastion_memories",       0x0bd74600, 0x0bd74600, 0x0bd74600},
    {"827.mods",    "827_agrabah_keyhole_memory",        0x0be7e000, 0x0be7e000, 0x0be7e000},
    {"828.mods",    "828_xion_and_riku",                 0x0bedf200, 0x0bedf200, 0x0bedf200},
    {"829.mods",    "829_rikus_resolve",                 0x0c76a800, 0x0c76a800, 0x0c76a800},
    {"830.mods",    "830_mickey_and_riku_ansem",         0x0c863a00, 0x0c863a00, 0x0c863a00},
    {"831.mods",    "831_xion_and_namine",               0x0ca47c00, 0x0ca47c00, 0x0ca47c00},
    {"832.mods",    "832_xion_and_axel_face_off",        0x0cb01c00, 0x0cb01c00, 0x0cb01c00},
    {"833.mods",    "833_xion_attacks",                  0x0cee2000, 0x0cee2000, 0x0cee2000},
    {"834.mods",    "834_winner",                        0x0d45bc00, 0x0d45bc00, 0x0d45bc00},
    {"835.mods",    "835_skyscrapper_battle",            0x0d5e0400, 0x0d5e0400, 0x0d5e0400},
    {"836.mods",    "836_roxas_and_riku",                0x0d6f9400, 0x0d6f9400, 0x0d6f9400},
    {"837.mods",    "837_riku_turns_into_ansem",         0x0da1ea00, 0x0da1ea00, 0x0da1ea00},
    {"838.mods",    "838_clocktower",                    0x0e063600, 0x0e063600, 0x0e063600},
    {"839_de.mods", "839_riku_please_stop_him_de",       0x0e0db400, 0x0e0db400, 0x0e0db400},
    {"839_en.mods", "839_riku_please_stop_him_en",       0x0e0e1200, 0x0e0e1200, 0x0e0e1200},
    {"839_es.mods", "839_riku_please_stop_him_es",       0x0e0e6c00, 0x0e0e6c00, 0x0e0e6c00},
    {"839_fr.mods", "839_riku_please_stop_him_fr",       0x0e0ecc00, 0x0e0ecc00, 0x0e0ecc00},
    {"839_it.mods", "839_riku_please_stop_him_it",       0x0e0f1600, 0x0e0f1600, 0x0e0f1600},
    {"840.mods",    "840_after_the_battle",              0x0e0f5e00, 0x0e0f5e00, 0x0e0f5e00},
    {"841.mods",    "841_xion_fading_from_clocktower",   0x0e444c00, 0x0e444c00, 0x0e444c00},
    {"842.mods",    "842_a_new_day",                     0x0e4bd400, 0x0e4bd400, 0x0e4bd400},
    {"843.mods",    "843_the_usual_spot",                0x0e641200, 0x0e641200, 0x0e641200},
    {"845.mods",    "845_the_dark_margin_sora_whisper",  0x0e6fa600, 0x0e6fa600, 0x0e6fa600},
    {"846.mods",    "846_axel_and_saix",                 0x0e75bc00, 0x0e75bc00, 0x0e75bc00},
    {"847.mods",    "847_roxas_leaves_the_organization", 0x0e9c2000, 0x0e9c2000, 0x0e9c2000},
    {"848.mods",    "848_xions_end",                     0x0eb91800, 0x0eb91800, 0x0eb91800},
};

PluginKingdomHeartsDays::PluginKingdomHeartsDays(u32 gameCode)
{
    GameCode = gameCode;

    HUDState = 0;

    priorGameScene = -1;
    GameScene = -1;
    UIScale = 4;
    AspectRatio = 0;
    ShowMap = true;
    ShowTarget = false;
    ShowMissionGauge = false;
    ShowMissionInfo = false;

    _muchOlderHad3DOnTopScreen = false;
    _muchOlderHad3DOnBottomScreen = false;
    _olderHad3DOnTopScreen = false;
    _olderHad3DOnBottomScreen = false;
    _had3DOnTopScreen = false;
    _had3DOnBottomScreen = false;

    _hasVisible3DOnBottomScreen = false;

    CurrentCutscene = nullptr;

    PriorHotkeyMask = 0;
    PriorPriorHotkeyMask = 0;

    LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;
    SwitchTargetPressOnHold = false;
}

std::string PluginKingdomHeartsDays::assetsFolder() {
    return "days";
}

const char* PluginKingdomHeartsDays::gpuOpenGL_FS() {
    return kCompositorFS_KhDays;
};

const char* PluginKingdomHeartsDays::gpu3DOpenGL_VS_Z() {
    return kRenderVS_Z_KhDays;
};

void PluginKingdomHeartsDays::gpuOpenGL_FS_initVariables(GLuint CompShader) {
    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "IsBottomScreen2DTextureBlack");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "IsTopScreen2DTextureBlack");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "PriorGameScene");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "GameScene");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "KHUIScale");
    CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "TopScreenAspectRatio");
    CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "ShowMap");
    CompGpuLoc[CompShader][7] = glGetUniformLocation(CompShader, "ShowTarget");
    CompGpuLoc[CompShader][8] = glGetUniformLocation(CompShader, "ShowMissionGauge");
    CompGpuLoc[CompShader][9] = glGetUniformLocation(CompShader, "ShowMissionInfo");
}

void PluginKingdomHeartsDays::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1i(CompGpuLoc[CompShader][0], IsBottomScreen2DTextureBlack ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][1], IsTopScreen2DTextureBlack ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][2], priorGameScene);
    glUniform1i(CompGpuLoc[CompShader][3], GameScene);
    glUniform1i(CompGpuLoc[CompShader][4], UIScale);
    glUniform1f(CompGpuLoc[CompShader][5], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][6], ShowMap ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][7], ShowTarget ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][8], ShowMissionGauge ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][9], ShowMissionInfo ? 1 : 0);
}

void PluginKingdomHeartsDays::gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags)
{
    CompGpu3DLoc[flags][0] = glGetUniformLocation(prog, "TopScreenAspectRatio");
    CompGpu3DLoc[flags][1] = glGetUniformLocation(prog, "GameScene");
    CompGpu3DLoc[flags][2] = glGetUniformLocation(prog, "KHUIScale");
    CompGpu3DLoc[flags][3] = glGetUniformLocation(prog, "ShowMissionInfo");
}

void PluginKingdomHeartsDays::gpu3DOpenGL_VS_Z_updateVariables(u32 flags)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1f(CompGpu3DLoc[flags][0], aspectRatio);
    glUniform1i(CompGpu3DLoc[flags][1], GameScene);
    glUniform1i(CompGpu3DLoc[flags][2], UIScale);
    glUniform1i(CompGpu3DLoc[flags][3], ShowMissionInfo ? 1 : 0);
}

u32 PluginKingdomHeartsDays::applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress)
{
    if (HotkeyPress & (1 << 15)) { // HUD Toggle
        hudToggle(nds);
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithoutMap || GameScene == gameScene_InGameWithCutscene) {
        // Enabling X + D-Pad
        if (HotkeyMask & ((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21))) { // D-pad
            u32 dpadMenuAddress = 0;
            if (isUsaCart()) {
                dpadMenuAddress = INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US;
            }
            if (isEuropeCart()) {
                dpadMenuAddress = INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU;
            }
            if (isJapanCart()) {
                dpadMenuAddress = INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP;
            }
            if (isJapanCartRev1()) {
                dpadMenuAddress = INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1;
            }

            if (nds->ARM7Read8(dpadMenuAddress) & 0x02) {
                nds->ARM7Write8(dpadMenuAddress, nds->ARM7Read8(dpadMenuAddress) - 0x02);
            }
        }

        // So the arrow keys can be used to control the command menu
        if (HotkeyMask & ((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21))) {
            InputMask &= ~(1<<10); // X
            InputMask |= (1<<5); // left
            InputMask |= (1<<4); // right
            InputMask |= (1<<6); // up
            InputMask |= (1<<7); // down
            if (PriorPriorHotkeyMask & (1 << 18)) // Old D-pad left
                InputMask &= ~(1<<5); // left
            if (PriorPriorHotkeyMask & (1 << 19)) // Old D-pad right
                InputMask &= ~(1<<4); // right
            if (PriorPriorHotkeyMask & (1 << 20)) // Old D-pad up
                InputMask &= ~(1<<6); // up
            if (PriorPriorHotkeyMask & (1 << 21)) // Old D-pad down
                InputMask &= ~(1<<7); // down
        }

        // R / Lock On
        {
            if (HotkeyMask & (1 << 16)) {
                if (LastLockOnPress == 1) {
                    LastLockOnPress = 0;
                }
                else if (LastLockOnPress > 10) {
                    LastLockOnPress = 0;
                }
            }
            if (LastLockOnPress == 0) {
                InputMask &= ~(1<<8); // R
            }
            if (LastLockOnPress == 4 || LastLockOnPress == 5 || LastLockOnPress == 6) {
                InputMask &= ~(1<<8); // R (three frames later)
            }
        }

        // Switch Target
        {
            if (HotkeyMask & (1 << 17)) {
                if (LastSwitchTargetPress == 1) {
                    LastSwitchTargetPress = 0;
                }
                else if (LastSwitchTargetPress > SWITCH_TARGET_TIME_BETWEEN_SWITCH) {
                    LastSwitchTargetPress = 0;
                }
                else {
                    SwitchTargetPressOnHold = true;
                }
            }
            if (SwitchTargetPressOnHold) {
                if (LastSwitchTargetPress == SWITCH_TARGET_TIME_BETWEEN_SWITCH ||
                    LastSwitchTargetPress == SWITCH_TARGET_TIME_BETWEEN_SWITCH + 1 ||
                    LastSwitchTargetPress == SWITCH_TARGET_TIME_BETWEEN_SWITCH + 2) {
                    LastSwitchTargetPress = 0;
                    SwitchTargetPressOnHold = false;
                }
            }
            if (LastSwitchTargetPress == 0 || LastSwitchTargetPress == 1 || LastSwitchTargetPress == 2) {
                InputMask &= ~(1<<8); // R
            }
        }
    }
    else {
        // So the arrow keys can be used as directionals
        if (HotkeyMask & (1 << 18)) { // D-pad left
            InputMask &= ~(1<<5); // left
        }
        if (HotkeyMask & (1 << 19)) { // D-pad right
            InputMask &= ~(1<<4); // right
        }
        if (HotkeyMask & (1 << 20)) { // D-pad up
            InputMask &= ~(1<<6); // up
        }
        if (HotkeyMask & (1 << 21)) { // D-pad down
            InputMask &= ~(1<<7); // down
        }

        if (HotkeyMask & (1 << 16)) { // R / Lock On
            InputMask &= ~(1<<8); // R
        }
    }

    PriorPriorHotkeyMask = PriorHotkeyMask;
    PriorHotkeyMask = HotkeyMask;

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;

    return InputMask;
}

void PluginKingdomHeartsDays::applyTouchKeyMask(melonDS::NDS* nds, u32 TouchKeyMask)
{
    nds->SetTouchKeyMask(TouchKeyMask);
}

void PluginKingdomHeartsDays::hudToggle(melonDS::NDS* nds)
{
    HUDState = (HUDState + 1) % 3;
    if (HUDState == 0) { // exploration mode
        ShowMap = true;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = false;
    }
    else if (HUDState == 1) { // mission details mode
        ShowMap = false;
        ShowTarget = true;
        ShowMissionGauge = true;
        ShowMissionInfo = false;
    }
    else { // mission mode
        ShowMap = false;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = true;
    }
}

const char* PluginKingdomHeartsDays::getGameSceneName()
{
    switch (GameScene) {
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
        case gameScene_LoadingScreen: return "Game scene: Loading screen";
        case gameScene_Other2D: return "Game scene: Unknown (2D)";
        case gameScene_Other: return "Game scene: Unknown (3D)";
        default: return "Game scene: Unknown";
    }
}

bool PluginKingdomHeartsDays::isBufferBlack(unsigned int* buffer)
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

bool PluginKingdomHeartsDays::isTopScreen2DTextureBlack(melonDS::NDS* nds)
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* topBuffer = nds->GPU.Framebuffer[FrontBuffer][0].get();
    return isBufferBlack(topBuffer);
}

bool PluginKingdomHeartsDays::isBottomScreen2DTextureBlack(melonDS::NDS* nds)
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* bottomBuffer = nds->GPU.Framebuffer[FrontBuffer][1].get();
    return isBufferBlack(bottomBuffer);
}

bool PluginKingdomHeartsDays::shouldSkipFrame(melonDS::NDS* nds)
{
    bool isTopBlack = isTopScreen2DTextureBlack(nds);
    bool isBottomBlack = isBottomScreen2DTextureBlack(nds);

    IsBottomScreen2DTextureBlack = isBottomBlack;
    IsTopScreen2DTextureBlack = isTopBlack;

    if (GameScene == gameScene_InGameWithCutscene)
    {
        if (nds->PowerControl9 >> 15 != 0) // 3D on top screen
        {
            _hasVisible3DOnBottomScreen = !isBottomBlack;

            if (nds->GPU.GPU2D_A.MasterBrightness == 0 && nds->GPU.GPU2D_B.MasterBrightness == 32784) {
                _hasVisible3DOnBottomScreen = false;
            }

            if (nds->GPU.GPU2D_B.MasterBrightness & (1 << 14)) { // fade to white, on "Mission Complete"
                _hasVisible3DOnBottomScreen = false;
            }

            if (_hasVisible3DOnBottomScreen) {
                return true;
            }
        }
        else // 3D on bottom screen
        {
            return !_hasVisible3DOnBottomScreen;
        }
    }

    return false;
}

int PluginKingdomHeartsDays::detectGameScene(melonDS::NDS* nds)
{
    // printf("0x02194CBF: %08x %08x\n", nds->ARM7Read32(0x02194CBF), nds->ARM7Read32(0x02194CC3));

    // Also happens during intro, during the start of the mission review, on some menu screens; those seem to use real 2D elements
    bool no3D = nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 0;

    // 3D element mimicking 2D behavior
    bool doesntLook3D = nds->GPU.GPU3D.RenderNumPolygons < 20;

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
        bool mayBeMainMenu = has3DOnTopScreen && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;

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

            mayBeMainMenu = has3DOnTopScreen && nds->GPU.GPU3D.NumVertices < 15 && nds->GPU.GPU3D.NumPolygons < 15;
            if (mayBeMainMenu) {
                return gameScene_MainMenu;
            }
        }

        // Day 50 specific condition
        if (GameScene == gameScene_InGameWithMap && nds->GPU.GPU2D_B.BlendCnt == 172 && nds->GPU.GPU2D_B.BlendAlpha == 16 &&
                                                    nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0)
        {
            return gameScene_InGameWithMap;
        }

        // Day counter
        if (GameScene == gameScene_DayCounter && !no3D)
        {
            return gameScene_DayCounter;
        }
        if (GameScene != gameScene_Intro && has3DOnTopScreen)
        {
            if (nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1)
            {
                return gameScene_DayCounter; // 1 digit
            }
            if (nds->GPU.GPU3D.NumVertices == 8 && nds->GPU.GPU3D.NumPolygons == 2 && nds->GPU.GPU3D.RenderNumPolygons == 2)
            {
                return gameScene_DayCounter; // 2 digits
            }
            if (nds->GPU.GPU3D.NumVertices == 12 && nds->GPU.GPU3D.NumPolygons == 3 && nds->GPU.GPU3D.RenderNumPolygons == 3)
            {
                return gameScene_DayCounter; // 3 digits
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
            mayBeMainMenu = has3DOnTopScreen && nds->GPU.GPU3D.NumVertices > 0 && nds->GPU.GPU3D.NumPolygons > 0;
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
            if (nds->GPU.GPU3D.RenderNumPolygons > 0)
            {
                return gameScene_InGameMenu;
            }

            if (nds->GPU.GPU2D_B.BlendCnt == 143 && nds->GPU.GPU2D_B.BlendAlpha == 16)
            {
                return gameScene_LoadingScreen;
            }

            return gameScene_Cutscene;
        }

        // Bottom cutscene
        bool isBottomCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
             nds->GPU.GPU2D_A.EVA == 16 && nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 9 &&
             nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (isBottomCutscene)
        {
            return gameScene_Cutscene;
        }

        mayBeMainMenu = has3DOnTopScreen && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 0;
        if (mayBeMainMenu)
        {
            return gameScene_MainMenu;
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

        // Story Mode - Normal missions - Day 357
        inHoloMissionMenu = ((nds->GPU.GPU3D.NumVertices == 332 && nds->GPU.GPU3D.NumPolygons == 102 && nds->GPU.GPU3D.RenderNumPolygons == 102) ||
                             (nds->GPU.GPU3D.NumVertices == 340 && nds->GPU.GPU3D.NumPolygons == 104 && nds->GPU.GPU3D.RenderNumPolygons == 104)) &&
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
        bool inMissionPauseMenu = nds->GPU.GPU2D_A.EVY == 8 && (nds->GPU.GPU2D_B.EVY == 8 || nds->GPU.GPU2D_B.EVY == 16);
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

    if (GameScene == gameScene_InGameWithMap)
    {
        return gameScene_InGameWithCutscene;
    }
    if (has3DOnBottomScreen)
    {
        if (nds->GPU.GPU3D.RenderNumPolygons < 100)
        {
            return gameScene_InGameMenu;
        }

        return gameScene_InGameWithCutscene;
    }
    
    // Unknown
    return gameScene_Other;
}

void PluginKingdomHeartsDays::setAspectRatio(melonDS::NDS* nds, float aspectRatio)
{
    int aspectRatioKey = (int)round(0x1000 * aspectRatio);

    u32 aspectRatioMenuAddress = 0;
    if (isUsaCart()) {
        aspectRatioMenuAddress = ASPECT_RATIO_ADDRESS_US;
    }
    if (isEuropeCart()) {
        aspectRatioMenuAddress = ASPECT_RATIO_ADDRESS_EU;
    }
    if (isJapanCart()) {
        aspectRatioMenuAddress = ASPECT_RATIO_ADDRESS_JP;
    }
    if (isJapanCartRev1()) {
        aspectRatioMenuAddress = ASPECT_RATIO_ADDRESS_JP_REV1;
    }

    if (nds->ARM7Read32(aspectRatioMenuAddress) == 0x00001555) {
        nds->ARM7Write32(aspectRatioMenuAddress, aspectRatioKey);
    }

    AspectRatio = aspectRatio;
}

bool PluginKingdomHeartsDays::setGameScene(melonDS::NDS* nds, int newGameScene)
{
    bool updated = false;
    if (GameScene != newGameScene) 
    {
        updated = true;

        // Game scene
        priorGameScene = GameScene;
        GameScene = newGameScene;
    }

    return updated;
}

u32 PluginKingdomHeartsDays::getAddress(CutsceneEntry* entry) {
    if (isUsaCart()) {
        return entry->usAddress;
    }
    if (isEuropeCart()) {
        return entry->euAddress;
    }
    if (isJapanCart()) {
        return entry->jpAddress;
    }
    if (isJapanCartRev1()) {
        return entry->jpAddress;
    }
    return 0;
}

CutsceneEntry* PluginKingdomHeartsDays::detectCutscene(melonDS::NDS* nds)
{
    u32 cutsceneAddress = 0;
    if (isUsaCart()) {
        cutsceneAddress = CUTSCENE_ADDRESS_US;
    }
    if (isEuropeCart()) {
        cutsceneAddress = CUTSCENE_ADDRESS_EU;
    }
    if (isJapanCart()) {
        cutsceneAddress = CUTSCENE_ADDRESS_JP;
    }
    if (isJapanCartRev1()) {
        cutsceneAddress = CUTSCENE_ADDRESS_JP_REV1;
    }

    u32 cutsceneAddressValue = nds->ARM7Read32(cutsceneAddress);

    if (cutsceneAddressValue == 0) {
        return nullptr;
    }

    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getAddress(entry) == cutsceneAddressValue) {
            return entry;
        }
    }

    printf("Unknown cutscene: 0x%08x\n", cutsceneAddressValue);

    return nullptr;
}

void PluginKingdomHeartsDays::refreshCutscene(melonDS::NDS* nds)
{
    CutsceneEntry* lastCutscene = CurrentCutscene;
    CutsceneEntry* cutscene = nullptr;
    if (GameScene == gameScene_Cutscene) {
        cutscene = detectCutscene(nds);
    }
    if (cutscene == nullptr) {
        if (lastCutscene != nullptr) {
            onIngameCutsceneEnd(nds, lastCutscene);
        }
    }
    else {
        if (lastCutscene == nullptr) {
            onIngameCutsceneStart(nds, cutscene);
        }
        if (lastCutscene != nullptr && cutscene->usAddress != lastCutscene->usAddress) {
            onIngameCutsceneEnd(nds, lastCutscene);
            onIngameCutsceneStart(nds, cutscene);
        }
    }
    CurrentCutscene = cutscene;
}

bool PluginKingdomHeartsDays::refreshGameScene(melonDS::NDS* nds)
{
    int newGameScene = detectGameScene(nds);
    
    debugLogs(nds, newGameScene);

    bool updated = setGameScene(nds, newGameScene);

    refreshCutscene(nds);

    return updated;
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

#define PRINT_AS_32_BIT_HEX(ADDRESS) printf(#ADDRESS": 0x%08x\n", nds->ARM7Read32(ADDRESS))
#define PRINT_AS_32_BIT_BIN(ADDRESS) printf(#ADDRESS": "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(nds->ARM7Read32(ADDRESS)))

void PluginKingdomHeartsDays::debugLogs(melonDS::NDS* nds, int gameScene)
{
    if (!DEBUG_MODE_ENABLED) {
        return;
    }

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