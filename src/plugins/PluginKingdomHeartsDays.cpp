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
#define ASPECT_RATIO_ADDRESS_JP_REV1 0x02023C6C

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

// 0x10 => credits
#define IS_CREDITS_US      0x020446c2
#define IS_CREDITS_EU      0x020446e2 // TODO: KH Unconfirmed (calculated)
#define IS_CREDITS_JP      0x02044b22 // TODO: KH Unconfirmed (calculated)
#define IS_CREDITS_JP_REV1 0x02044ae2 // TODO: KH Unconfirmed (calculated)

// 0x80 => playable (example: ingame); 0x04 => not playable (menus and credits)
#define IS_PLAYABLE_AREA_US      0x020446c6
#define IS_PLAYABLE_AREA_EU      0x020446e6
#define IS_PLAYABLE_AREA_JP      0x02044b26
#define IS_PLAYABLE_AREA_JP_REV1 0x02044ae6

#define PAUSE_SCREEN_ADDRESS_US      0x0204bd64
#define PAUSE_SCREEN_ADDRESS_EU      0x0204bd84
#define PAUSE_SCREEN_ADDRESS_JP      0x0204c1c4
#define PAUSE_SCREEN_ADDRESS_JP_REV1 0x0204c184

#define PAUSE_SCREEN_VALUE_NONE       0x00
#define PAUSE_SCREEN_VALUE_TRUE_PAUSE 0x01
#define PAUSE_SCREEN_VALUE_FAKE_PAUSE 0x02 // mission mode; game isn't paused, but pause view is shown

#define DEATH_SCREEN_ADDRESS_US      0x0204bd84
#define DEATH_SCREEN_ADDRESS_EU      0x0204bda4
#define DEATH_SCREEN_ADDRESS_JP      0x0204c56e
#define DEATH_SCREEN_ADDRESS_JP_REV1 0x0204c60c

#define DEATH_SCREEN_VALUE_US      0x80
#define DEATH_SCREEN_VALUE_EU      0x80
#define DEATH_SCREEN_VALUE_JP      0x00
#define DEATH_SCREEN_VALUE_JP_REV1 0x00

// 0x60 => The End
#define THE_END_SCREEN_ADDRESS_US      0x0204becd
#define THE_END_SCREEN_ADDRESS_EU      0x0204beed // TODO: KH Unconfirmed (calculated)
#define THE_END_SCREEN_ADDRESS_JP      0x0204c32d // TODO: KH Unconfirmed (calculated)
#define THE_END_SCREEN_ADDRESS_JP_REV1 0x0204c2ed // TODO: KH Unconfirmed (calculated)

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

#define TUTORIAL_ADDRESS_US      0x0207f9dc
#define TUTORIAL_ADDRESS_EU      0x0207f9fc
#define TUTORIAL_ADDRESS_JP      0x0207f8dc
#define TUTORIAL_ADDRESS_JP_REV1 0x0207f89c

#define CURRENT_MAP_FROM_WORLD_US      0x02188EE6
#define CURRENT_MAP_FROM_WORLD_EU      0x02189CC6
#define CURRENT_MAP_FROM_WORLD_JP      0x02187FC6
#define CURRENT_MAP_FROM_WORLD_JP_REV1 0x02188046

#define SONG_ADDRESS_US      0x02191D5E
#define SONG_ADDRESS_EU      0x02192B3E
#define SONG_ADDRESS_JP      0x02191D5E // TODO: KH
#define SONG_ADDRESS_JP_REV1 0x02191D5E // TODO: KH

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US      0x02194CC3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU      0x02195AA3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP      0x02193E23
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1 0x02193DA3

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
    gameScene_InGameWithDouble3D,       // 9
    gameScene_MultiplayerMissionReview, // 10
    gameScene_Shop,                     // 11
    gameScene_LoadingScreen,            // 12
    gameScene_RoxasThoughts,            // 13
    gameScene_DeathScreen,              // 14
    gameScene_TheEnd,                   // 15
    gameScene_Other                     // 16
};

