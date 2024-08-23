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

// 0x2C => intro and main menu
#define IS_MAIN_MENU_US      0x0204242d
#define IS_MAIN_MENU_EU      0x0204244d
#define IS_MAIN_MENU_JP      0x0204288d
#define IS_MAIN_MENU_JP_REV1 0x0204284d

// 0x03 => cutscene; 0x01 => not cutscene
#define IS_CUTSCENE_US      0x02044640
#define IS_CUTSCENE_EU      0x02044660
#define IS_CUTSCENE_JP      0x02044aa0
#define IS_CUTSCENE_JP_REV1 0x02044a60

// 0x80 => playable (example: ingame); 0x04 => not playable (menus)
#define IS_PLAYABLE_AREA_US      0x020446c6
#define IS_PLAYABLE_AREA_EU      0x020446e6
#define IS_PLAYABLE_AREA_JP      0x02044b26
#define IS_PLAYABLE_AREA_JP_REV1 0x02044ae6

#define CURRENT_WORLD_US      0x0204C2CF
#define CURRENT_WORLD_EU      0x0204C2EF
#define CURRENT_WORLD_JP      0x0204C72F
#define CURRENT_WORLD_JP_REV1 0x0204C6EF

#define CURRENT_MISSION_US      0x0204C21C
#define CURRENT_MISSION_EU      0x0204C23C
#define CURRENT_MISSION_JP      0x0204C67C
#define CURRENT_MISSION_JP_REV1 0x0204C63C

#define IS_DAYS_COUNTER_US      0x0204f6c5
#define IS_DAYS_COUNTER_EU      0x0204f6e5
#define IS_DAYS_COUNTER_JP      0x020508a9
#define IS_DAYS_COUNTER_JP_REV1 0x02050869

#define IS_DAYS_COUNTER_VALUE_US      0x00
#define IS_DAYS_COUNTER_VALUE_EU      0x00
#define IS_DAYS_COUNTER_VALUE_JP      0x10
#define IS_DAYS_COUNTER_VALUE_JP_REV1 0x10

#define CURRENT_MAIN_MENU_VIEW_US      0x0205ac04
#define CURRENT_MAIN_MENU_VIEW_EU      0x0205ac24
#define CURRENT_MAIN_MENU_VIEW_JP      0x0205a5e4
#define CURRENT_MAIN_MENU_VIEW_JP_REV1 0x0205a5a5

#define LOAD_MENU_MAIN_MENU_VIEW_US      0xB0
#define LOAD_MENU_MAIN_MENU_VIEW_EU      0xD0
#define LOAD_MENU_MAIN_MENU_VIEW_JP      0xD0
#define LOAD_MENU_MAIN_MENU_VIEW_JP_REV1 0x36

#define CUTSCENE_ADDRESS_US      0x02093A4C
#define CUTSCENE_ADDRESS_EU      0x02093A6C
#define CUTSCENE_ADDRESS_JP      0x0209394C
#define CUTSCENE_ADDRESS_JP_REV1 0x0209390C

#define CUTSCENE_ADDRESS_2_US      0x02093A94
#define CUTSCENE_ADDRESS_2_EU      0x02093AB4
#define CUTSCENE_ADDRESS_2_JP      0x02093994
#define CUTSCENE_ADDRESS_2_JP_REV1 0x02093954

#define CURRENT_INGAME_MENU_VIEW_US      0x020b572c
#define CURRENT_INGAME_MENU_VIEW_EU      0x020b574c
#define CURRENT_INGAME_MENU_VIEW_JP      0x020b19ac
#define CURRENT_INGAME_MENU_VIEW_JP_REV1 0x020b196c

#define IS_TUTORIAL_ADDRESS_US      0x020d7408
#define IS_TUTORIAL_ADDRESS_EU      0x020d7410
#define IS_TUTORIAL_ADDRESS_JP      0x020d6950
#define IS_TUTORIAL_ADDRESS_JP_REV1 0x020d6910

#define IS_TUTORIAL_VALUE_US      0x55
#define IS_TUTORIAL_VALUE_EU      0xb0
#define IS_TUTORIAL_VALUE_JP      0x30
#define IS_TUTORIAL_VALUE_JP_REV1 0xb0

#define CURRENT_MAP_FROM_WORLD_US      0x02188EE6
#define CURRENT_MAP_FROM_WORLD_EU      0x02189CC6
#define CURRENT_MAP_FROM_WORLD_JP      0x02187FC6
#define CURRENT_MAP_FROM_WORLD_JP_REV1 0x02188046

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US      0x02194CC3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU      0x02195AA3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP      0x02193E23
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1 0x02193DA3

