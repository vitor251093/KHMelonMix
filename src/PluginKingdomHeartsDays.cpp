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

#define CURRENT_MISSION_US      0x0204C21C
#define CURRENT_MISSION_EU      0x0204C23C
#define CURRENT_MISSION_JP      0x0204C67C
#define CURRENT_MISSION_JP_REV1 0x0204C63C

#define CURRENT_WORLD_US      0x0204C2CF
#define CURRENT_WORLD_EU      0x0204C2EF
#define CURRENT_WORLD_JP      0x0204C72F
#define CURRENT_WORLD_JP_REV1 0x0204C6EF

#define CURRENT_MAP_FROM_WORLD_US      0x02188EE6
#define CURRENT_MAP_FROM_WORLD_EU      0x02188EE6 // TODO: KH
#define CURRENT_MAP_FROM_WORLD_JP      0x02188EE6 // TODO: KH
#define CURRENT_MAP_FROM_WORLD_JP_REV1 0x02188EE6 // TODO: KH

#define CUTSCENE_ADDRESS_US      0x02093A4C
#define CUTSCENE_ADDRESS_EU      0x02093A6C
#define CUTSCENE_ADDRESS_JP      0x02093A4C // TODO: KH
#define CUTSCENE_ADDRESS_JP_REV1 0x02093A4C // TODO: KH

#define CUTSCENE_ADDRESS_2_US      0x02093A94
#define CUTSCENE_ADDRESS_2_EU      0x02093AB4 // TODO: KH
#define CUTSCENE_ADDRESS_2_JP      0x02093A94 // TODO: KH
#define CUTSCENE_ADDRESS_2_JP_REV1 0x02093A94 // TODO: KH

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US      0x02194CC3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU      0x02195AA3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP      0x02193E23
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1 0x02193DA3

#define CUTSCENE_SKIP_START_FRAMES_COUNT 60

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
    gameScene_RoxasThoughts,            // 16
    gameScene_Other2D,                  // 17
    gameScene_Other                     // 18
};