PluginKingdomHeartsDays::PluginKingdomHeartsDays(u32 gameCode)
{
    GameCode = gameCode;

    PriorGameScene = -1;
    GameScene = -1;
    HUDState = -1;
    hudToggle();

    initCutsceneVariables();
    initBgmVariables();

    priorMap = -1;
    Map = 0;
    UIScale = 4;
    AspectRatio = 0;

    // game scene detection utils
    _muchOlderHad3DOnTopScreen = false;
    _muchOlderHad3DOnBottomScreen = false;
    _olderHad3DOnTopScreen = false;
    _olderHad3DOnBottomScreen = false;
    _had3DOnTopScreen = false;
    _had3DOnBottomScreen = false;

    // should render frame utils
    _hasVisible3DOnBottomScreen = false;
    _ignore3DOnBottomScreen = false;
    _priorIgnore3DOnBottomScreen = false;
    _priorPriorIgnore3DOnBottomScreen = false;

    // apply hotkey to input mask utils
    PriorHotkeyMask = 0;
    PriorPriorHotkeyMask = 0;
    LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;
    SwitchTargetPressOnHold = false;

    Cutscenes = std::array<Plugins::CutsceneEntry, 46> {{
        {"802",    "802_mm", "802_opening",                       0x088b2e00, 0x08b3d400, 0x0876e800, 0},
        {"803",    "803",    "803_meet_xion",                     0x0987ec00, 0x09b09200, 0x097c8800, 0},
        {"804",    "804",    "804_roxas_recusant_sigil",          0x09ae9400, 0x09d73a00, 0x09a4e000, 0},
        {"805",    "805",    "805_the_dark_margin",               0x09b80600, 0x09e0ac00, 0x09aeb200, 0},
        {"806",    "806",    "806_sora_entering_pod",             0x09e83800, 0x0a10de00, 0x09e0ba00, 0},
        {"808",    "808",    "808_sunset_memory",                 0x09f24c00, 0x0a1af200, 0x09eb2e00, 0},
        {"809",    "809",    "809_xions_defeat",                  0x09f79400, 0x0a203a00, 0x09f0ac00, 0},
        {"810",    "810",    "810_the_main_in_black_reflects",    0x09ff8000, 0x0a282600, 0x09f8d400, 0},
        {"813",    "813",    "813_xions_defeat",                  0x0a13f600, 0x0a3c9c00, 0x0a0e1600, 0},
        {"814",    "814",    "814_sora_walk",                     0x0a677c00, 0x0a902200, 0x0a64f000, 0},
        {"815",    "815",    "815_sora_release_kairi",            0x0a6e4200, 0x0a96e800, 0x0a6bf000, 0},
        {"816",    "816",    "816_kairi_memories",                0x0a7a9200, 0x0aa33800, 0x0a78ca00, 0},
        {"817",    "817_mm", "817_namine_and_diz",                0x0a857600, 0x0aae1c00, 0x0a845800, 0},
        {"818",    "818",    "818_why_the_sun_sets_red",          0x0ab4be00, 0x0add6400, 0x0ab57800, 0},
        {"819",    "819",    "819_sora_wakes_up",                 0x0afeac00, 0x0afeac00, 0x0afeac00, 0}, // double cutscene complement
        {"821",    "821",    "821_snarl_of_memories",             0x0b043e00, 0x0b2ce400, 0x0b084a00, 0},
        {"822",    "822",    "822_riku_takes_care_of_xion",       0x0b514600, 0x0b79ec00, 0x0b57ea00, 0},
        {"823",    "823",    "823_roxas_passes_by",               0x0b5b5e00, 0x0b840400, 0x0b626e00, 0},
        {"824",    "824",    "824_xions_dream",                   0x0b65a200, 0x0b8e4800, 0x0b6d1800, 0},
        {"825",    "825",    "825_xions_capture",                 0x0b8a7a00, 0x0bb32000, 0x0b937600, 0},
        {"826",    "826",    "826_hollow_bastion_memories",       0x0bd74600, 0x0bffec00, 0x0be21a00, 0},
        {"827",    "827",    "827_agrabah_keyhole_memory",        0x0be7e000, 0x0c108600, 0x0bf35400, 0},
        {"828",    "828",    "828_xion_and_riku",                 0x0bedf200, 0x0c169800, 0x0bf9ae00, 0},
        {"829",    "829",    "829_rikus_resolve",                 0x0c76a800, 0x0c9f4e00, 0x0c873400, 0},
        {"830",    "830_mm", "830_mickey_and_riku_ansem",         0x0c863a00, 0x0caee000, 0x0c981000, 0},
        {"831",    "831",    "831_xion_and_namine",               0x0ca47c00, 0x0ccd2200, 0x0cb79400, 0},
        {"832",    "832",    "832_xion_and_axel_face_off",        0x0cb01c00, 0x0cd8c200, 0x0cc3b000, 0},
        {"833",    "833_mm", "833_xion_attacks",                  0x0cee2000, 0x0d16c600, 0x0d043200, 0},
        {"834",    "834",    "834_winner",                        0x0d45bc00, 0x0d6e6200, 0x0d5f7800, 0},
        {"835",    "835",    "835_skyscrapper_battle",            0x0d5e0400, 0x0d86aa00, 0x0d782e00, 0},
        {"836",    "836_mm", "836_roxas_and_riku",                0x0d6f9400, 0x0d983a00, 0x0d8a7e00, 0},
        {"837",    "837",    "837_riku_turns_into_ansem",         0x0da1ea00, 0x0dca9000, 0x0dbed000, 0},
        {"838",    "838",    "838_clocktower",                    0x0e063600, 0x0e063600, 0x0e063600, 0}, // double cutscene complement
        {"839_de", "839",    "839_riku_please_stop_him_de",       0x0e0db400, 0x0e0db400, 0x0e0db400, 0}, // double cutscene complement
        {"839_en", "839",    "839_riku_please_stop_him_en",       0x0e0e1200, 0x0e0e1200, 0x0e0e1200, 0}, // double cutscene complement
        {"839_es", "839",    "839_riku_please_stop_him_es",       0x0e0e6c00, 0x0e0e6c00, 0x0e0e6c00, 0}, // double cutscene complement
        {"839_fr", "839",    "839_riku_please_stop_him_fr",       0x0e0ecc00, 0x0e0ecc00, 0x0e0ecc00, 0}, // double cutscene complement
        {"839_it", "839",    "839_riku_please_stop_him_it",       0x0e0f1600, 0x0e0f1600, 0x0e0f1600, 0}, // double cutscene complement
        {"840",    "840",    "840_after_the_battle",              0x0e0f5e00, 0x0e380400, 0x0e2e4200, 0},
        {"841",    "841",    "841_xion_fading_from_clocktower",   0x0e444c00, 0x0e444c00, 0x0e444c00, 0}, // double cutscene complement
        {"842",    "842",    "842_a_new_day",                     0x0e4bd400, 0x0e747a00, 0x0e6dfa00, 0},
        {"843",    "843_mm", "843_the_usual_spot",                0x0e641200, 0x0e8cb800, 0x0e873800, 0},
        {"845",    "845",    "845_the_dark_margin_sora_whisper",  0x0e6fa600, 0x0e984c00, 0x0e938e00, 0},
        {"846",    "846",    "846_axel_and_saix",                 0x0e75bc00, 0x0e9e6200, 0x0e99ee00, 0},
        {"847",    "847",    "847_roxas_leaves_the_organization", 0x0e9c2000, 0x0ec4c600, 0x0ec12c00, 0},
        {"848",    "848",    "848_xions_end",                     0x0eb91800, 0x0ee1be00, 0x0edf4600, 0},
    }};
}