#define CUTSCENE_SKIP_START_FRAMES_COUNT  40
#define SWITCH_TARGET_PRESS_FRAME_LIMIT   100
#define SWITCH_TARGET_TIME_BETWEEN_SWITCH 20
#define LOCK_ON_PRESS_FRAME_LIMIT         100

enum
{
    gameScene_Intro,                    // 0
    gameScene_MainMenu,                 // 1
    gameScene_IntroLoadMenu,            // 2
    gameScene_DayCounter,               // 3
    gameScene_Cutscene,                 // 4
    gameScene_InGameWithMap,            // 5
    gameScene_InGameMenu,               // 6
    gameScene_PauseMenu,                // 7
    gameScene_Tutorial,                 // 8
    gameScene_InGameWithCutscene,       // 9
    gameScene_MultiplayerMissionReview, // 10
    gameScene_Shop,                     // 11
    gameScene_LoadingScreen,            // 12
    gameScene_RoxasThoughts,            // 13
    gameScene_Other                     // 14
};

CutsceneEntry Cutscenes[] =
{
    {"802",    "802_opening",                       0x088b2e00, 0x08b3d400, 0x0876e800},
    {"803",    "803_meet_xion",                     0x0987ec00, 0x09b09200, 0x097c8800},
    {"804",    "804_roxas_recusant_sigil",          0x09ae9400, 0x09d73a00, 0x09a4e000},
    {"805",    "805_the_dark_margin",               0x09b80600, 0x09e0ac00, 0x09aeb200},
    {"806",    "806_sora_entering_pod",             0x09e83800, 0x0a10de00, 0x09e0ba00},
    {"808",    "808_sunset_memory",                 0x09f24c00, 0x0a1af200, 0x09eb2e00},
    {"809",    "809_xions_defeat",                  0x09f79400, 0x0a203a00, 0x09f0ac00},
    {"810",    "810_the_main_in_black_reflects",    0x09ff8000, 0x0a282600, 0x09f8d400},
    {"813",    "813_xions_defeat",                  0x0a13f600, 0x0a3c9c00, 0x0a0e1600},
    {"814",    "814_sora_walk",                     0x0a677c00, 0x0a902200, 0x0a64f000},
    {"815",    "815_sora_release_kairi",            0x0a6e4200, 0x0a96e800, 0x0a6bf000},
    {"816",    "816_kairi_memories",                0x0a7a9200, 0x0aa33800, 0x0a78ca00},
    {"817",    "817_namine_and_diz",                0x0a857600, 0x0aae1c00, 0x0a845800},
    {"818",    "818_why_the_sun_sets_red",          0x0ab4be00, 0x0add6400, 0x0ab57800},
    {"819",    "819_sora_wakes_up",                 0x0afeac00, 0x0afeac00, 0x0afeac00}, // double cutscene complement
    {"821",    "821_snarl_of_memories",             0x0b043e00, 0x0b2ce400, 0x0b084a00},
    {"822",    "822_riku_takes_care_of_xion",       0x0b514600, 0x0b79ec00, 0x0b57ea00},
    {"823",    "823_roxas_passes_by",               0x0b5b5e00, 0x0b840400, 0x0b626e00},
    {"824",    "824_xions_dream",                   0x0b65a200, 0x0b8e4800, 0x0b6d1800},
    {"825",    "825_xions_capture",                 0x0b8a7a00, 0x0bb32000, 0x0b937600},
    {"826",    "826_hollow_bastion_memories",       0x0bd74600, 0x0bffec00, 0x0be21a00},
    {"827",    "827_agrabah_keyhole_memory",        0x0be7e000, 0x0c108600, 0x0bf35400},
    {"828",    "828_xion_and_riku",                 0x0bedf200, 0x0c169800, 0x0bf9ae00},
    {"829",    "829_rikus_resolve",                 0x0c76a800, 0x0c9f4e00, 0x0c873400},
    {"830",    "830_mickey_and_riku_ansem",         0x0c863a00, 0x0caee000, 0x0c981000},
    {"831",    "831_xion_and_namine",               0x0ca47c00, 0x0ccd2200, 0x0cb79400},
    {"832",    "832_xion_and_axel_face_off",        0x0cb01c00, 0x0cd8c200, 0x0cc3b000},
    {"833",    "833_xion_attacks",                  0x0cee2000, 0x0d16c600, 0x0d043200},
    {"834",    "834_winner",                        0x0d45bc00, 0x0d6e6200, 0x0d5f7800},
    {"835",    "835_skyscrapper_battle",            0x0d5e0400, 0x0d86aa00, 0x0d782e00},
    {"836",    "836_roxas_and_riku",                0x0d6f9400, 0x0d983a00, 0x0d8a7e00},
    {"837",    "837_riku_turns_into_ansem",         0x0da1ea00, 0x0dca9000, 0x0dbed000},
    {"838",    "838_clocktower",                    0x0e063600, 0x0e063600, 0x0e063600}, // double cutscene complement
    {"839_de", "839_riku_please_stop_him_de",       0x0e0db400, 0x0e0db400, 0x0e0db400}, // double cutscene complement
    {"839_en", "839_riku_please_stop_him_en",       0x0e0e1200, 0x0e0e1200, 0x0e0e1200}, // double cutscene complement
    {"839_es", "839_riku_please_stop_him_es",       0x0e0e6c00, 0x0e0e6c00, 0x0e0e6c00}, // double cutscene complement
    {"839_fr", "839_riku_please_stop_him_fr",       0x0e0ecc00, 0x0e0ecc00, 0x0e0ecc00}, // double cutscene complement
    {"839_it", "839_riku_please_stop_him_it",       0x0e0f1600, 0x0e0f1600, 0x0e0f1600}, // double cutscene complement
    {"840",    "840_after_the_battle",              0x0e0f5e00, 0x0e380400, 0x0e2e4200},
    {"841",    "841_xion_fading_from_clocktower",   0x0e444c00, 0x0e444c00, 0x0e444c00}, // double cutscene complement
    {"842",    "842_a_new_day",                     0x0e4bd400, 0x0e747a00, 0x0e6dfa00},
    {"843",    "843_the_usual_spot",                0x0e641200, 0x0e8cb800, 0x0e873800},
    {"845",    "845_the_dark_margin_sora_whisper",  0x0e6fa600, 0x0e984c00, 0x0e938e00},
    {"846",    "846_axel_and_saix",                 0x0e75bc00, 0x0e9e6200, 0x0e99ee00},
    {"847",    "847_roxas_leaves_the_organization", 0x0e9c2000, 0x0ec4c600, 0x0ec12c00},
    {"848",    "848_xions_end",                     0x0eb91800, 0x0ee1be00, 0x0edf4600},
};