CutsceneEntry Cutscenes[] =
{
     // still couldn't find a proper way to detect the opening from the Theater
    {"802",    "802_opening",                       0x088b2e00, 0x08B3D400, 0x088b2e00}, // lacks JP
    {"803",    "803_meet_xion",                     0x0987ec00, 0x09b09200, 0x0987ec00}, // lacks JP
    {"804",    "804_roxas_recusant_sigil",          0x09ae9400, 0x09ae9400, 0x09ae9400}, // lacks EU, JP
    {"805",    "805_the_dark_margin",               0x09b80600, 0x09E0AC00, 0x09b80600}, // lacks JP
    {"806",    "806_sora_entering_pod",             0x09e83800, 0x09e83800, 0x09e83800}, // lacks EU, JP
    {"808",    "808_sunset_memory",                 0x09f24c00, 0x09f24c00, 0x09f24c00}, // lacks EU, JP
    {"809",    "809_xions_defeat",                  0x09f79400, 0x09f79400, 0x09f79400}, // lacks EU, JP
    {"810",    "810_the_main_in_black_reflects",    0x09ff8000, 0x0A282600, 0x09ff8000}, // lacks JP
    {"813",    "813_xions_defeat",                  0x0a13f600, 0x0A3C9C00, 0x0a13f600}, // lacks JP
    {"814",    "814_sora_walk",                     0x0a677c00, 0x0a677c00, 0x0a677c00}, // lacks EU, JP
    {"815",    "815_sora_release_kairi",            0x0a6e4200, 0x0a6e4200, 0x0a6e4200}, // lacks EU, JP
    {"816",    "816_kairi_memories",                0x0a7a9200, 0x0a7a9200, 0x0a7a9200}, // lacks EU, JP
    {"817",    "817_namine_and_diz",                0x0a857600, 0x0AAE1C00, 0x0a857600}, // lacks JP
    {"818",    "818_why_the_sun_sets_red",          0x0ab4be00, 0x0add6400, 0x0ab4be00}, // lacks JP
    {"819",    "819_sora_wakes_up",                 0x0afeac00, 0x0afeac00, 0x0afeac00}, // double cutscene complement
    {"821",    "821_snarl_of_memories",             0x0b043e00, 0x0B2CE400, 0x0b043e00}, // lacks JP
    {"822",    "822_riku_takes_care_of_xion",       0x0b514600, 0x0B79EC00, 0x0b514600}, // lacks JP
    {"823",    "823_roxas_passes_by",               0x0b5b5e00, 0x0B840400, 0x0b5b5e00}, // lacks JP
    {"824",    "824_xions_dream",                   0x0b65a200, 0x0B8E4800, 0x0b65a200}, // lacks JP
    {"825",    "825_xions_capture",                 0x0b8a7a00, 0x0BB32000, 0x0b8a7a00}, // lacks JP
    {"826",    "826_hollow_bastion_memories",       0x0bd74600, 0x0bd74600, 0x0bd74600}, // lacks EU, JP
    {"827",    "827_agrabah_keyhole_memory",        0x0be7e000, 0x0be7e000, 0x0be7e000}, // lacks EU, JP
    {"828",    "828_xion_and_riku",                 0x0bedf200, 0x0C169800, 0x0bedf200}, // lacks JP
    {"829",    "829_rikus_resolve",                 0x0c76a800, 0x0C9F4E00, 0x0c76a800}, // lacks JP
    {"830",    "830_mickey_and_riku_ansem",         0x0c863a00, 0x0CAEE000, 0x0c863a00}, // lacks JP
    {"831",    "831_xion_and_namine",               0x0ca47c00, 0x0CCD2200, 0x0ca47c00}, // lacks JP
    {"832",    "832_xion_and_axel_face_off",        0x0cb01c00, 0x0CD8C200, 0x0cb01c00}, // lacks JP
    {"833",    "833_xion_attacks",                  0x0cee2000, 0x0D16C600, 0x0cee2000}, // lacks JP
    {"834",    "834_winner",                        0x0d45bc00, 0x0D6E6200, 0x0d45bc00}, // lacks JP
    {"835",    "835_skyscrapper_battle",            0x0d5e0400, 0x0D86AA00, 0x0d5e0400}, // lacks JP
    {"836",    "836_roxas_and_riku",                0x0d6f9400, 0x0D983A00, 0x0d6f9400}, // lacks JP
    {"837",    "837_riku_turns_into_ansem",         0x0da1ea00, 0x0DCA9000, 0x0da1ea00}, // lacks JP
    {"838",    "838_clocktower",                    0x0e063600, 0x0e063600, 0x0e063600}, // double cutscene complement
    {"839_de", "839_riku_please_stop_him_de",       0x0e0db400, 0x0e0db400, 0x0e0db400}, // double cutscene complement
    {"839_en", "839_riku_please_stop_him_en",       0x0e0e1200, 0x0e0e1200, 0x0e0e1200}, // double cutscene complement
    {"839_es", "839_riku_please_stop_him_es",       0x0e0e6c00, 0x0e0e6c00, 0x0e0e6c00}, // double cutscene complement
    {"839_fr", "839_riku_please_stop_him_fr",       0x0e0ecc00, 0x0e0ecc00, 0x0e0ecc00}, // double cutscene complement
    {"839_it", "839_riku_please_stop_him_it",       0x0e0f1600, 0x0e0f1600, 0x0e0f1600}, // double cutscene complement
    {"840",    "840_after_the_battle",              0x0e0f5e00, 0x0E380400, 0x0e0f5e00}, // lacks JP
    {"841",    "841_xion_fading_from_clocktower",   0x0e444c00, 0x0e444c00, 0x0e444c00}, // double cutscene complement
    {"842",    "842_a_new_day",                     0x0e4bd400, 0x0E747A00, 0x0e4bd400}, // lacks JP
    {"843",    "843_the_usual_spot",                0x0e641200, 0x0E8CB800, 0x0e641200}, // lacks JP
    {"845",    "845_the_dark_margin_sora_whisper",  0x0e6fa600, 0x0e6fa600, 0x0e6fa600}, // lacks EU, JP
    {"846",    "846_axel_and_saix",                 0x0e75bc00, 0x0E9E6200, 0x0e75bc00}, // lacks JP
    {"847",    "847_roxas_leaves_the_organization", 0x0e9c2000, 0x0EC4C600, 0x0e9c2000}, // lacks JP
    {"848",    "848_xions_end",                     0x0eb91800, 0x0EE1BE00, 0x0eb91800}, // lacks JP
};