void PluginKingdomHeartsDays::loadLocalization() {
    u8* rom = (u8*)nds->GetNDSCart()->GetROM();

    std::string language = KHDaysLanguage;
    if (language == "") {
        language = "en-US";
    }

    std::string localizationFilePath = LocalizationFilePath("en-US");
    Platform::FileHandle* f = Platform::OpenLocalFile(localizationFilePath.c_str(), Platform::FileMode::ReadText);
    if (f) {
        char linebuf[1024];
        char entryname[32];
        char entryval[1024];
        while (!Platform::IsEndOfFile(f))
        {
            if (!Platform::FileReadLine(linebuf, 1024, f))
                break;

            int ret = sscanf(linebuf, "%31[A-Za-z_0-9]=%[^\t\r\n]", entryname, entryval);
            entryname[31] = '\0';
            if (ret < 2) continue;

            std::string entrynameStr = std::string(entryname);
            if (entrynameStr.compare(0, 2, "0x") == 0) {
                int addrGap = 0;
                unsigned int addr = std::stoul(entrynameStr.substr(2), nullptr, 16);

                bool ended = false;
                for (int i = 0; i < 1023; i++) {
                    if (*((u8*)&rom[addr + i]) == 0x00) {
                        break;
                    }

                    if (entryval[i + addrGap] == '\\' && entryval[i + addrGap + 1] == 'n') {
                        *((u8*)&rom[addr + i]) = 0x0A;
                        addrGap++;
                        continue;
                    }

                    if (entryval[i + addrGap] == 0 || entryval[i + addrGap] == '\0') {
                        ended = true;
                    }

                    if (ended) {
                        *((u8*)&rom[addr + i]) = 0x20;
                    }
                    else {
                        *((u8*)&rom[addr + i]) = entryval[i + addrGap];
                    }
                }
            }
        }

        CloseFile(f);
    }
    else if (false) { // Method that exports the strings from the game ROM
        int firstAddr = 0;
        int lastAddr = 0;
        bool validCharFound = false;
        bool forbCharFound = false;
        for (int addr = 0x0689D330; addr < 0x06C49D0C; addr++) {
            bool usual = rom[addr] >= 0x41 && rom[addr] <= 0x7E;
            bool accents = rom[addr] == 0xC2 || rom[addr] == 0xC3 || (rom[addr] >= 0x80 && rom[addr] <= 0xBF);
            bool quotes = rom[addr] == 0xE2 || rom[addr] == 0x80 || rom[addr] == 0x9C || rom[addr] == 0x9D;
            bool unusual = (rom[addr] >= 0x20 && rom[addr] <= 0x40) || accents || quotes || rom[addr] == 0x0A;
            bool forb = rom[addr] == 0x93 || rom[addr] == 0x5F || rom[addr] == 0x2F;
            if (usual || unusual) {
                if (firstAddr == 0) {
                    firstAddr = addr;
                    lastAddr = addr;
                }
                else {
                    lastAddr = addr;
                }
            }
            if (usual) {
                validCharFound = true;
            }
            if (forb) {
                forbCharFound = true;
            }
            if (!usual && !unusual) {
                if (firstAddr != 0) {
                    if (!forbCharFound && validCharFound && lastAddr - firstAddr > 2) {
                        printf("0x%08X=", firstAddr);
                        for (int pAddr = firstAddr; pAddr <= lastAddr; pAddr++) {
                            if ((char)rom[pAddr] == 0x0A) {
                                printf("\\n");
                            }
                            else {
                                printf("%c", (char)rom[pAddr]);
                            }
                        }
                        printf("\n");
                    }

                    firstAddr = 0;
                    lastAddr = 0;
                    validCharFound = false;
                    forbCharFound = false;
                }
            }
        }
    }
}

void PluginKingdomHeartsDays::onLoadROM() {
    loadLocalization();

    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
}

std::string PluginKingdomHeartsDays::assetsFolder() {
    return "days";
}

std::string PluginKingdomHeartsDays::assetsRegionSubfolder() {
    return getStringByCart("us", "eu", "jp", "jp_rev1");
}

const char* PluginKingdomHeartsDays::gpuOpenGL_FS() {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kCompositorFS_KhDays;
};

const char* PluginKingdomHeartsDays::gpu3DOpenGLClassic_VS_Z() {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kRenderVS_Z_KhDays;
};