PluginKingdomHeartsDays::PluginKingdomHeartsDays(u32 gameCode)
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
    _ignore3DOnBottomScreen = false;
    _priorIgnore3DOnBottomScreen = false;
    _priorPriorIgnore3DOnBottomScreen = false;

    _StartPressCount = 0;
    _CanSkipHdCutscene = false;
    _SkipDsCutscene = false;
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
    _NextCutscene = nullptr;

    PriorHotkeyMask = 0;
    PriorPriorHotkeyMask = 0;

    LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;
    SwitchTargetPressOnHold = false;
}

void PluginKingdomHeartsDays::onLoadROM() {
    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
    // *((u32*)&rom[0x01b613b5]) = 0x0033e943;
    // *((u32*)&rom[0x050c57cb]) = 0x0033e943;
    // *((u32*)&rom[0x05b3e4ff]) = 0x0033e943;
    // *((u32*)&rom[0x07afdb4d]) = 0x0033e943;
    // *((u32*)&rom[0x07f97608]) = 0x0033e943;
    // *((u32*)&rom[0x0b02c7ff]) = 0x0033e943;
    // *((u32*)&rom[0x0e104248]) = 0x0033e943;
    // *((u32*)&rom[0x0e6e691f]) = 0x0033e943;

    // for (u32 address = 0; address <= 0xF78870C; address+= 1) {
    //     if (*((u8*)&rom[address])       == '_' &&
    //         *((u8*)&rom[address + 0x1]) == 'l' &&
    //         *((u8*)&rom[address + 0x2]) == 'a' &&
    //         *((u8*)&rom[address + 0x3]) == 'r')
    //     {
    //         *((u8*)&rom[address])       = '_';
    //         *((u8*)&rom[address + 0x1]) = 'a';
    //         *((u8*)&rom[address + 0x2]) = 'b';
    //         *((u8*)&rom[address + 0x3]) = 'c';
    //     }

    //     if (*((u8*)&rom[address])       == '_' &&
    //         *((u8*)&rom[address + 0x1]) == 'v' &&
    //         *((u8*)&rom[address + 0x2]) == 'e' &&
    //         *((u8*)&rom[address + 0x3]) == 'n')
    //     {
    //         *((u8*)&rom[address])       = '_';
    //         *((u8*)&rom[address + 0x1]) = 'l';
    //         *((u8*)&rom[address + 0x2]) = 'a';
    //         *((u8*)&rom[address + 0x3]) = 'r';
    //     }
    // }

    // u16 tmp = *((u16*)&rom[0x0033e911]);
    // *((u16*)&rom[0x0033e911]) = *((u16*)&rom[0x0033e943]);
    // *((u16*)&rom[0x0033e943]) = tmp;

    // *((u32*)&rom[0x0334d70d]) = 0x1033e943; // original: 0x1033e911;
    
    // printf("Roxas folder: %02x%02x\n", rom[0x0033e934], rom[0x0033e935]);

    // rom[0x0033e934] = 0x76;
    // rom[0x0033e935] = 0x65;



    // PRINT_AS_8_BIT_HEX(0x0205ac04);
    // PRINT_AS_8_BIT_HEX(0x020d73e4);
    // printf("\n");
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
    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "PriorGameScene");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "GameScene");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "KHUIScale");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "TopScreenAspectRatio");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "ShowMap");
    CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "ShowTarget");
    CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "ShowMissionGauge");
    CompGpuLoc[CompShader][7] = glGetUniformLocation(CompShader, "ShowMissionInfo");
    CompGpuLoc[CompShader][8] = glGetUniformLocation(CompShader, "HideScene");
    CompGpuLoc[CompShader][9] = glGetUniformLocation(CompShader, "MainMenuView");
}