DialogueEntry Dialogues[] =
{
    {"hd006" ,  1 , 1 , 0 , 0 , false, 0x0225ABD9 , 0x4C00610074006500},
    {"hd008" ,  7 , 1 , 0 , 0 , false, 0x0229FFB0 , 0x47006F0020007400 , "hd009"},
    {"hd010" , 11 , 1 , 0 , 0 , false, 0x02280904 , 0x5900650061006800},
    {"hd018" , 13 , 1 , 0 , 0 , true , 0x021CC5D8 , 0x4927766520676F74},
    {"hd019" , 13 , 1 , 0 , 0 , true , 0x021CB8DC , 0x4D6F6E206E6F6D2E},
    {"hd020" , 14 , 1 , 0 , 0 , true , 0x021CCCDC , 0x426F6E6E65206368},
    {"hd021" , 14 , 1 , 0 , 0 , true , 0x021CB6E0 , 0x57686F6121205869 , "hd022"},
    {"hd025" , 15 , 1 , 0 , 0 , true , 0x021D186D , 0x776865726520646F},
    {"hd034" , 20 , 1 , 0 , 0 , true , 0x021CD018 , 0x4865792C20526F78 , "hd035"},
    {"hd039" , 21 , 1 , 0 , 0 , false, 0x021c3310 , 0x5900750070002E00},
    {"hd043a", 23 , 1 , 0 , 0 , true , 0x021D05DC , 0x526F786173203F00},
    {"hd043b", 23 , 1 , 0 , 0 , true , 0X021CF40C , 0X5427617572617320 , "hd044"},
    {"hd048b", 29 , 1 , 0 , 0 , false, 0x022004D0 , 0x570065006C006C00},
    {"hd050" , 30 , 1 , 0 , 0 , false, 0x0227CF50 , 0x4200750074002000 , "hd051"},
    {"hd054" , 31 , 1 , 0 , 0 , true , 0x022140F8 , 0x5468616E6B732C20 , "hd055"},
    {"hd059" , 36 , 1 , 0 , 0 , true , 0x021CD264 , 0x4865792C20646F6E},
    {"hd063" , 37 , 1 , 0 , 0 , true , 0x0229B8F0 , 0x59006F0075002700 , "hd064"},
    {"hd065" , 42 , 1 , 0 , 0 , false, 0x022AB56C , 0x4800750068003F00 , "hd066a"},
    {"hd066b", 42 , 1 , 0 , 0 , true , 0x021CE9A4 , 0x486527732077726F},
    {"hd068" , 43 , 1 , 0 , 0 , false, 0x021D3D68 , 0x4900270064002000},
    {"hd070" , 44 , 1 , 0 , 0 , false, 0x021D5E80 , 0x4F006B0061007900},
    {"hd071" , 44 , 1 , 0 , 0 , true , 0x021D04E8 , 0x506575742DC3AA74},
    {"hd076" , 51 , 1 , 0 , 0 , false, 0x0227B770 , 0x490020006A007500 , "hd077"},
    {"hd082a", 56 , 1 , 0 , 0 , true , 0x021CCCCC , 0x58696F6E20210000},
    {"hd082b", 56 , 1 , 0 , 0 , true , 0x021CD100 , 0x4927766520676F74},
    {"hd084" , 56 , 1 , 0 , 0 , true , 0x021CD190 , 0x536F72612E2E2E00 , "hd085" , 0 , 0 , "hd086a"},
    {"hd087" , 61 , 1 , 0 , 0 , false, 0x022A0B58 , 0x430027006D006F00},
    {"hd092" , 65 , 1 , 0 , 0 , true , 0x021CEDC8 , 0x5468656E2E2E2E49},
    {"hd096" , 73 , 1 , 0 , 0 , true , 0x022472BC , 0x49742773206E6F20}
};