void PluginKingdomHeartsDays::gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32* x, s32* y, s32 z, s32* rgb) {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return;
    }

    float aspectRatio = AspectRatio / (4.f / 3.f);
    float iuTexScale = (5.0)/UIScale;

    float _x = (float)(*x - ScreenWidth/2);
    float _y = (float)(*y - ScreenHeight/2);
    float _z = ((float)z)/(1 << 22);
    if (HideAllHUD)
    {
        if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_PauseMenu || GameScene == gameScene_InGameWithDouble3D)
        {
            if (_x >= -(1.00)*(ScreenWidth/2)  && _x <= +(1.00)*(ScreenWidth/2) &&
                _y >= -(1.00)*(ScreenHeight/2) && _y <= +(1.00)*(ScreenHeight/2) &&
                _z < (s32)(-(0.0007)) && _z >= (s32)(-(1.000))) {
                _x = (0 - 1.0)*(ScreenWidth/2);
                _y = (0 - 1.0)*(ScreenHeight/2);
            }
        }
    }
    else
    {
        if (GameScene == gameScene_InGameWithMap)
        {
            float effectLayer = -0.0007; // blue shine behind the heart counter and "CHAIN" label
            float textLayer = -0.0009; // heart counter, timer, "BONUS" label and +X floating labels

            float heartTopMargin = (ShowMissionInfo ? 20.0 : 2.0);
            float heartWidth = (ScreenWidth*9)/20.0;
            float heartHeight = ScreenHeight/2.5;
            if ((_x >= -(1.000)*(ScreenWidth/2)  && _x <= -(0.000)*(ScreenWidth/2) &&
                 _y >= -(1.000)*(ScreenHeight/2) && _y <= -(0.200)*(ScreenHeight/2) &&
                (abs(_z - effectLayer) < 0.0001))
                ||
                (_x >= -(1.000)*(ScreenWidth/2) && _x <= -(0.200)*(ScreenWidth/2) &&
                 _y >= -(1.000)*(ScreenHeight/2) && _y <= -(0.500)*(ScreenHeight/2) &&
                (abs(_z - textLayer) < 0.0001))) {
                _x = ((((_x/(ScreenWidth/2)  + 1.0)*(heartWidth/iuTexScale))/ScreenWidth)*2.0/aspectRatio - 1.0)*(ScreenWidth/2);
                _y = ((((_y/(ScreenHeight/2) + 1.0)*(heartHeight/iuTexScale) + heartTopMargin/iuTexScale)/ScreenHeight)*2.0 - 1.0)*(ScreenHeight/2);
            }
        }

        if (GameScene == gameScene_PauseMenu)
        {
            if (_x >= -(1.00)*(ScreenWidth/2)  && _x <= -(0.000)*(ScreenWidth/2) &&
                _y >= -(1.00)*(ScreenHeight/2) && _y <= -(0.500)*(ScreenHeight/2) &&
                _z < (s32)(-(0.0007)) && _z >= (s32)(-(1.000))) {
                _x = (0 - 1.0)*(ScreenWidth/2);
                _y = (0 - 1.0)*(ScreenHeight/2);
            }
        }
    }

    *x = (s32)(_x + ScreenWidth/2);
    *y = (s32)(_y + ScreenHeight/2);
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
    CompGpuLoc[CompShader][8] = glGetUniformLocation(CompShader, "HideAllHUD");
    CompGpuLoc[CompShader][9] = glGetUniformLocation(CompShader, "HideScene");
    CompGpuLoc[CompShader][10] = glGetUniformLocation(CompShader, "MainMenuView");
    CompGpuLoc[CompShader][11] = glGetUniformLocation(CompShader, "DSCutsceneState");
}

void PluginKingdomHeartsDays::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    CutsceneEntry* tsCutscene = detectTopScreenCutscene();
    CutsceneEntry* bsCutscene = detectBottomScreenCutscene();
    int dsCutsceneState = (tsCutscene == nullptr ? 0 : 2) + (bsCutscene == nullptr ? 0 : 1);

    bool isCredits = nds->ARM7Read8(getU32ByCart(IS_CREDITS_US, IS_CREDITS_EU, IS_CREDITS_JP, IS_CREDITS_JP_REV1)) == 0x10;
    if (isCredits) {
        dsCutsceneState = 3;
    }

    glUniform1i(CompGpuLoc[CompShader][0], PriorGameScene);
    glUniform1i(CompGpuLoc[CompShader][1], GameScene);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1f(CompGpuLoc[CompShader][3], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][4], ShowMap ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][5], ShowTarget ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][6], ShowMissionGauge ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][7], ShowMissionInfo ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][8], HideAllHUD ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][9], _ShouldHideScreenForTransitions ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][10], getCurrentMainMenuView());
    glUniform1i(CompGpuLoc[CompShader][11], dsCutsceneState);
}

void PluginKingdomHeartsDays::gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags)
{
    CompGpu3DLoc[flags][0] = glGetUniformLocation(prog, "TopScreenAspectRatio");
    CompGpu3DLoc[flags][1] = glGetUniformLocation(prog, "GameScene");
    CompGpu3DLoc[flags][2] = glGetUniformLocation(prog, "KHUIScale");
    CompGpu3DLoc[flags][3] = glGetUniformLocation(prog, "ShowMissionInfo");
    CompGpu3DLoc[flags][4] = glGetUniformLocation(prog, "HideAllHUD");
}

void PluginKingdomHeartsDays::gpu3DOpenGL_VS_Z_updateVariables(u32 flags)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1f(CompGpu3DLoc[flags][0], aspectRatio);
    glUniform1i(CompGpu3DLoc[flags][1], GameScene);
    glUniform1i(CompGpu3DLoc[flags][2], UIScale);
    glUniform1i(CompGpu3DLoc[flags][3], ShowMissionInfo ? 1 : 0);
    glUniform1i(CompGpu3DLoc[flags][4], HideAllHUD ? 1 : 0);
}

void PluginKingdomHeartsDays::onLoadState()
{
    loadLocalization();

    GameScene = gameScene_InGameWithMap;

    _CurrentBackgroundMusic = 0x101;
}