void PluginKingdomHeartsDays::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1i(CompGpuLoc[CompShader][0], PriorGameScene);
    glUniform1i(CompGpuLoc[CompShader][1], GameScene);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1f(CompGpuLoc[CompShader][3], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][4], ShowMap ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][5], ShowTarget ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][6], ShowMissionGauge ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][7], ShowMissionInfo ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][8], _ShouldHideScreenForTransitions ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][9], getCurrentMainMenuView());
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

void PluginKingdomHeartsDays::onLoadState()
{
    GameScene = gameScene_InGameWithMap;
}

void PluginKingdomHeartsDays::applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress)
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

    if (_RunningReplacementCutscene && (_SkipDsCutscene || (~(*InputMask)) & (1 << 3)) && _CanSkipHdCutscene) { // Start (skip HD cutscene)
        _SkipDsCutscene = true;
        if (!_ShouldTerminateIngameCutscene) { // can only skip after DS cutscene was skipped
            _SkipDsCutscene = false;
            _CanSkipHdCutscene = false;
            _ShouldStopReplacementCutscene = true;
            *InputMask |= (1<<3);
        }
        else {
            if (_StartPressCount == 0) {
                _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT;
            }
        }
    }

    if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene) {
        if (_StartPressCount > 0) {
            _StartPressCount--;
            *InputMask &= ~(1<<3); // Start (skip DS cutscene)
        }
        *HotkeyMask |= (1<<4); // Fast Forward (skip DS cutscene)
    }

    if (*HotkeyPress & (1 << 15)) { // HUD Toggle
        hudToggle();
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithCutscene) {
        // Enabling X + D-Pad
        if ((*HotkeyMask) & ((1 << 18) | (1 << 19) | (1 << 20) | (1 << 21))) { // D-pad
            u32 dpadMenuAddress = getAddressByCart(INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1);

            if (nds->ARM7Read8(dpadMenuAddress) & 0x02) {
                nds->ARM7Write8(dpadMenuAddress, nds->ARM7Read8(dpadMenuAddress) - 0x02);
            }
        }

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

        // R / Lock On
        {
            if ((*HotkeyMask) & (1 << 16)) {
                if (LastLockOnPress == 1) {
                    LastLockOnPress = 0;
                }
                else if (LastLockOnPress > 10) {
                    LastLockOnPress = 0;
                }
            }
            if (LastLockOnPress == 0) {
                *InputMask &= ~(1<<8); // R
            }
            if (LastLockOnPress == 4 || LastLockOnPress == 5 || LastLockOnPress == 6) {
                *InputMask &= ~(1<<8); // R (three frames later)
            }
        }

        // Switch Target
        {
            if ((*HotkeyMask) & (1 << 17)) {
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
                *InputMask &= ~(1<<8); // R
            }
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

        if ((*HotkeyMask) & (1 << 16)) { // R / Lock On
            *InputMask &= ~(1<<8); // R
        }
    }

    PriorPriorHotkeyMask = PriorHotkeyMask;
    PriorHotkeyMask = (*HotkeyMask);

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;
}