#define SequentialCutscenesSize 3
char SequentialCutscenes[SequentialCutscenesSize][2][12] = {{"837", "840"}, {"848", "834"}, {"842", "843"}};

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
    _ignore3DOnBottomScreen = false;
    _priorIgnore3DOnBottomScreen = false;
    _priorPriorIgnore3DOnBottomScreen = false;

    _StartPressCount = 0;
    _SkipPressCount = 0;
    _StartedReplacementCutscene = false;
    _ShouldTerminateIngameCutscene = false;
    _ShouldStartReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;
    _ShouldHideScreenForTransitions = false;
    _CurrentCutscene = nullptr;

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
    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "PriorGameScene");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "GameScene");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "KHUIScale");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "TopScreenAspectRatio");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "ShowMap");
    CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "ShowTarget");
    CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "ShowMissionGauge");
    CompGpuLoc[CompShader][7] = glGetUniformLocation(CompShader, "ShowMissionInfo");
    CompGpuLoc[CompShader][8] = glGetUniformLocation(CompShader, "HideScene");
}

void PluginKingdomHeartsDays::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1i(CompGpuLoc[CompShader][0], priorGameScene);
    glUniform1i(CompGpuLoc[CompShader][1], GameScene);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1f(CompGpuLoc[CompShader][3], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][4], ShowMap ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][5], ShowTarget ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][6], ShowMissionGauge ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][7], ShowMissionInfo ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][8], _ShouldHideScreenForTransitions ? 1 : 0);
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

void PluginKingdomHeartsDays::onLoadState(melonDS::NDS* nds)
{
    u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
    u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
    nds->ARM7Write32(cutsceneAddress, 0x0);
    nds->ARM7Write32(cutsceneAddress2, 0x0);
}