void PluginKingdomHeartsDays::applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress)
{
    bool shouldContinue = _superApplyHotkeyToInputMask(InputMask, HotkeyMask, HotkeyPress);
    if (!shouldContinue) {
        return;
    }

    if (GameScene == -1) {
        return;
    }

    if (GameScene == gameScene_LoadingScreen) {
        *HotkeyMask |= (1<<4); // Fast Forward (skip loading screen)
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithDouble3D) {
        // Enabling X + D-Pad
        if ((*HotkeyMask) & ((1 << 22) | (1 << 23) | (1 << 24) | (1 << 25))) { // D-pad (HK_CommandMenuLeft, HK_CommandMenuRight, HK_CommandMenuUp, HK_CommandMenuDown)
            u32 dpadMenuAddress = getU32ByCart(INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1);

            if (nds->ARM7Read8(dpadMenuAddress) & 0x02) {
                nds->ARM7Write8(dpadMenuAddress, nds->ARM7Read8(dpadMenuAddress) - 0x02);
            }
        }

        // So the arrow keys can be used to control the command menu
        if ((*HotkeyMask) & ((1 << 22) | (1 << 23) | (1 << 24) | (1 << 25))) { //  (HK_CommandMenuLeft, HK_CommandMenuRight, HK_CommandMenuUp, HK_CommandMenuDown)
            *InputMask &= ~(1<<10); // X
            *InputMask |= (1<<5); // left
            *InputMask |= (1<<4); // right
            *InputMask |= (1<<6); // up
            *InputMask |= (1<<7); // down
            if (PriorPriorHotkeyMask & (1 << 22)) // Old D-pad left (HK_CommandMenuLeft)
                *InputMask &= ~(1<<5); // left
            if (PriorPriorHotkeyMask & (1 << 23)) // Old D-pad right (HK_CommandMenuRight)
                *InputMask &= ~(1<<4); // right
            if (PriorPriorHotkeyMask & (1 << 24)) // Old D-pad up (HK_CommandMenuUp)
                *InputMask &= ~(1<<6); // up
            if (PriorPriorHotkeyMask & (1 << 25)) // Old D-pad down (HK_CommandMenuDown)
                *InputMask &= ~(1<<7); // down
        }

        // R / Lock On
        {
            if ((*HotkeyMask) & (1 << 19)) { // (HK_RLockOn)
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
            if ((*HotkeyMask) & (1 << 20) || (*HotkeyMask) & (1 << 21)) { // (HK_LSwitchTarget, HK_RSwitchTarget)
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
        if ((*HotkeyMask) & (1 << 22)) { // D-pad left (HK_CommandMenuLeft)
            *InputMask &= ~(1<<5); // left
        }
        if ((*HotkeyMask) & (1 << 23)) { // D-pad right (HK_CommandMenuRight)
            *InputMask &= ~(1<<4); // right
        }
        if ((*HotkeyMask) & (1 << 24)) { // D-pad up (HK_CommandMenuUp)
            *InputMask &= ~(1<<6); // up
        }
        if ((*HotkeyMask) & (1 << 25)) { // D-pad down (HK_CommandMenuDown)
            *InputMask &= ~(1<<7); // down
        }

        if ((*HotkeyMask) & (1 << 19)) { // R / Lock On (HK_RLockOn)
            *InputMask &= ~(1<<8); // R
        }
    }

    PriorPriorHotkeyMask = PriorHotkeyMask;
    PriorHotkeyMask = (*HotkeyMask);

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;
}

bool PluginKingdomHeartsDays::applyTouchKeyMask(u32 TouchKeyMask)
{
    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithDouble3D) {
        nds->SetTouchKeyMask(TouchKeyMask, false);
        return true;
    }

    return false;
}

void PluginKingdomHeartsDays::hudToggle()
{
    HUDState = (HUDState + 1) % 4;
    if (HUDState == 0) { // exploration mode
        ShowMap = true;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = false;
        HideAllHUD = false;
    }
    else if (HUDState == 1) { // mission mode
        ShowMap = false;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = true;
        HideAllHUD = false;
    }
    else if (HUDState == 2) { // mission details mode
        ShowMap = false;
        ShowTarget = true;
        ShowMissionGauge = true;
        ShowMissionInfo = false;
        HideAllHUD = false;
    }
    else { // zero hud
        ShowMap = false;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = false;
        HideAllHUD = true;
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
        case gameScene_InGameWithDouble3D: return "Game scene: Ingame (with cutscene)";
        case gameScene_MultiplayerMissionReview: return "Game scene: Multiplayer Mission Review";
        case gameScene_Shop: return "Game scene: Shop";
        case gameScene_LoadingScreen: return "Game scene: Loading screen";
        case gameScene_RoxasThoughts: return "Game scene: Roxas Thoughts";
        case gameScene_DeathScreen: return "Game scene: Death screen";
        case gameScene_TheEnd: return "Game scene: The End";
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
    if (!_superShouldRenderFrame())
    {
        return false;
    }
    if (GameScene == gameScene_InGameWithDouble3D)
    {
        u32 currentMap = getCurrentMap();
        bool alternateSecondScreenDetection = (currentMap == 346);

        if (nds->PowerControl9 >> 15 != 0) // 3D on top screen
        {
            _hasVisible3DOnBottomScreen = alternateSecondScreenDetection ? true : !IsBottomScreen2DTextureBlack;

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

            if (!alternateSecondScreenDetection && _hasVisible3DOnBottomScreen) {
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

    u8 mainMenuOrIntroOrLoadMenuVal = nds->ARM7Read8(getU32ByCart(IS_MAIN_MENU_US, IS_MAIN_MENU_EU, IS_MAIN_MENU_JP, IS_MAIN_MENU_JP_REV1));
    bool isMainMenuOrIntroOrLoadMenu = mainMenuOrIntroOrLoadMenuVal == 0x28 || mainMenuOrIntroOrLoadMenuVal == 0x2C;
    bool isCutscene = nds->ARM7Read8(getU32ByCart(IS_CUTSCENE_US, IS_CUTSCENE_EU, IS_CUTSCENE_JP, IS_CUTSCENE_JP_REV1)) == 0x03;
    bool isCredits = nds->ARM7Read8(getU32ByCart(IS_CREDITS_US, IS_CREDITS_EU, IS_CREDITS_JP, IS_CREDITS_JP_REV1)) == 0x10;
    bool isUnplayableArea = nds->ARM7Read8(getU32ByCart(IS_PLAYABLE_AREA_US, IS_PLAYABLE_AREA_EU, IS_PLAYABLE_AREA_JP, IS_PLAYABLE_AREA_JP_REV1)) == 0x04;
    bool isLoadMenu = nds->ARM7Read8(getU32ByCart(CURRENT_MAIN_MENU_VIEW_US, CURRENT_MAIN_MENU_VIEW_EU, CURRENT_MAIN_MENU_VIEW_JP, CURRENT_MAIN_MENU_VIEW_JP_REV1)) ==
        getU32ByCart(LOAD_MENU_MAIN_MENU_VIEW_US, LOAD_MENU_MAIN_MENU_VIEW_EU, LOAD_MENU_MAIN_MENU_VIEW_JP, LOAD_MENU_MAIN_MENU_VIEW_JP_REV1);
    bool isDaysCounter = nds->ARM7Read8(getU32ByCart(IS_DAYS_COUNTER_US, IS_DAYS_COUNTER_EU, IS_DAYS_COUNTER_JP, IS_DAYS_COUNTER_JP_REV1)) ==
        getU32ByCart(IS_DAYS_COUNTER_VALUE_US, IS_DAYS_COUNTER_VALUE_EU, IS_DAYS_COUNTER_VALUE_JP, IS_DAYS_COUNTER_VALUE_JP_REV1);
    bool isDeathCounter = nds->ARM7Read8(getU32ByCart(DEATH_SCREEN_ADDRESS_US, DEATH_SCREEN_ADDRESS_EU, DEATH_SCREEN_ADDRESS_JP, DEATH_SCREEN_ADDRESS_JP_REV1)) ==
        getU32ByCart(DEATH_SCREEN_VALUE_US, DEATH_SCREEN_VALUE_EU, DEATH_SCREEN_VALUE_JP, DEATH_SCREEN_VALUE_JP_REV1);
    bool isTutorial = nds->ARM7Read32(getU32ByCart(TUTORIAL_ADDRESS_US, TUTORIAL_ADDRESS_EU, TUTORIAL_ADDRESS_JP, TUTORIAL_ADDRESS_JP_REV1)) != 0;
    bool isPauseScreen = nds->ARM7Read8(getU32ByCart(PAUSE_SCREEN_ADDRESS_US, PAUSE_SCREEN_ADDRESS_EU, PAUSE_SCREEN_ADDRESS_JP, PAUSE_SCREEN_ADDRESS_JP_REV1)) != 0;
    bool isTheEnd = nds->ARM7Read8(getU32ByCart(THE_END_SCREEN_ADDRESS_US, THE_END_SCREEN_ADDRESS_EU, THE_END_SCREEN_ADDRESS_JP, THE_END_SCREEN_ADDRESS_JP_REV1)) == 0x60;

    if (isCredits)
    {
        if (isTheEnd)
        {
            return gameScene_TheEnd;
        }
        else
        {
            return gameScene_Cutscene;
        }
    }

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
    }
    if (!wasSaveLoaded && (GameScene == -1 || GameScene == gameScene_Intro))
    {
        return gameScene_Intro;
    }

    if (has3DOnBothScreens)
    {
        if (nds->GPU.GPU3D.RenderNumPolygons < 20)
        {
            return GameScene;
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
            if (GameScene != gameScene_InGameWithDouble3D)
            {
                return gameScene_MultiplayerMissionReview;
            }
        }

        return gameScene_InGameWithDouble3D;
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

    if (!isUnplayableArea && isDeathCounter)
    {
        return gameScene_DeathScreen;
    }

    if (isPauseScreen)
    {
        return gameScene_PauseMenu;
    }
    else if (GameScene == gameScene_PauseMenu)
    {
        return PriorGameScene;
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

        if (nds->GPU.GPU2D_B.MasterBrightness == 32784) // TODO: Replace with memory detection
        {
            return gameScene_RoxasThoughts;
        }
    }

    if (isTutorial)
    {
        return gameScene_Tutorial;
    }

    // Regular gameplay with a map
    return gameScene_InGameWithMap;
}

void PluginKingdomHeartsDays::setAspectRatio(float aspectRatio)
{
    if (GameScene != -1)
    {
        int aspectRatioKey = (int)round(0x1000 * aspectRatio);

        u32 aspectRatioMenuAddress = getU32ByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP, ASPECT_RATIO_ADDRESS_JP_REV1);

        if (nds->ARM7Read32(aspectRatioMenuAddress) == 0x00001555) {
            nds->ARM7Write32(aspectRatioMenuAddress, aspectRatioKey);
        }
    }

    AspectRatio = aspectRatio;
}

u32 PluginKingdomHeartsDays::getCutsceneAddress(CutsceneEntry* entry)
{
    return getU32ByCart(entry->usAddress, entry->euAddress, entry->jpAddress, entry->jpAddress - 0x200);
}

u32 PluginKingdomHeartsDays::getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address)
{
    u32 value = 0;
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCartRev1()) {
        value = jpRev1Address;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

std::string PluginKingdomHeartsDays::getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress, std::string jpRev1Address)
{
    std::string value = "";
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCartRev1()) {
        value = jpRev1Address;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

bool PluginKingdomHeartsDays::getBoolByCart(bool usAddress, bool euAddress, bool jpAddress, bool jpRev1Address)
{
    bool value = false;
    if (isUsaCart()) {
        value = usAddress;
    }
    else if (isEuropeCart()) {
        value = euAddress;
    }
    else if (isJapanCartRev1()) {
        value = jpRev1Address;
    }
    else if (isJapanCart()) {
        value = jpAddress;
    }
    return value;
}

u32 PluginKingdomHeartsDays::detectTopScreenCutsceneAddress()
{
    return getU32ByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
}

u32 PluginKingdomHeartsDays::detectBottomScreenCutsceneAddress()
{
    return getU32ByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
}

CutsceneEntry* PluginKingdomHeartsDays::detectTopScreenCutscene()
{
    if (GameScene == -1)
    {
        return nullptr;
    }

    u32 cutsceneAddressValue = 0;
    u32 cutsceneAddress = detectTopScreenCutsceneAddress();
    if (cutsceneAddress != 0) {
        cutsceneAddressValue = nds->ARM7Read32(cutsceneAddress);
        if (cutsceneAddressValue == 0 || (cutsceneAddressValue - (cutsceneAddressValue & 0xFF)) == 0xea000000) {
            cutsceneAddressValue = 0;
        }
    }

    CutsceneEntry* cutscene1 = nullptr;
    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getCutsceneAddress(entry) == cutsceneAddressValue) {
            cutscene1 = entry;
        }
    }

    return cutscene1;
}

CutsceneEntry* PluginKingdomHeartsDays::detectBottomScreenCutscene()
{
    if (GameScene == -1)
    {
        return nullptr;
    }

    u32 cutsceneAddressValue2 = 0;
    u32 cutsceneAddress2 = detectBottomScreenCutsceneAddress();
    if (cutsceneAddress2 != 0) {
        cutsceneAddressValue2 = nds->ARM7Read32(cutsceneAddress2);
        if (cutsceneAddressValue2 == 0 || (cutsceneAddressValue2 - (cutsceneAddressValue2 & 0xFF)) == 0xea000000) {
            cutsceneAddressValue2 = 0;
        }
    }

    CutsceneEntry* cutscene2 = nullptr;
    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getCutsceneAddress(entry) == cutsceneAddressValue2) {
            cutscene2 = entry;
        }
    }

    return cutscene2;
}

bool PluginKingdomHeartsDays::isCutsceneGameScene()
{
    return GameScene == gameScene_Cutscene;
}

bool PluginKingdomHeartsDays::didIngameCutsceneEnded()
{
    if (!isCutsceneGameScene()) {
        return true;
    }

    if (isSaveLoaded()) {
        // the old cutscene ended, and a new cutscene started
        return _NextCutscene != nullptr;
    }

    return false;
}

bool PluginKingdomHeartsDays::canReturnToGameAfterReplacementCutscene()
{
    if (isSaveLoaded()) {
        // either:
        // 1. the cutscene is over
        // 2. the old cutscene ended, and a new cutscene started, so it needs to be skipped as well
        // 3. the cutscene is unskippable, so even if it didn't end, we need to return
        return !isCutsceneGameScene() || _NextCutscene != nullptr || _IsUnskippableCutscene;
    }
    
    return true;
}

std::filesystem::path PluginKingdomHeartsDays::patchCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath) {
    std::string filename = "hd" + std::string(cutscene->MmName) + ".mp4";
    std::filesystem::path fullPath = folderPath / filename;
    if (!std::filesystem::exists(fullPath))
    {
        // TODO: KH Cutscene should be patched, if needed
        if (strcmp(cutscene->MmName, "802_mm") == 0) {
            fullPath = folderPath / "DOP.mp4";
        }
        if (strcmp(cutscene->MmName, "817_mm") == 0) {
            fullPath = folderPath / "hd817.mp4";
        }
        if (strcmp(cutscene->MmName, "830_mm") == 0) {
            fullPath = folderPath / "hd830.mp4";
        }
        if (strcmp(cutscene->MmName, "833_mm") == 0) {
            fullPath = folderPath / "hd833.mp4";
        }
        if (strcmp(cutscene->MmName, "836_mm") == 0) {
            fullPath = folderPath / "hd836.mp4";
        }
        if (strcmp(cutscene->MmName, "843_mm") == 0) {
            fullPath = folderPath / "DED.mp4";
        }
    }
    if (!std::filesystem::exists(fullPath))
    {
        return "";
    }
    return fullPath;
}