void PluginKingdomHeartsDays::applyTouchKeyMask(u32 TouchKeyMask)
{
    if (GameScene == -1)
    {
        return;
    }

    nds->SetTouchKeyMask(TouchKeyMask);
}

void PluginKingdomHeartsDays::hudToggle()
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
        case gameScene_InGameMenu: return "Game scene: Ingame menu";
        case gameScene_PauseMenu: return "Game scene: Pause menu";
        case gameScene_Tutorial: return "Game scene: Tutorial";
        case gameScene_InGameWithCutscene: return "Game scene: Ingame (with cutscene)";
        case gameScene_MultiplayerMissionReview: return "Game scene: Multiplayer Mission Review";
        case gameScene_Shop: return "Game scene: Shop";
        case gameScene_LoadingScreen: return "Game scene: Loading screen";
        case gameScene_RoxasThoughts: return "Game scene: Roxas Thoughts";
        case gameScene_Other: return "Game scene: Unknown";
        default: return "Game scene: REALLY unknown";
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

bool PluginKingdomHeartsDays::isTopScreen2DTextureBlack()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* topBuffer = nds->GPU.Framebuffer[FrontBuffer][0].get();
    return isBufferBlack(topBuffer);
}

bool PluginKingdomHeartsDays::isBottomScreen2DTextureBlack()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    u32* bottomBuffer = nds->GPU.Framebuffer[FrontBuffer][1].get();
    return isBufferBlack(bottomBuffer);
}

bool PluginKingdomHeartsDays::shouldRenderFrame()
{
    if (GameScene == gameScene_InGameWithCutscene)
    {
        if (nds->PowerControl9 >> 15 != 0) // 3D on top screen
        {
            _hasVisible3DOnBottomScreen = !IsBottomScreen2DTextureBlack;

            if (nds->GPU.GPU2D_A.MasterBrightness == 0 && nds->GPU.GPU2D_B.MasterBrightness == 32784) {
                _hasVisible3DOnBottomScreen = false;
            }

            if (nds->GPU.GPU2D_B.MasterBrightness & (1 << 14)) { // fade to white, on "Mission Complete"
                _hasVisible3DOnBottomScreen = false;
            }
        }
        else // 3D on bottom screen
        {
            IsBottomScreen2DTextureBlack = isBottomScreen2DTextureBlack();

            _priorPriorIgnore3DOnBottomScreen = _priorIgnore3DOnBottomScreen;
            _priorIgnore3DOnBottomScreen = _ignore3DOnBottomScreen;
            _ignore3DOnBottomScreen = false;

            if (_hasVisible3DOnBottomScreen) {
                int FrontBuffer = nds->GPU.FrontBuffer;
                u32* bottomBuffer = nds->GPU.Framebuffer[FrontBuffer][1].get();
                if (bottomBuffer) {
                    unsigned int color = bottomBuffer[(192*256)/2 + 96] & 0xFFFFFF;
                    if (color == 0) {
                        _ignore3DOnBottomScreen = true;
                    }
                }
            }
        }

        bool showBottomScreen = _hasVisible3DOnBottomScreen && !(_ignore3DOnBottomScreen && _priorIgnore3DOnBottomScreen && _priorPriorIgnore3DOnBottomScreen);
        return (nds->PowerControl9 >> 15 != 0) ? !showBottomScreen : showBottomScreen;
    }

    return true;
}