u32 PluginKingdomHeartsDays::applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress)
{
    if (_StartedReplacementCutscene && (~InputMask) & (1 << 3) && (_SkipPressCount++) < 1) { // Start
        _ShouldStopReplacementCutscene = true;
    }

    if (!_StartedReplacementCutscene && (_ShouldTerminateIngameCutscene || _ShouldReturnToGameAfterCutscene) &&
        (++_StartPressCount) <= CUTSCENE_SKIP_START_FRAMES_COUNT) {
        InputMask &= ~(1<<3); // Start
    }

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
        case gameScene_RoxasThoughts: return "Game scene: Roxas Thoughts";
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

bool PluginKingdomHeartsDays::shouldRenderFrame(melonDS::NDS* nds)
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
            IsBottomScreen2DTextureBlack = isBottomScreen2DTextureBlack(nds);

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

int PluginKingdomHeartsDays::detectGameScene(melonDS::NDS* nds)
{
    //printf("Game scene: %d\n",  GameScene);

    // printf("0x02194CBF: %08x %08x\n", nds->ARM7Read32(0x02194CBF), nds->ARM7Read32(0x02194CC3));

    // Also happens during intro, during the start of the mission review, on some menu screens; those seem to use real 2D elements
    bool no3D = nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 0;

    // 3D element mimicking 2D behavior
    bool doesntLook3D = nds->GPU.GPU3D.RenderNumPolygons < 20;

    bool wasSaveLoaded = getCurrentMap(nds) != 0;
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

    if (has3DOnBothScreens)
    {
        // Needed by opening cutscene triggered by being idle
        bool isMainMenu = GameScene == gameScene_MainMenu && !wasSaveLoaded && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;
        if (isMainMenu)
        {
            return gameScene_MainMenu;
        }

        bool isMissionVictory = (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 130) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 2625) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 0   && nds->GPU.GPU2D_B.BlendCnt == 2114) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 130 && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 322 && nds->GPU.GPU2D_B.BlendCnt == 0) ||
                                (nds->GPU.GPU2D_A.BlendCnt == 840 && nds->GPU.GPU2D_B.BlendCnt == 0);
        if (isMissionVictory)
        {
            if (GameScene == gameScene_Cutscene)
            {
                return gameScene_Cutscene;
            }
            if (GameScene != gameScene_InGameWithCutscene)
            {
                return gameScene_MultiplayerMissionReview;
            }
        }

        return gameScene_InGameWithCutscene;
    }
    else if (has3DOnBottomScreen)
    {
        if (nds->GPU.GPU3D.RenderNumPolygons < 20)
        {
            if (GameScene == gameScene_InGameMenu)
            {
                return gameScene_InGameMenu;
            }

            // Opening cutscene
            if (GameScene == gameScene_MainMenu)
            {
                if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons == 1)
                {
                    return gameScene_Cutscene;
                }

                // Needed by opening cutscene triggered by being idle
                bool isMainMenu = !wasSaveLoaded && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;
                if (isMainMenu)
                {
                    return gameScene_MainMenu;
                }
            }
            if (GameScene == gameScene_Cutscene)
            {
                if (nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0 && nds->GPU.GPU3D.RenderNumPolygons >= 0 && nds->GPU.GPU3D.RenderNumPolygons <= 3)
                {
                    return gameScene_Cutscene;
                }
            }

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

        if (nds->GPU.GPU3D.RenderNumPolygons < 100)
        {
            return gameScene_InGameMenu;
        }

        // Unknown
        return gameScene_Other;
    }
    else if (!has3DOnTopScreen)
    {
        // Unknown 2D
        return gameScene_Other2D;
    }
    else if (!wasSaveLoaded)
    {
        // Intro load menu
        bool isIntroLoadMenu = (nds->GPU.GPU2D_B.BlendCnt == 4164 || nds->GPU.GPU2D_B.BlendCnt == 4161) &&
            (nds->GPU.GPU2D_A.EVA == 0 || nds->GPU.GPU2D_A.EVA == 16) &&
            nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 0 &&
            (nds->GPU.GPU2D_B.EVA < 10 && nds->GPU.GPU2D_B.EVA >= 0) && 
            (nds->GPU.GPU2D_B.EVB >  7 && nds->GPU.GPU2D_B.EVB <= 16) && nds->GPU.GPU2D_B.EVY == 0;
        bool mayBeMainMenu = !wasSaveLoaded && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 1;

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

        // Bottom cutscene
        bool isBottomCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
            nds->GPU.GPU2D_A.EVA == 16 && nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 9 &&
            nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (isBottomCutscene)
        {
            return gameScene_Cutscene;
        }

        mayBeMainMenu = !wasSaveLoaded && nds->GPU.GPU3D.NumVertices == 4 && nds->GPU.GPU3D.NumPolygons == 1 && nds->GPU.GPU3D.RenderNumPolygons == 0;
        if (mayBeMainMenu)
        {
            return gameScene_MainMenu;
        }
    }

    bool isShop = (nds->GPU.GPU3D.RenderNumPolygons == 264 && nds->GPU.GPU2D_A.BlendCnt == 0 && 
                   nds->GPU.GPU2D_B.BlendCnt == 0 && nds->GPU.GPU2D_B.BlendAlpha == 16) ||
            (GameScene == gameScene_Shop && nds->GPU.GPU3D.NumVertices == 0 && nds->GPU.GPU3D.NumPolygons == 0);
    if (isShop)
    {
        return gameScene_Shop;
    }

    if (doesntLook3D)
    {
        // Mission Mode / Story Mode - Challenges
        bool inHoloMissionMenu = nds->GPU.GPU2D_A.BlendCnt == 129 && (nds->GPU.GPU2D_B.BlendCnt >= 143 && nds->GPU.GPU2D_B.BlendCnt <= 207);
        if (inHoloMissionMenu)
        {
            return gameScene_InHoloMissionMenu;
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
        if (GameScene != gameScene_Intro)
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

        // In Game Save Menu
        bool isGameSaveMenu = nds->GPU.GPU2D_A.BlendCnt == 4164 && (nds->GPU.GPU2D_B.EVA == 0 || nds->GPU.GPU2D_B.EVA == 16) && 
             nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0 &&
            (nds->GPU.GPU2D_A.EVA < 10 && nds->GPU.GPU2D_A.EVA >= 2) && 
            (nds->GPU.GPU2D_A.EVB >  7 && nds->GPU.GPU2D_A.EVB <= 14);
        if (isGameSaveMenu) 
        {
            return gameScene_InGameSaveMenu;
        }

        // Bottom cutscene
        bool isBottomCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
             nds->GPU.GPU2D_A.EVA == 16 && nds->GPU.GPU2D_A.EVB == 0 && nds->GPU.GPU2D_A.EVY == 9 &&
             nds->GPU.GPU2D_B.EVA == 16 && nds->GPU.GPU2D_B.EVB == 0 && nds->GPU.GPU2D_B.EVY == 0;
        if (isBottomCutscene)
        {
            return gameScene_Cutscene;
        }

        // Cutscene (Day 359)
        bool isCutscene = nds->GPU.GPU2D_A.BlendCnt == 0 && 
             nds->GPU.GPU2D_A.EVA == 0 && nds->GPU.GPU2D_A.EVB == 16 && nds->GPU.GPU2D_A.EVY == 9 &&
             nds->GPU.GPU2D_B.EVA == 8 && nds->GPU.GPU2D_B.EVB == 8  && nds->GPU.GPU2D_B.EVY == 0;
        if (isCutscene)
        {
            return gameScene_Cutscene;
        }

        if (nds->GPU.GPU2D_B.MasterBrightness == 32784)
        {
            return gameScene_RoxasThoughts;
        }

        // Unknown 2D
        return gameScene_Other2D;
    }

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

void PluginKingdomHeartsDays::setAspectRatio(melonDS::NDS* nds, float aspectRatio)
{
    int aspectRatioKey = (int)round(0x1000 * aspectRatio);

    u32 aspectRatioMenuAddress = getAddressByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP, ASPECT_RATIO_ADDRESS_JP_REV1);

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

CutsceneEntry* PluginKingdomHeartsDays::detectCutscene(melonDS::NDS* nds)
{
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
        if (getAddress(entry) == cutsceneAddressValue) {
            cutscene1 = entry;
        }
        if (getAddress(entry) == cutsceneAddressValue2) {
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

CutsceneEntry* PluginKingdomHeartsDays::detectSequenceCutscene(melonDS::NDS* nds)
{
    for (int seqIndex = 0; seqIndex < SequentialCutscenesSize; seqIndex++) {
        if (strcmp(_CurrentCutscene->DsName, SequentialCutscenes[seqIndex][0]) == 0) {
            for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
                if (strcmp(entry->DsName, SequentialCutscenes[seqIndex][1]) == 0) {
                    return entry;
                }
            }
        }
    }
    return nullptr;
}

void PluginKingdomHeartsDays::refreshCutscene(melonDS::NDS* nds)
{
#if !REPLACEMENT_CUTSCENES_ENABLED
    return;
#endif

    bool isCutsceneScene = GameScene == gameScene_Cutscene;
    CutsceneEntry* cutscene = detectCutscene(nds);
    bool wasSaveLoaded = getCurrentMap(nds) != 0;

    if (cutscene != nullptr && strcmp(cutscene->DsName, "843") == 0 && wasSaveLoaded) {
        // cutscene = nullptr;
        // u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
        // u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
        // nds->ARM7Write32(cutsceneAddress,  0x0b514600);
        // nds->ARM7Write32(cutsceneAddress2, 0x0b514600);
    }
    if (cutscene != nullptr) {
        onIngameCutsceneIdentified(nds, cutscene);
    }
    if (isCutsceneScene && _ShouldTerminateIngameCutscene) {
        onTerminateIngameCutscene(nds);
    }
    if (_ShouldReturnToGameAfterCutscene && (!isCutsceneScene || (!wasSaveLoaded && _StartPressCount == CUTSCENE_SKIP_START_FRAMES_COUNT))) {
        onReturnToGameAfterCutscene(nds);
    }
}

std::string PluginKingdomHeartsDays::CutsceneFilePath(CutsceneEntry* cutscene) {
    std::string filename = "hd" + std::string(cutscene->DsName) + ".mp4";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "cutscenes" / filename;
    if (!std::filesystem::exists(fullPath)) {
        // TODO: KH try to load the cutscene from EPIC\Mare\MOVIE\Days\en
        return "";
    }
    return fullPath.string();
}

void PluginKingdomHeartsDays::onIngameCutsceneIdentified(melonDS::NDS* nds, CutsceneEntry* cutscene) {
    if (_CurrentCutscene != nullptr && _CurrentCutscene->usAddress == cutscene->usAddress) {
        return;
    }

    // Workaround so those two cutscenes are played in sequence ingame
    bool wasSaveLoaded = getCurrentMap(nds) != 0;
    if (wasSaveLoaded) {
        for (int seqIndex = 0; seqIndex < SequentialCutscenesSize; seqIndex++) {
            if (_CurrentCutscene != nullptr && strcmp(_CurrentCutscene->DsName, SequentialCutscenes[seqIndex][1]) == 0 &&
                                               strcmp(cutscene->DsName,         SequentialCutscenes[seqIndex][0]) == 0) {
                return;
            }
        }
    }

    std::string path = CutsceneFilePath(cutscene);
    if (path == "") {
        return;
    }

    printf("Detected cutscene: %s\n", cutscene->Name);
    log("Cutscene detected");

    _StartPressCount = 0;
    _SkipPressCount = 0;
    _CurrentCutscene = cutscene;
    _ShouldTerminateIngameCutscene = true;
}
void PluginKingdomHeartsDays::onTerminateIngameCutscene(melonDS::NDS* nds) {
    if (_CurrentCutscene == nullptr) {
        return;
    }
    printf("Starting cutscene: %s\n", _CurrentCutscene->Name);
    log("Starting cutscene");
    _ShouldTerminateIngameCutscene = false;
    _ShouldStartReplacementCutscene = true;
}
void PluginKingdomHeartsDays::onReplacementCutsceneStart(melonDS::NDS* nds) {
    log("Cutscene started");
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = true;
}

void PluginKingdomHeartsDays::onReplacementCutsceneEnd(melonDS::NDS* nds) {
    log("Should stop ingame cutscene");
    _StartedReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = true;

    CutsceneEntry* sequence = detectSequenceCutscene(nds);
    _ShouldHideScreenForTransitions = sequence != nullptr;
}
void PluginKingdomHeartsDays::onReturnToGameAfterCutscene(melonDS::NDS* nds) {
    log("Ingame cutscene reached its end");
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;

    // Ugly workaround to play one cutscene after another one, because both are skipped with a single "Start" click
    bool newCutsceneWillPlay = false;
    bool wasSaveLoaded = getCurrentMap(nds) != 0;
    if (wasSaveLoaded) {
        CutsceneEntry* sequence = detectSequenceCutscene(nds);
        if (sequence != nullptr) {
            onIngameCutsceneIdentified(nds, sequence);
            onTerminateIngameCutscene(nds);
            newCutsceneWillPlay = true;
        }
    }

    if (!newCutsceneWillPlay) {
        _CurrentCutscene = nullptr;

        u32 cutsceneAddress = getAddressByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
        u32 cutsceneAddress2 = getAddressByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
        nds->ARM7Write32(cutsceneAddress, 0x0);
        nds->ARM7Write32(cutsceneAddress2, 0x0);
    }
}

bool PluginKingdomHeartsDays::refreshGameScene(melonDS::NDS* nds)
{
    int newGameScene = detectGameScene(nds);
    
    debugLogs(nds, newGameScene);

    bool updated = setGameScene(nds, newGameScene);

    refreshCutscene(nds);

    return updated;
}

u32 PluginKingdomHeartsDays::getCurrentMission(melonDS::NDS* nds)
{
    return nds->ARM7Read8(getAddressByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
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
u32 PluginKingdomHeartsDays::getCurrentMap(melonDS::NDS* nds)
{
    u8 world = nds->ARM7Read8(getAddressByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    u8 map = nds->ARM7Read8(getAddressByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
    u32 fullMap = world;
    fullMap = (fullMap << 4*2) | map;
    return fullMap;
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

#define PRINT_AS_32_BIT_HEX(ADDRESS) printf("0x%08x: 0x%08x\n", ADDRESS, nds->ARM7Read32(ADDRESS))
#define PRINT_AS_32_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read32(ADDRESS)))

void PluginKingdomHeartsDays::debugLogs(melonDS::NDS* nds, int gameScene)
{
    if (!DEBUG_MODE_ENABLED) {
        return;
    }

    printf("Game scene: %d\n",  gameScene);
    printf("Current map: %d\n", getCurrentMap(nds));
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