std::string PluginKingdomHeartsDays::CutsceneFilePath(CutsceneEntry* cutscene) {
    std::string filename = "hd" + std::string(cutscene->MmName) + ".mp4";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    filename = "hd" + std::string(cutscene->DsName) + ".mp4";
    fullPath = assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    if (!KH_15_25_Remix_Location.empty()) {
        std::filesystem::path collectionPath = KH_15_25_Remix_Location;
        std::filesystem::path newEpicFolderPath = collectionPath / "EPIC" / "Mare" / "MOVIE" / "Days" / "en";
        if (std::filesystem::exists(newEpicFolderPath)) {
            std::filesystem::path newEpicFullPath = patchCutsceneIfNeeded(cutscene, newEpicFolderPath);
            if (newEpicFullPath != "") {
                return newEpicFullPath.string();
            }
        }
        std::filesystem::path newSteamFolderPath = collectionPath / "STEAM" / "Mare" / "MOVIE" / "Days" / "dt";
        if (std::filesystem::exists(newSteamFolderPath)) {
            std::filesystem::path newSteamFullPath = patchCutsceneIfNeeded(cutscene, newSteamFolderPath);
            if (newSteamFullPath != "") {
                return newSteamFullPath.string();
            }
        }
    }

    return "";
}

bool PluginKingdomHeartsDays::isUnskippableCutscene(CutsceneEntry* cutscene) {
    return isSaveLoaded() && strcmp(cutscene->DsName, "843") == 0;
}