int PluginKingdomHeartsDays::detectGameScene()
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

    u8 mainMenuOrIntroOrLoadMenuVal = nds->ARM7Read8(getAddressByCart(IS_MAIN_MENU_US, IS_MAIN_MENU_EU, IS_MAIN_MENU_JP, IS_MAIN_MENU_JP_REV1));
    bool isMainMenuOrIntroOrLoadMenu = mainMenuOrIntroOrLoadMenuVal == 0x28 || mainMenuOrIntroOrLoadMenuVal == 0x2C;
    bool isCutscene = nds->ARM7Read8(getAddressByCart(IS_CUTSCENE_US, IS_CUTSCENE_EU, IS_CUTSCENE_JP, IS_CUTSCENE_JP_REV1)) == 0x03;
    bool isUnplayableArea = nds->ARM7Read8(getAddressByCart(IS_PLAYABLE_AREA_US, IS_PLAYABLE_AREA_EU, IS_PLAYABLE_AREA_JP, IS_PLAYABLE_AREA_JP_REV1)) == 0x04;
    bool isLoadMenu = nds->ARM7Read8(getAddressByCart(CURRENT_MAIN_MENU_VIEW_US, CURRENT_MAIN_MENU_VIEW_EU, CURRENT_MAIN_MENU_VIEW_JP, CURRENT_MAIN_MENU_VIEW_JP_REV1)) ==
        getAddressByCart(LOAD_MENU_MAIN_MENU_VIEW_US, LOAD_MENU_MAIN_MENU_VIEW_EU, LOAD_MENU_MAIN_MENU_VIEW_JP, LOAD_MENU_MAIN_MENU_VIEW_JP_REV1);
    bool isDaysCounter = nds->ARM7Read8(getAddressByCart(IS_DAYS_COUNTER_US, IS_DAYS_COUNTER_EU, IS_DAYS_COUNTER_JP, IS_DAYS_COUNTER_JP_REV1)) ==
        getAddressByCart(IS_DAYS_COUNTER_VALUE_US, IS_DAYS_COUNTER_VALUE_EU, IS_DAYS_COUNTER_VALUE_JP, IS_DAYS_COUNTER_VALUE_JP_REV1);
    bool isTutorial = nds->ARM7Read8(getAddressByCart(IS_TUTORIAL_ADDRESS_US, IS_TUTORIAL_ADDRESS_EU, IS_TUTORIAL_ADDRESS_JP, IS_TUTORIAL_ADDRESS_JP_REV1)) ==
        getAddressByCart(IS_TUTORIAL_VALUE_US, IS_TUTORIAL_VALUE_EU, IS_TUTORIAL_VALUE_JP, IS_TUTORIAL_VALUE_JP_REV1);

    if (isCutscene)
    {
        return gameScene_Cutscene;
    }

    if (isMainMenuOrIntroOrLoadMenu)
    {
        if (!wasSaveLoaded && has3DOnTopScreen && !has3DOnBottomScreen)
        {
            if (isLoadMenu)
            {
                return gameScene_IntroLoadMenu;
            }
            
            bool mayBeMainMenu = !wasSaveLoaded && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;

            if (GameScene == gameScene_IntroLoadMenu)
            {
                if (mayBeMainMenu)
                {
                    return gameScene_MainMenu;
                }
            }

            if (GameScene == gameScene_MainMenu)
            {
                mayBeMainMenu = nds->GPU.GPU3D.NumVertices < 15 && nds->GPU.GPU3D.NumPolygons < 15;
                if (mayBeMainMenu) {
                    return gameScene_MainMenu;
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
        }

        return gameScene_MainMenu;
    }
    if (!wasSaveLoaded && (GameScene == -1 || GameScene == gameScene_Intro))
    {
        return gameScene_Intro;
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
        return gameScene_Other;
    }

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

    if (nds->GPU.GPU3D.RenderNumPolygons < 20)
    {
        if (isDaysCounter)
        {
            return gameScene_DayCounter;
        }

        if (nds->GPU.GPU2D_B.MasterBrightness == 32784)
        {
            return gameScene_RoxasThoughts;
        }
    }

    if (isTutorial)
    {
        return gameScene_Tutorial;
    }

    // Pause Menu
    bool inMissionPauseMenu = nds->GPU.GPU2D_A.EVY == 8 && (nds->GPU.GPU2D_B.EVY == 8 || nds->GPU.GPU2D_B.EVY == 16);
    if (inMissionPauseMenu)
    {
        return gameScene_PauseMenu;
    }
    else if (GameScene == gameScene_PauseMenu)
    {
        return PriorGameScene;
    }

    // Regular gameplay with a map
    return gameScene_InGameWithMap;
}

void PluginKingdomHeartsDays::setAspectRatio(float aspectRatio)
{
    if (GameScene != -1)
    {
        int aspectRatioKey = (int)round(0x1000 * aspectRatio);

        u32 aspectRatioMenuAddress = getAddressByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP, ASPECT_RATIO_ADDRESS_JP_REV1);

        if (nds->ARM7Read32(aspectRatioMenuAddress) == 0x00001555) {
            nds->ARM7Write32(aspectRatioMenuAddress, aspectRatioKey);
        }
    }

    AspectRatio = aspectRatio;
}

bool PluginKingdomHeartsDays::setGameScene(int newGameScene)
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

u32 PluginKingdomHeartsDays::getCutsceneAddress(CutsceneEntry* entry)
{
    return getAddressByCart(entry->usAddress, entry->euAddress, entry->jpAddress, entry->jpAddress - 0x200);
}

u32 PluginKingdomHeartsDays::getAddressByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address)
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
    if (isJapanCartRev1()) {
        cutsceneAddress = jpRev1Address;
    }
    return cutsceneAddress;
}

CutsceneEntry* PluginKingdomHeartsDays::detectCutscene()
{
    if (GameScene == -1)
    {
        return nullptr;
    }

    u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
    u32 cutsceneAddressValue = nds->ARM7Read32(cutsceneAddress);
    if (cutsceneAddressValue == 0 || (cutsceneAddressValue - (cutsceneAddressValue & 0xFF)) == 0xea000000) {
        cutsceneAddressValue = 0;
    }

    u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
    u32 cutsceneAddressValue2 = nds->ARM7Read32(cutsceneAddress2);
    if (cutsceneAddressValue2 == 0 || (cutsceneAddressValue2 - (cutsceneAddressValue2 & 0xFF)) == 0xea000000) {
        cutsceneAddressValue2 = 0;
    }

    CutsceneEntry* cutscene1 = nullptr;
    CutsceneEntry* cutscene2 = nullptr;
    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getCutsceneAddress(entry) == cutsceneAddressValue) {
            cutscene1 = entry;
        }
        if (getCutsceneAddress(entry) == cutsceneAddressValue2) {
            cutscene2 = entry;
        }
    }

    if (cutscene1 == nullptr && cutscene2 != nullptr) {
        cutscene1 = cutscene2;
    }

    if (cutscene1 == nullptr && cutsceneAddressValue != 0 && cutsceneAddressValue2 != 0) {
        // printf("Unknown cutscene: 0x%08x - 0x%08x\n", cutsceneAddressValue, cutsceneAddressValue2);
    }

    return cutscene1;
}

CutsceneEntry* PluginKingdomHeartsDays::detectSequenceCutscene()
{
    return nullptr;
}

void PluginKingdomHeartsDays::refreshCutscene()
{
#if !REPLACEMENT_CUTSCENES_ENABLED
    return;
#endif

    bool isCutsceneScene = GameScene == gameScene_Cutscene;
    CutsceneEntry* cutscene = detectCutscene();
    bool wasSaveLoaded = isSaveLoaded();
    
    if (cutscene != nullptr) {
        onIngameCutsceneIdentified(cutscene);
    }

    bool cutsceneEnded = !isCutsceneScene || _NextCutscene != nullptr;

    // Natural progression for all cutscenes
    if (_ShouldTerminateIngameCutscene && !_RunningReplacementCutscene && isCutsceneScene) {
        _ShouldStartReplacementCutscene = true;
    }

    if (wasSaveLoaded) { // In game cutscenes (starting from Day 7)
        
        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && cutsceneEnded) {
            onTerminateIngameCutscene();
        }

        if (_ShouldReturnToGameAfterCutscene && (cutsceneEnded || _PlayingCredits)) {
            onReturnToGameAfterCutscene();
        }
    }
    else { // Intro when waiting on the title screen, theater, and cutscenes before Day 7

        if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && !isCutsceneScene) {
            onTerminateIngameCutscene();
        }

        if (_ShouldReturnToGameAfterCutscene) {
            onReturnToGameAfterCutscene();
        }
    }
}

std::string PluginKingdomHeartsDays::CutsceneFilePath(CutsceneEntry* cutscene) {
    std::string filename = "hd" + std::string(cutscene->DsName) + ".mp4";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (!std::filesystem::exists(fullPath)) {
        // TODO: KH try to load the cutscene from EPIC\Mare\MOVIE\Days\en
        return "";
    }
    return fullPath.string();
}