u16 PluginKingdomHeartsDays::detectBackgroundMusic() {
    u16 soundtrack = nds->ARM7Read16(getU32ByCart(SONG_ADDRESS_US, SONG_ADDRESS_EU, SONG_ADDRESS_JP, SONG_ADDRESS_JP_REV1));
    if (soundtrack > 0) {
        return soundtrack;
    }
    return 0;
}

std::string PluginKingdomHeartsDays::BackgroundMusicFilePath(std::string name) {
    std::string filename = name + ".wav";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "audio" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    filename = name + ".mp3";
    fullPath = assetsFolderPath / "audio" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    return "";
}

void PluginKingdomHeartsDays::refreshBackgroundMusic() {
#if !REPLACEMENT_BGM_ENABLED
    return;
#endif

    u16 fakeSoundtrackId = 0x100;
    u16 soundtrackId = detectBackgroundMusic();

    std::string soundtrackPath = BackgroundMusicFilePath("bgm" + std::to_string(soundtrackId));
    bool replacementAvailable = (soundtrackPath != "");

    if (soundtrackId != _CurrentBackgroundMusic) {
        if (soundtrackId == fakeSoundtrackId) {
            _LastSoundtrackId = soundtrackId;
        }
        else if (soundtrackId == 0xFFFF) {
            if (_LastSoundtrackId != fakeSoundtrackId && _CurrentBackgroundMusic != 0) {
                _ShouldStopReplacementBgmMusic = true;
                printf("Stopping replacement song %d\n", _CurrentBackgroundMusic);
    
                _CurrentBackgroundMusic = soundtrackId;
                _LastSoundtrackId = soundtrackId;
            }
        }
        else {
            _ShouldStopReplacementBgmMusic = true;

            if (replacementAvailable) {
                u32 address = getU32ByCart(SONG_ADDRESS_US, SONG_ADDRESS_EU, SONG_ADDRESS_JP, SONG_ADDRESS_JP_REV1);
                nds->ARM7Write16(address, fakeSoundtrackId);

                _ShouldStartReplacementBgmMusic = replacementAvailable;
                printf("Starting replacement song %d\n", soundtrackId);
        
                _CurrentBackgroundMusic = soundtrackId;
                _LastSoundtrackId = soundtrackId;
            }
        }
    }
    else {
        if (replacementAvailable) {
            u32 address = getU32ByCart(SONG_ADDRESS_US, SONG_ADDRESS_EU, SONG_ADDRESS_JP, SONG_ADDRESS_JP_REV1);
            nds->ARM7Write16(address, fakeSoundtrackId);
        }
    
        _CurrentBackgroundMusic = soundtrackId;
        _LastSoundtrackId = soundtrackId;
    }
}