void PluginKingdomHeartsDays::onIngameCutsceneIdentified(CutsceneEntry* cutscene) {
    if (_CurrentCutscene != nullptr && _CurrentCutscene->usAddress == cutscene->usAddress) {
        return;
    }

    std::string path = CutsceneFilePath(cutscene);
    if (path == "") {
        return;
    }

    if (_CurrentCutscene != nullptr) {
        _NextCutscene = cutscene;
        return;
    }

    printf("Preparing to load cutscene: %s\n", cutscene->Name);
    log("Cutscene detected");

    _CanSkipHdCutscene = true;
    _CurrentCutscene = cutscene;
    _NextCutscene = nullptr;
    _ShouldTerminateIngameCutscene = true;
    _PlayingCredits = isSaveLoaded() && strcmp(cutscene->DsName, "843") == 0;
}
void PluginKingdomHeartsDays::onTerminateIngameCutscene() {
    if (_CurrentCutscene == nullptr) {
        return;
    }
    log("Ingame cutscene terminated");
    _ShouldTerminateIngameCutscene = false;
    _StoppedIngameCutscene = true;

    if (_PlayingCredits) {
        _StoppedIngameCutscene = false;
    }
}
void PluginKingdomHeartsDays::onReplacementCutsceneStarted() {
    log("Cutscene started");
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = true;
    _RunningReplacementCutscene = true;
}

void PluginKingdomHeartsDays::onReplacementCutsceneEnd() {
    log("Replacement cutscene ended");
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = true;
    _ShouldHideScreenForTransitions = false;
}
void PluginKingdomHeartsDays::onReturnToGameAfterCutscene() {
    log("Returning to the game");
    _StartPressCount = 0;
    _PlayingCredits = false;
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;
    _ShouldUnmuteAfterCutscene = true;

    _CurrentCutscene = nullptr;

    if (_NextCutscene == nullptr) {
        u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
        u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
        nds->ARM7Write32(cutsceneAddress, 0x0);
        nds->ARM7Write32(cutsceneAddress2, 0x0);
    }
}

bool PluginKingdomHeartsDays::refreshGameScene()
{
    int newGameScene = detectGameScene();
    
    debugLogs(newGameScene);

    bool updated = setGameScene(newGameScene);

    refreshCutscene();

    return updated;
}

u32 PluginKingdomHeartsDays::getCurrentMission()
{
    return nds->ARM7Read8(getAddressByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
}

// The states below also happen in multiple other places outside the main menu menus
// 0 -> none
// 1 -> main menu root / character selection
// 2 -> panel
// 3 -> holo-mission / challenges
// 4 -> roxas's diary / enemy profile
// 5 -> tutorials and mission review
// 6 -> config
// 7 -> save
u32 PluginKingdomHeartsDays::getCurrentMainMenuView()
{
    if (GameScene == -1)
    {
        return 0;
    }

    u8 val = nds->ARM7Read8(getAddressByCart(CURRENT_INGAME_MENU_VIEW_US, CURRENT_INGAME_MENU_VIEW_EU, CURRENT_INGAME_MENU_VIEW_JP, CURRENT_INGAME_MENU_VIEW_JP_REV1));
    if (val == 0x00) return 1;
    if (val == 0x02) return 2;
    if (val == 0x01) return 3;
    if (val == 0x07) return 4;
    if (val == 0x06) return 5;
    if (val == 0x05) return 6;
    if (val == 0x04) return 7;
    return 0;
}

// map == 0 => No map
// map >> 8 == 0 => Twilight Town and Day 357
// map >> 8 == 1 => Wonderland
// map >> 8 == 2 => Olympus
// map >> 8 == 3 => Agrabah
// map >> 8 == 4 => The World That Never Was
// map >> 8 == 5 => Halloween Town
// map >> 8 == 6 => ?
// map >> 8 == 7 => ?
// map >> 8 == 8 => Neverland
// map >> 8 == 9 => Beast's Castle
u32 PluginKingdomHeartsDays::getCurrentMap()
{
    if (GameScene == -1)
    {
        return 0;
    }

    u8 world = nds->ARM7Read8(getAddressByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    u8 map = nds->ARM7Read8(getAddressByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
    u32 fullMap = world;
    fullMap = (fullMap << 4*2) | map;

    if (Map != fullMap) {
        priorMap = Map;
        Map = fullMap;
    }

    if (Map == 128) { // cutscene
        return priorMap;
    }

    return Map;
}

bool PluginKingdomHeartsDays::isSaveLoaded()
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

void PluginKingdomHeartsDays::debugLogs(int gameScene)
{
    // PRINT_AS_8_BIT_HEX(0x02046a60); // 0x9c
    // PRINT_AS_8_BIT_HEX(0x020d73f0); // 0xd0
    // printf("\n");

    if (!DEBUG_MODE_ENABLED) {
        return;
    }

    printf("Game scene: %d\n",  gameScene);
    printf("Current map: %d\n", getCurrentMap());
    printf("Current main menu view: %d\n", getCurrentMainMenuView());
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