int PluginKingdomHeartsDays::DelayBeforeStartReplacementBgmMusic() {
    u32 currentMission = getCurrentMission();
    if (currentMission == 92 && _CurrentBackgroundMusic == 22) {
        return 12500;
    }
    return 0;
}

std::string PluginKingdomHeartsDays::LocalizationFilePath(std::string language) {
    std::string filename = language + ".ini";
    std::string assetsFolderName = assetsFolder();
    std::string assetsRegionSubfolderName = assetsRegionSubfolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "localization" / assetsRegionSubfolderName / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    return "";
}

u32 PluginKingdomHeartsDays::getCurrentMission()
{
    return nds->ARM7Read8(getU32ByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
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

    u8 val = nds->ARM7Read8(getU32ByCart(CURRENT_INGAME_MENU_VIEW_US, CURRENT_INGAME_MENU_VIEW_EU, CURRENT_INGAME_MENU_VIEW_JP, CURRENT_INGAME_MENU_VIEW_JP_REV1));
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

    u8 world = nds->ARM7Read8(getU32ByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    u8 map = nds->ARM7Read8(getU32ByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
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

void PluginKingdomHeartsDays::debugLogs(int gameScene)
{
    // PRINT_AS_8_BIT_HEX(0x0204c184);
    // PRINT_AS_8_BIT_HEX(0x0204c185);
    // printf("\n");

    if (!DEBUG_MODE_ENABLED) {
        return;
    }

    printf("Game scene: %d\n",  gameScene);
    printf("Current map: %d\n", getCurrentMap());
    printf("Current main menu view: %d\n", getCurrentMainMenuView());
    printf("Is save loaded: %d\n", isSaveLoaded() ? 1 : 0);
    printf("\n");
}

}