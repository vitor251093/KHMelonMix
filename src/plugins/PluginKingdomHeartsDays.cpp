#include "PluginKingdomHeartsDays.h"
#include <cmath>

namespace Plugins
{

u32 PluginKingdomHeartsDays::usGamecode = 1162300249;
u32 PluginKingdomHeartsDays::euGamecode = 1346849625;
u32 PluginKingdomHeartsDays::jpGamecode = 1246186329;

#define ASPECT_RATIO_ADDRESS_US      0x02023C9C
#define ASPECT_RATIO_ADDRESS_EU      0x02023CBC
#define ASPECT_RATIO_ADDRESS_JP      0x02023C9C
#define ASPECT_RATIO_ADDRESS_JP_REV1 0x02023C6C

// 0x2C00 => intro and main menu
#define IS_MAIN_MENU_US      0x0204242d
#define IS_MAIN_MENU_EU      0x0204244d
#define IS_MAIN_MENU_JP      0x0204288d
#define IS_MAIN_MENU_JP_REV1 0x0204284d

// 0x00 => cannot control (ingame cutscenes, or not ingame at all); 0x01 => can control
#define IS_CHARACTER_CONTROLLABLE_US      0x02042460
#define IS_CHARACTER_CONTROLLABLE_EU      0x02042480 // TODO: KH Unconfirmed (calculated)
#define IS_CHARACTER_CONTROLLABLE_JP      0x020428C0 // TODO: KH Unconfirmed (calculated)
#define IS_CHARACTER_CONTROLLABLE_JP_REV1 0x02042880 // TODO: KH Unconfirmed (calculated)

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

#define CURRENT_INGAME_MENU_VIEW_1_US      0x020446b6
#define CURRENT_INGAME_MENU_VIEW_1_EU      0x020446d6 // TODO: KH Unconfirmed (calculated)
#define CURRENT_INGAME_MENU_VIEW_1_JP      0x02044b16 // TODO: KH Unconfirmed (calculated)
#define CURRENT_INGAME_MENU_VIEW_1_JP_REV1 0x02044ad6 // TODO: KH Unconfirmed (calculated)

#define CURRENT_INGAME_MENU_VIEW_2_US      0x020446b8
#define CURRENT_INGAME_MENU_VIEW_2_EU      0x020446d8 // TODO: KH Unconfirmed (calculated)
#define CURRENT_INGAME_MENU_VIEW_2_JP      0x02044b18 // TODO: KH Unconfirmed (calculated)
#define CURRENT_INGAME_MENU_VIEW_2_JP_REV1 0x02044ad8 // TODO: KH Unconfirmed (calculated)

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
#define DEATH_SCREEN_ADDRESS_JP      0x0204c1e4
#define DEATH_SCREEN_ADDRESS_JP_REV1 0x0204c60c

#define DEATH_SCREEN_VALUE_US      0x80
#define DEATH_SCREEN_VALUE_EU      0x80
#define DEATH_SCREEN_VALUE_JP      0x80
#define DEATH_SCREEN_VALUE_JP_REV1 0x00

// 0x60 => The End
#define THE_END_SCREEN_ADDRESS_US      0x0204becd
#define THE_END_SCREEN_ADDRESS_EU      0x0204beed // TODO: KH Unconfirmed (calculated)
#define THE_END_SCREEN_ADDRESS_JP      0x0204c32d // TODO: KH Unconfirmed (calculated)
#define THE_END_SCREEN_ADDRESS_JP_REV1 0x0204c2ed // TODO: KH Unconfirmed (calculated)

#define CURRENT_MISSION_US      0x0204C21C
#define CURRENT_MISSION_EU      0x0204C23C
#define CURRENT_MISSION_JP      0x0204C67C
#define CURRENT_MISSION_JP_REV1 0x0204C63C

#define CURRENT_WORLD_US      0x0204C2CF
#define CURRENT_WORLD_EU      0x0204C2EF
#define CURRENT_WORLD_JP      0x0204C72F
#define CURRENT_WORLD_JP_REV1 0x0204C6EF

#define CURRENT_MAP_FROM_WORLD_US      0x0204C3C8
#define CURRENT_MAP_FROM_WORLD_EU      0x0204C3E8 // TODO: KH Unconfirmed (calculated)
#define CURRENT_MAP_FROM_WORLD_JP      0x0204C828 // TODO: KH Unconfirmed (calculated)
#define CURRENT_MAP_FROM_WORLD_JP_REV1 0x0204C7E8 // TODO: KH Unconfirmed (calculated)

#define IS_DAYS_COUNTER_US      0x0204f6c5
#define IS_DAYS_COUNTER_EU      0x0204f6e5
#define IS_DAYS_COUNTER_JP      0x020508a9
#define IS_DAYS_COUNTER_JP_REV1 0x02050869

#define IS_DAYS_COUNTER_VALUE_US      0x00
#define IS_DAYS_COUNTER_VALUE_EU      0x00
#define IS_DAYS_COUNTER_VALUE_JP      0x10
#define IS_DAYS_COUNTER_VALUE_JP_REV1 0x10

#define IS_LOAD_MENU_US      0x0205ac04
#define IS_LOAD_MENU_EU      0x0205ac24
#define IS_LOAD_MENU_JP      0x0205a5e4
#define IS_LOAD_MENU_JP_REV1 0x0205a5a5

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

#define SONG_ID_ADDRESS_US      0x02191D5E
#define SONG_ID_ADDRESS_EU      0x02192B3E
#define SONG_ID_ADDRESS_JP      0x02190EBE
#define SONG_ID_ADDRESS_JP_REV1 0x02190E3E

#define SSEQ_TABLE_ADDRESS_US      0x020E51B0
#define SSEQ_TABLE_ADDRESS_EU      0x020E5F90
#define SSEQ_TABLE_ADDRESS_JP      0x020E4310
#define SSEQ_TABLE_ADDRESS_JP_REV1 0x020E4290

#define STRM_ADDRESS_US      0x0204B6B4
#define STRM_ADDRESS_EU      0x0204B6D4
#define STRM_ADDRESS_JP      0x0204BB14
#define STRM_ADDRESS_JP_REV1 0x0204BAD4

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US      0x02194CC3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU      0x02195AA3
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP      0x02193E23
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1 0x02193DA3

#define getAnyByCart(usAddress,euAddress,jpAddress,jpRev1Address) (isUsaCart() ? (usAddress) : (isEuropeCart() ? (euAddress) : (isJapanCartRev1() ? (jpRev1Address) : (jpAddress))))

enum
{
    gameScene_Intro,
    gameScene_TitleScreen,
    gameScene_IntroLoadMenu,
    gameScene_SaveMenu,
    gameScene_ConfigMenu,
    gameScene_DayCounter,
    gameScene_Cutscene,
    gameScene_InGameWithMap,
    gameScene_InGameMenu,
    gameScene_PauseMenu,
    gameScene_Tutorial,
    gameScene_InGameWithDouble3D,
    gameScene_MultiplayerMissionReview,
    gameScene_Shop,
    gameScene_LoadingScreen,
    gameScene_RoxasThoughts,
    gameScene_WorldSelector,
    gameScene_DeathScreen,
    gameScene_TheEnd,
    gameScene_Other
};

enum
{
    gameSceneState_showHud,
    gameSceneState_dialogVisible,
    gameSceneState_dialogPortraitLabelVisible,
    gameSceneState_showMinimap,
    gameSceneState_showFullscreenMap,
    gameSceneState_showTarget,
    gameSceneState_showMissionGauge,
    gameSceneState_cutsceneFromChallengeMission,
    gameSceneState_topScreenMissionInformationVisible,
    gameSceneState_showBottomScreenMissionInformation,
    gameSceneState_loadScreenDeletePrompt,
    gameSceneState_bottomScreenSora,
    gameSceneState_bottomScreenCutscene,
    gameSceneState_topScreenCutscene
};

enum
{
    HK_AttackInteract,
    HK_Jump,
    HK_GuardCombo,
    HK_LSwitchTarget,
    HK_RSwitchTarget,
    HK_RLockOn,
    HK_CommandMenuLeft,
    HK_CommandMenuRight,
    HK_CommandMenuUp,
    HK_CommandMenuDown,
    HK_HUDToggle,
    HK_FullscreenMapToggle,
    HK_ReplacementTexturesToggle
};

PluginKingdomHeartsDays::PluginKingdomHeartsDays(u32 gameCode)
{
    GameCode = gameCode;

    hudToggle();

    priorMap = -1;
    Map = 0;

    customKeyMappingNames = {
        "HK_AttackInteract",
        "HK_Jump",
        "HK_GuardCombo",
        "HK_LSwitchTarget",
        "HK_RSwitchTarget",
        "HK_RLockOn",
        "HK_CommandMenuLeft",
        "HK_CommandMenuRight",
        "HK_CommandMenuUp",
        "HK_CommandMenuDown",
        "HK_HUDToggle",
        "HK_FullscreenMapToggle",
        "HK_ReplacementTexturesToggle"
    };
    customKeyMappingLabels = {
        "Attack / Interact",
        "Jump",
        "Guard / Combo",
        "Switch Target - Left",
        "Switch Target - Right",
        "Lock On",
        "Command Menu - Back",
        "Command Menu - Select",
        "Command Menu - Up",
        "Command Menu - Down",
        "HUD Toggle",
        "Fullscreen Map Toggle",
        "Toggle Replacement Textures"
    };

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

    BgmEntries = std::array<BgmEntry, 38> {{
        { 1,  0,    "TwilightR_B",      "Sinister Sundown" },
        { 2,  1,    "TwilightR_F",      "Lazy Afternoons" },
        { 3,  2,    "DestinysForce",    "Destiny's Force" },
        { 4,  3,    "Result",           "Results and Rewards" },
        { 5,  4,    "Alice_F",          "Welcome to Wonderland" },
        { 6,  5,    "Alice_B",          "To Our Surprise" },
        { 7,  6,    "Herc_F",           "Olympus Coliseum" },
        { 8,  7,    "Herc_B",           "Go for It!" },
        { 9,  8,    "Halloween_F",      "This is Halloween" },
        { 10, 9,    "Halloween_B",      "Spooks of Halloween Town" },
        { 11, 10,   "Alasin_F",         "A Day in Agrabah" },
        { 12, 11,   "Alasin_B",         "Arabian Dream" },
        { 13, 12,   "Existence_F",      "Sacred Moon" },
        { 14, 13,   "Existence_B",      "Critical Drive" },
        { 15, 14,   "SacredMoon",       "Mystic Moon" },
        { 16, 15,   "Beast_F",          "Waltz of the Damned" },
        { 17, 16,   "Beast_B",          "Dance of the Daring" },
        { 18, 17,   "ThemeXIII",        "Organization XIII" },
        { 19, 18,   "ThemeRoxas",       "Roxas" },
        { 20, 19,   "MissionBoss1",     "Tension Rising" },
        { 21, 20,   "DisneyBoss1",      "Rowdy Rumble" },
        { 22, 21,   "XIVtheme",         "Musique pour la tristesse de Xion" },
        { 23, 22,   "ShorodingDark",    "Shrouding Dark Cloud" },
        { 24, 23,   "Boss3",            "Vim and Vigor" },
        { 25, 24,   "Riku",             "Riku" },
        { 26, 25,   "StrangeWhispers",  "Strange Whispers" },
        // note: no Song 27 in the DS IDs!
        { 28, 26,   "ThemeOfFriends",   "Theme of Friends" },
        { 29, 27,   "MissingYou",       "Missing You" },
        { 30, 28,   "Resultsingle",     "Crossing the Finish Line" },
        { 31, 29,   "Entrymulti",       "Cavern of Remembrance" },
        { 32, 30,   "XIIItheme2",       "Xemnas" },
        { 33, 31,   "Neverland_F",      "Secret of Neverland" },
        { 34, 32,   "Neverland_B",      "Crossing to Neverland" },
        { 35, 33,   "Icetime",          "At dusk I will think of you" },
        { 36, 34,   "Boss4",            "Fight and Away" },
        { 37, 35,   "RikuBattle",       "Another Side Battle Version" },
        { 38, 36,   "Xionbattle",       "Vector to the Heavens" }
    }};

    StreamedBgmEntries = std::array<StreamedBgmEntry, 2> {{
        { 0x5a, 0, "Dearly Beloved", 2900195 },
        { 0x78, 39, "Dearly Beloved (Reprise)", 2459033 }
    }};
}

void PluginKingdomHeartsDays::loadLocalization() {
    u8* rom = (u8*)nds->GetNDSCart()->GetROM();

    std::string language = TextLanguage;
    if (language == "") {
        language = "en-US";
    }

    std::string LocalizationFilePath = localizationFilePath(language);
    Platform::FileHandle* f = Platform::OpenLocalFile(LocalizationFilePath.c_str(), Platform::FileMode::ReadText);
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
    Plugin::onLoadROM();

    loadLocalization();

    u8* rom = (u8*)nds->GetNDSCart()->GetROM();

    // Getting cutscene address offset in ROM, so we can support patched ROMs
    u32 firstCutsceneAddr = 0;
    std::array<u8, 0x20> firstCutsceneContent = {0x4d, 0x4f, 0x44, 0x53, 0x4e, 0x33, 0x0a, 0x00, 0x85, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0xf6, 0x28, 0xfc, 0x0e, 0x03, 0x00, 0x02, 0x00, 0xd8, 0x7f, 0x00, 0x00 };
    for (int addr = 0x08000000; addr < 0x0b000000; addr++) {
        bool match = true;
        for (int offset = 0; offset < 0x20; offset++) {
            if (rom[addr + offset] != firstCutsceneContent[offset]) {
                match = false;
                break;
            }
        }
        if (match) {
            firstCutsceneAddr = addr;
            break;
        }
    }
    cutscenesAddressOffset = firstCutsceneAddr - getMobiCutsceneAddress(&Cutscenes[0]);
}

void PluginKingdomHeartsDays::onLoadState() {
    Plugin::onLoadState();

    loadLocalization();

    GameScene = gameScene_InGameWithMap;
}

std::string PluginKingdomHeartsDays::assetsFolder() {
    return "days";
}

std::string PluginKingdomHeartsDays::assetsRegionSubfolder() {
    return getAnyByCart("us", "eu", "jp", "jp_rev1");
}

std::string PluginKingdomHeartsDays::tomlUniqueIdentifier() {
    return getAnyByCart("KHDays_US", "KHDays_EU", "KHDays_JP", "KHDays_JPRev1");
}

void PluginKingdomHeartsDays::renderer_beforeBuildingShapes()
{
    if (GameScene == gameScene_InGameWithDouble3D)
    {
        bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;

        u16 bottomScreenMasterBrightness = has3DOnTopScreen ? nds->GPU.GPU2D_B.MasterBrightness : nds->GPU.GPU2D_A.MasterBrightness;
        _hasVisible3DOnBottomScreen = true;

        // fade from/to white, on "Mission Complete"
        if (bottomScreenMasterBrightness & (1 << 14)) {
            _hasVisible3DOnBottomScreen = false;
        }
        // fade from/to black, on victory pose
        // cheshire cat dialog
        if ((bottomScreenMasterBrightness & (1 << 15)) &&
            ((bottomScreenMasterBrightness & 0x10) == 1 || (bottomScreenMasterBrightness & 0xF) < 4)) {
            _hasVisible3DOnBottomScreen = false;
        }

        _priorPriorIgnore3DOnBottomScreen = _priorIgnore3DOnBottomScreen;
        _priorIgnore3DOnBottomScreen = _ignore3DOnBottomScreen;
        _ignore3DOnBottomScreen = isBottomScreen2DTextureBlack();

        ShouldShowBottomScreen = _hasVisible3DOnBottomScreen && (!_ignore3DOnBottomScreen || !_priorIgnore3DOnBottomScreen || !_priorPriorIgnore3DOnBottomScreen);

        if (DaysDisableHisMemories)
        {
            ShouldShowBottomScreen = false;
        }
    }
}

void PluginKingdomHeartsDays::renderer_afterBuildingShapes()
{
    /*if (GameScene == gameScene_InGameWithDouble3D)
    {
        bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;
        bool has3DOnBottomScreen = (nds->PowerControl9 >> 9) == 1;

        if (has3DOnTopScreen)
        {
            int FrontBuffer = nds->GPU.FrontBuffer ? 0 : 1;
            u32* topScreen = nds->GPU.Framebuffer[FrontBuffer][1].get();
            memcpy(&_double3DTopScreen2DTexture[0], &topScreen[0], 256 * 192 * 4);
            _double3DTopScreen2DTextureEnabled = true;
        }
        else if (has3DOnBottomScreen && _double3DTopScreen2DTextureEnabled)
        {
            int FrontBuffer = nds->GPU.FrontBuffer ? 0 : 1;
            u32* topScreen = nds->GPU.Framebuffer[FrontBuffer][0].get();
            u32* bottomScreen = nds->GPU.Framebuffer[FrontBuffer][1].get();
            memcpy(&bottomScreen[0], &_double3DTopScreen2DTexture[0], 256 * 192 * 4);
            nds->GPU.GetRenderer2D().SetFramebuffer(topScreen, bottomScreen);
        }
    }
    else
    {
        _double3DTopScreen2DTextureEnabled = false;
    }*/
}

void PluginKingdomHeartsDays::renderer_2DShapes_saveScreenMenu(std::vector<ShapeData2D>* shapes, float aspectRatio, float hudScale)
{
    // save label
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 0)
            .withSize(100, 16)
            .placeAtCorner(corner_TopLeft)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // rest of save label header
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(100, 0)
            .withSize(20, 16)
            .placeAtCorner(corner_TopRight)
            .sourceScale(1000.0, 1.0)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // footer
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 144)
            .withSize(256, 48)
            .placeAtCorner(corner_BottomLeft)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // rest of footer
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(251, 144)
            .withSize(5, 48)
            .placeAtCorner(corner_BottomRight)
            .sourceScale(1000.0, 1.0)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // main content
    shapes->push_back(ShapeBuilder2D::square()
            .placeAtCorner(corner_Center)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));
}

void PluginKingdomHeartsDays::renderer_2DShapes_loadScreenMenu(std::vector<ShapeData2D>* shapes, float aspectRatio, float hudScale)
{
    bool showDeletePrompt = ((GameSceneState & (1 << gameSceneState_loadScreenDeletePrompt)) > 0);

    // load label
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(0, 0)
            .withSize(100, 16)
            .placeAtCorner(corner_TopLeft)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // rest of load label header
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(100, 0)
            .withSize(20, 16)
            .placeAtCorner(corner_TopRight)
            .sourceScale(1000.0, 1.0)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // footer
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(0, showDeletePrompt ? 128 : 144)
            .withSize(256, showDeletePrompt ? 64 : 48)
            .placeAtCorner(corner_BottomLeft)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // rest of footer
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(251, showDeletePrompt ? 128 : 144)
            .withSize(5, showDeletePrompt ? 64 : 48)
            .placeAtCorner(corner_BottomRight)
            .sourceScale(1000.0, 1.0)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // main content
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .placeAtCorner(corner_Center)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    // background
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(252, 16)
            .withSize(3, 80)
            .placeAtCorner(corner_Center)
            .sourceScale(1000.0)
            .hudScale(hudScale)
            .build(aspectRatio));
}

void PluginKingdomHeartsDays::renderer_2DShapes_component_characterDialog(std::vector<ShapeData2D>* shapes, float aspectRatio, float hudScale)
{
    float bottomMargin = 7.0;

    // this is required to keep the same size and position regardless of HUD scale;
    // the value 5.333 guarantees that, if the game is running in 4:3, the dialogs
    // will have the original look from the NDS
    float dialogScale = 5.333/hudScale;

    if ((GameSceneState & (1 << gameSceneState_dialogPortraitLabelVisible)) > 0)
    {
        // dialog (portrait label right side)
        shapes->push_back(ShapeBuilder2D::square()
                .fromPosition(184, 170)
                .withSize(7, 14)
                .placeAtCorner(corner_Bottom)
                .sourceScale(dialogScale)
                .withMargin(128 * dialogScale, 0.0, 0.0, bottomMargin + 8 * dialogScale)
                .mirror(mirror_X)
                .hudScale(hudScale)
                .build(aspectRatio));
    }

    int boxHeight = dialogBoxHeight();

    // dialog (biggest part)
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 30)
            .withSize(256, 162)
            .placeAtCorner(corner_Bottom)
            .sourceScale(dialogScale)
            .withMargin(0.0, 0.0, 0.0, bottomMargin)
            .hudScale(hudScale)
            .build(aspectRatio));

    // dialog (left side border)
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 192 - 8 - 4)
            .withSize(boxHeight, 4)
            .placeAtCorner(corner_Bottom)
            .sourceScale(dialogScale)
            .rotateToTheRight()
            .cropSquareCorners(0.0, 4.0, 0.0, 4.0)
            .withMargin(0.0, 0.0, 136 * dialogScale, bottomMargin + 8 * dialogScale)
            .hudScale(hudScale)
            .build(aspectRatio));

    // dialog (left side)
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 30)
            .withSize(3, 162)
            .placeAtCorner(corner_Bottom)
            .sourceScale(dialogScale * 6.5, dialogScale)
            .withMargin(0.0, 0.0, 128 * dialogScale, bottomMargin)
            .hudScale(hudScale)
            .build(aspectRatio));

    // dialog (right side border)
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 192 - 8 - 4)
            .withSize(boxHeight, 4)
            .placeAtCorner(corner_Bottom)
            .sourceScale(dialogScale)
            .rotateToTheLeft()
            .cropSquareCorners(4.0, 0.0, 4.0, 0.0)
            .withMargin(136 * dialogScale, 0.0, 0.0, bottomMargin + 8 * dialogScale)
            .hudScale(hudScale)
            .build(aspectRatio));

    // dialog (right side)
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(0, 30)
            .withSize(3, 162)
            .placeAtCorner(corner_Bottom)
            .sourceScale(dialogScale * 6.5, dialogScale)
            .withMargin(128 * dialogScale, 0.0, 0.0, bottomMargin)
            .hudScale(hudScale)
            .build(aspectRatio));

    // background
    shapes->push_back(ShapeBuilder2D::square()
            .fromPosition(118, 50)
            .withSize(20, 10)
            .placeAtCorner(corner_Center)
            .sourceScale(1000.0)
            .hudScale(hudScale)
            .build(aspectRatio));
}

void PluginKingdomHeartsDays::renderer_2DShapes_component_targetView(std::vector<ShapeData2D>* shapes, float aspectRatio, float hudScale)
{
    float targetScale = 0.666;
    int targetLabelMargin = 12;
    int targetWidth = 64;
    int targetRightMargin = 70;

    // target label (part 1)
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(32, 51)
            .withSize(targetLabelMargin, 9)
            .placeAtCorner(corner_TopRight)
            .withMargin(0.0, 30.0, 9.0 + targetWidth*targetScale - targetLabelMargin*targetScale + targetRightMargin, 0.0)
            .sourceScale(targetScale)
            .colorToAlpha(248, 248, 248)
            .hudScale(hudScale)
            .build(aspectRatio));

    // target label (part 2)
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(32 + targetLabelMargin, 51)
            .withSize(targetWidth - targetLabelMargin*2, 9)
            .placeAtCorner(corner_TopRight)
            .withMargin(0.0, 30.0, 9.0 + targetLabelMargin*targetScale + targetRightMargin, 0.0)
            .sourceScale(targetScale)
            .hudScale(hudScale)
            .build(aspectRatio));

    // target label (part 3)
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(32 + targetWidth - targetLabelMargin, 51)
            .withSize(targetLabelMargin, 9)
            .placeAtCorner(corner_TopRight)
            .withMargin(0.0, 30.0, 9.0 + targetRightMargin, 0.0)
            .sourceScale(targetScale)
            .colorToAlpha(248, 248, 248)
            .hudScale(hudScale)
            .build(aspectRatio));

    // target
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(32, 64)
            .withSize(targetWidth, 76)
            .placeAtCorner(corner_TopRight)
            .withMargin(0.0, 38.0, 9.0 + targetRightMargin, 0.0)
            .sourceScale(targetScale)
            .hudScale(hudScale)
            .build(aspectRatio));
}

void PluginKingdomHeartsDays::renderer_2DShapes_component_bottomMissionInformation(std::vector<ShapeData2D>* shapes, float aspectRatio, float hudScale)
{
    // bottom mission information (part 1)
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(0, 0)
            .withSize(75, 24)
            .placeAtCorner(corner_TopLeft)
            .singleColorToAlpha(32, 32, 32)
            .hudScale(hudScale)
            .build(aspectRatio));

    // bottom mission information (part 2)
    shapes->push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(75, 8)
            .withSize(181, 16)
            .placeAtCorner(corner_TopLeft)
            .withMargin(75.0, 8.0, 0.0, 0.0)
            .fadeBorderSize(0.0, 0.0, 64.0, 0.0)
            .hudScale(hudScale)
            .build(aspectRatio));
}

std::vector<ShapeData2D> PluginKingdomHeartsDays::renderer_2DShapes() {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    auto shapes = std::vector<ShapeData2D>();
    float hudScale = (((float)UIScale) - 4) / 2 + 4;
    int fullscreenMapTransitionDuration = 20;

    if (!SingleScreenMode &&
            GameScene != gameScene_InGameWithMap &&
            GameScene != gameScene_DeathScreen &&
            GameScene != gameScene_PauseMenu &&
            GameScene != gameScene_InGameWithDouble3D) {
        shapes.push_back(ShapeBuilder2D::square()
                    .placeAtCorner(corner_Center)
                    .hudScale(hudScale)
                    .preserveDsScale()
                    .build(aspectRatio));
        return shapes;
    }

    switch (GameScene) {
        case gameScene_IntroLoadMenu:
            renderer_2DShapes_loadScreenMenu(&shapes, aspectRatio, hudScale);
            break;

        case gameScene_DayCounter:
        case gameScene_RoxasThoughts:
            shapes.push_back(ShapeBuilder2D::square()
                    .placeAtCorner(corner_Center)
                    .hudScale(hudScale)
                    .preserveDsScale()
                    .build(aspectRatio));
            break;

        case gameScene_Cutscene:
            if ((GameSceneState & (1 << gameSceneState_bottomScreenCutscene)) > 0) {
                shapes.push_back(ShapeBuilder2D::square()
                        .fromBottomScreen()
                        .placeAtCorner(corner_Center)
                        .hudScale(hudScale)
                        .preserveDsScale()
                        .build(aspectRatio));
            }
            if ((GameSceneState & (1 << gameSceneState_topScreenCutscene)) > 0) {
                shapes.push_back(ShapeBuilder2D::square()
                        .placeAtCorner(corner_Center)
                        .hudScale(hudScale)
                        .preserveDsScale()
                        .build(aspectRatio));
            }
            break;

        case gameScene_InGameWithDouble3D:
            if (SingleScreenMode && (GameSceneState & (1 << gameSceneState_bottomScreenSora)) > 0) {
                break;
            }

        case gameScene_InGameWithMap:
            if ((GameSceneState & (1 << gameSceneState_showFullscreenMap)) > 0)
            {
                fullscreenMapTransitionStep += 1;
                if (fullscreenMapTransitionStep > fullscreenMapTransitionDuration) {
                    fullscreenMapTransitionStep = fullscreenMapTransitionDuration;
                }
            }
            else
            {
                fullscreenMapTransitionStep -= 1;
                if (fullscreenMapTransitionStep < 0) {
                    fullscreenMapTransitionStep = 0;
                }
            }

            if ((GameSceneState & (1 << gameSceneState_cutsceneFromChallengeMission)) > 0)
            {
                // top 'challenge' or 'holo-mission' label
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(0, 0)
                        .withSize(256, 24)
                        .placeAtCorner(corner_TopLeft)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                // line on the right side of the label
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(128, 0)
                        .withSize(128, 24)
                        .placeAtCorner(corner_TopRight)
                        .sourceScale(10.0, 1.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));
            }

            if ((GameSceneState & (1 << gameSceneState_dialogVisible)) > 0)
            {
                renderer_2DShapes_component_characterDialog(&shapes, aspectRatio, hudScale);
                return shapes;
            }

            if ((GameSceneState & (1 << gameSceneState_cutsceneFromChallengeMission)) > 0)
            {
                // 'any player can press start to skip'
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(0, 152)
                        .withSize(256, 40)
                        .placeAtCorner(corner_Bottom)
                        .withMargin(0.0, 0.0, 0.0, 7.0)
                        .sourceScale(1.5)
                        .hudScale(hudScale)
                        .build(aspectRatio));
            }

            if ((GameSceneState & (1 << gameSceneState_topScreenMissionInformationVisible)) > 0) {
                // top mission information
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(0, 0)
                        .withSize(256, 24)
                        .placeAtCorner(corner_TopLeft)
                        .fadeBorderSize(0.0, 0.0, 64.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                return shapes;
            }

            if (SingleScreenMode) {
                if ((GameSceneState & (1 << gameSceneState_showBottomScreenMissionInformation)) > 0) {
                    renderer_2DShapes_component_bottomMissionInformation(&shapes, aspectRatio, hudScale);
                }
            }

            if ((GameSceneState & (1 << gameSceneState_showHud)) > 0)
            {
                // item notification and timer (left side of the screen)
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(0, 0)
                        .withSize(108, 86)
                        .placeAtCorner(corner_TopLeft)
                        .withMargin(0.0, 35.0, 0.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                // countdown and locked on (top center of the screen)
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(93, 0)
                        .withSize(70, 20)
                        .placeAtCorner(corner_Top)
                        .withMargin(0.0, 9.0, 0.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                if (SingleScreenMode)
                {
                    bool showMinimap = (GameSceneState & (1 << gameSceneState_showMinimap)) > 0;
                    if (showMinimap) {
                        ShapeData2D minimapShape = ShapeBuilder2D::square()
                                .fromBottomScreen()
                                .fromPosition(128, 60)
                                .withSize(72, 72)
                                .placeAtCorner(corner_TopRight)
                                .withMargin(0.0, 30.0, 12.0, 0.0)
                                .sourceScale(0.972)
                                .fadeBorderSize(5.0, 5.0, 5.0, 5.0)
                                .opacity(0.85)
                                .invertGrayScaleColors()
                                .hudScale(hudScale)
                                .build(aspectRatio);

                        float fullscreenDegree = ((float)fullscreenMapTransitionStep) / fullscreenMapTransitionDuration;
                        if (fullscreenDegree > 0)
                        {
                            ShapeData2D bigMapShape = ShapeBuilder2D::square()
                                    .fromBottomScreen()
                                    .fromPosition(104, 56)
                                    .withSize(120, 80)
                                    .placeAtCorner(corner_Center)
                                    .sourceScale(2.2)
                                    .fadeBorderSize(7.0)
                                    .opacity(0.90)
                                    .invertGrayScaleColors()
                                    .hudScale(hudScale)
                                    .build(aspectRatio);
                            minimapShape.transitionTo(bigMapShape, fullscreenDegree);
                        }

                        // minimap
                        shapes.push_back(minimapShape);
                    }

                    if ((GameSceneState & (1 << gameSceneState_showTarget)) > 0) {
                        renderer_2DShapes_component_targetView(&shapes, aspectRatio, hudScale);
                    }

                    if ((GameSceneState & (1 << gameSceneState_showMissionGauge)) > 0) {
                        // mission gauge
                        shapes.push_back(ShapeBuilder2D::square()
                                .fromBottomScreen()
                                .fromPosition(5, 152)
                                .withSize(246, 40)
                                .placeAtCorner(corner_Bottom)
                                .cropSquareCorners(6.0, 6.0, 0.0, 0.0)
                                .hudScale(hudScale)
                                .build(aspectRatio));
                    }
                }

                // enemy health
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(163, 0)
                        .withSize(93, 22)
                        .placeAtCorner(corner_TopRight)
                        .withMargin(0.0, 7.5, 9.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                // sigils and death counter
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(163, 25)
                        .withSize(93, 30)
                        .placeAtCorner(corner_TopRight)
                        .withMargin(0.0, 104.5, 12.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                // command menu
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(0, 86)
                        .withSize(108, 106)
                        .placeAtCorner(corner_BottomLeft)
                        .sourceScale(1.2)
                        .withMargin(10.0, 0.0, 0.0, 0.0)
                        .hudScale(hudScale)
                        .build(aspectRatio));

                // player health (and player allies)
                shapes.push_back(ShapeBuilder2D::square()
                        .fromPosition(128, 84)
                        .withSize(128, 107)
                        .placeAtCorner(corner_BottomRight)
                        .sourceScale(1.2)
                        .withMargin(0.0, 0.0, 10.0, 10.5)
                        .hudScale(hudScale)
                        .build(aspectRatio));
            }

            /*if ((GameSceneState & (1 << gameSceneState_bottomScreenSora)) > 0) {
                // background
                shapes.push_back(ShapeBuilder2D::square()
                        .fromBottomScreen()
                        .sourceScale(aspectRatio, 1.0)
                        .placeAtCorner(corner_Center)
                        .hudScale(hudScale)
                        .preserveDsScale()
                        .build(aspectRatio));

                break;
            }*/

            // background
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(118, 162)
                    .withSize(20, 10)
                    .placeAtCorner(corner_Center)
                    .sourceScale(1000.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            break;

        case gameScene_PauseMenu:
            if (SingleScreenMode) {
                if ((GameSceneState & (1 << gameSceneState_showBottomScreenMissionInformation)) > 0) {
                    renderer_2DShapes_component_bottomMissionInformation(&shapes, aspectRatio, hudScale);
                }

                if ((GameSceneState & (1 << gameSceneState_showMissionGauge)) > 0) {
                    // mission gauge
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromBottomScreen()
                            .fromPosition(5, 152)
                            .withSize(246, 40)
                            .placeAtCorner(corner_Bottom)
                            .cropSquareCorners(6.0, 6.0, 0.0, 0.0)
                            .hudScale(hudScale)
                            .build(aspectRatio));
                }
            }

            // pause menu
            shapes.push_back(ShapeBuilder2D::square()
                    .placeAtCorner(corner_Center)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // background
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(118, 182)
                    .withSize(20, 10)
                    .placeAtCorner(corner_Center)
                    .sourceScale(1000.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            break;
    
        case gameScene_Tutorial:
            // tutorial
            shapes.push_back(ShapeBuilder2D::square()
                    .fromBottomScreen()
                    .fromPosition(5, 0)
                    .withSize(246, 192)
                    .placeAtCorner(corner_Center)
                    .sourceScale(5.0/hudScale)
                    .squareBorderRadius(10.0, 10.0, 5.0, 5.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // background
            shapes.push_back(ShapeBuilder2D::square()
                    .fromBottomScreen()
                    .fromPosition(0, 96)
                    .withSize(5, 5)
                    .placeAtCorner(corner_Center)
                    .sourceScale(1000.0)
                    .opacity(0.75)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            break;

        case gameScene_WorldSelector:
            // header left corner
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(0, 0)
                    .withSize(128, 25)
                    .placeAtCorner(corner_TopLeft)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // header right corner
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(128, 0)
                    .withSize(128, 25)
                    .placeAtCorner(corner_TopRight)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // header middle
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(123, 0)
                    .withSize(10, 25)
                    .placeAtCorner(corner_Top)
                    .sourceScale(1000.0, 1.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // world name and rank
            shapes.push_back(ShapeBuilder2D::square()
                    .fromPosition(0, 176)
                    .withSize(256, 16)
                    .placeAtCorner(corner_BottomLeft)
                    .squareBorderRadius(5.0, 5.0, 5.0, 5.0)
                    .withMargin(20.0, 0.0, 0.0, 10.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            // mission selector
            shapes.push_back(ShapeBuilder2D::square()
                    .fromBottomScreen()
                    .fromPosition(0, 0)
                    .withSize(256, 192)
                    .placeAtCorner(corner_Right)
                    .sourceScale(0.7)
                    .cropSquareCorners(5.0, 0.0, 5.0, 0.0)
                    .opacity(1.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));

            break;

        case gameScene_SaveMenu:
        {
            renderer_2DShapes_saveScreenMenu(&shapes, aspectRatio, hudScale);
            break;
        }

        case gameScene_ConfigMenu:
        {
            shapes.push_back(ShapeBuilder2D::square()
                            .placeAtCorner(corner_Center)
                            .hudScale(hudScale)
                            .preserveDsScale()
                            .build(aspectRatio));
            break;
        }

        case gameScene_InGameMenu:
        {
            u32 curMenu = getCurrentMainMenuView();
            // the others are in horizontal style

            switch (curMenu) {
                case 3:   // roxas's diary
                case 4: { // enemy profile
                    // header left corner
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromPosition(0, 0)
                            .withSize(128, 16)
                            .placeAtCorner(corner_TopLeft)
                            .hudScale(hudScale)
                            .build(aspectRatio));

                    // header right corner
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromPosition(128, 0)
                            .withSize(128, 16)
                            .placeAtCorner(corner_TopRight)
                            .hudScale(hudScale)
                            .build(aspectRatio));

                    // header middle
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromPosition(123, 0)
                            .withSize(10, 16)
                            .placeAtCorner(corner_Top)
                            .sourceScale(1000.0, 1.0)
                            .hudScale(hudScale)
                            .build(aspectRatio));

                    float doubleScreenScale = aspectRatio * 0.5;
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromBottomScreen()
                            .placeAtCorner(corner_Left)
                            .sourceScale(doubleScreenScale, doubleScreenScale)
                            .hudScale(hudScale)
                            .preserveDsScale()
                            .build(aspectRatio));

                    shapes.push_back(ShapeBuilder2D::square()
                            .fromPosition(0, 16)
                            .withSize(256, 176)
                            .placeAtCorner(corner_Right)
                            .sourceScale(doubleScreenScale, doubleScreenScale)
                            .hudScale(hudScale)
                            .preserveDsScale()
                            .build(aspectRatio));

                    // background
                    shapes.push_back(ShapeBuilder2D::square()
                            .fromBottomScreen()
                            .withSize(256, 8)
                            .placeAtCorner(corner_TopLeft)
                            .sourceScale(doubleScreenScale, doubleScreenScale)
                            .hudScale(hudScale)
                            .preserveDsScale()
                            .repeatAsBackground()
                            .build(aspectRatio));

                    break;
                }
                default:
                    break;
            }
            break;
        }

        case gameScene_LoadingScreen:
            shapes.push_back(ShapeBuilder2D::square()
                    .fromBottomScreen()
                    .placeAtCorner(corner_BottomRight)
                    .hudScale(hudScale)
                    .build(aspectRatio));
            break;
    
        case gameScene_DeathScreen:
            shapes.push_back(ShapeBuilder2D::square()
                    .placeAtCorner(corner_Center)
                    .withMargin(0.0, 20.0, 0.0, 0.0)
                    .hudScale(hudScale)
                    .build(aspectRatio));
            break;
    }
    
    return shapes;
}

std::vector<ShapeData3D> PluginKingdomHeartsDays::renderer_3DShapes() {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    auto shapes = std::vector<ShapeData3D>();

    if (!SingleScreenMode &&
            GameScene != gameScene_InGameWithMap &&
            GameScene != gameScene_DeathScreen &&
            GameScene != gameScene_PauseMenu &&
            GameScene != gameScene_InGameWithDouble3D &&
            GameScene != gameScene_WorldSelector) {
        bool has3DOnTopScreen = (nds->PowerControl9 >> 15) == 1;
        if (has3DOnTopScreen && GameScene != gameScene_WorldSelector) {
            shapes.push_back(ShapeBuilder3D::square()
                            .placeAtCorner(corner_Center)
                            .build(aspectRatio));
        }
        return shapes;
    }

    int gameSceneState = renderer_gameSceneState();
    switch (GameScene) {
        case gameScene_PauseMenu:
            shapes.push_back(ShapeBuilder3D::square()
                    .placeAtCorner(corner_Center)
                    .zRange(-1.0, -0.0007)
                    .hide()
                    .build(aspectRatio));
            break;

        case gameScene_InGameWithMap:
        case gameScene_InGameWithDouble3D:
            if ((gameSceneState & (1 << gameSceneState_showHud)) > 0)
            {
                // aim
                shapes.push_back(ShapeBuilder3D::square()
                        .polygonMode()
                        .polygonVertexesCount(4)
                        .polygonAttributes(1058996416)
                        .includeOutOfBoundsPolygons()
                        .zRange(-10000.0, -0.00000)
                        .adjustAspectRatioOnly()
                        .build(aspectRatio));

                // aim
                shapes.push_back(ShapeBuilder3D::square()
                        .polygonMode()
                        .polygonVertexesCount(4)
                        .polygonAttributes(1042219200)
                        .includeOutOfBoundsPolygons()
                        .zRange(-10000.0, -0.00000)
                        .adjustAspectRatioOnly()
                        .build(aspectRatio));

                bool showMissionInfo = (gameSceneState & (1 << gameSceneState_topScreenMissionInformationVisible)) > 0 ||
                                       (gameSceneState & (1 << gameSceneState_showBottomScreenMissionInformation)) > 0;
                float heartTopMargin = (showMissionInfo ? 24.0 : 0.0);

                // blue shine behind the heart counter and "CHAIN" label
                shapes.push_back(ShapeBuilder3D::square()
                        .withSize(128, 77)
                        .placeAtCorner(corner_TopLeft)
                        .withMargin(0.0, heartTopMargin, 0.0, 0.0)
                        .zRange(-0.0004, -0.0002)
                        .hudScale(UIScale)
                        .build(aspectRatio));

                // heart counter, timer, "BONUS" label and +X floating labels
                shapes.push_back(ShapeBuilder3D::square()
                        .withSize(102, 48)
                        .placeAtCorner(corner_TopLeft)
                        .withMargin(0.0, heartTopMargin, 0.0, 0.0)
                        .zRange(-0.0008, -0.0006)
                        .hudScale(UIScale)
                        .negatePolygonAttributes(34144384) // rain
                        .negatePolygonAttributes(34799744) // rain
                        .build(aspectRatio));
            }
            else
            {
                // no HUD
                shapes.push_back(ShapeBuilder3D::square()
                        .placeAtCorner(corner_Center)
                        .zRange(-1.0, -0.0007)
                        .hide()
                        .build(aspectRatio));
            }
            break;

        case gameScene_DeathScreen:
            shapes.push_back(ShapeBuilder3D::square()
                    .placeAtCorner(corner_Center)
                    .zRange(-1.0, -0.0007)
                    .withMargin(0.0, 20.0, 0.0, 0.0)
                    .hudScale(UIScale)
                    .build(aspectRatio));
            break;
    }

    return shapes;
};

int PluginKingdomHeartsDays::renderer_gameSceneState() {
    int state = 0;

    switch (GameScene) {
        case gameScene_IntroLoadMenu:
            if (isLoadScreenDeletePromptVisible()) {
                state |= (1 << gameSceneState_loadScreenDeletePrompt);
            }
            break;

        case gameScene_DayCounter:
            break;

        case gameScene_RoxasThoughts:
            break;

        case gameScene_Cutscene:
            if (detectTopScreenMobiCutscene() == nullptr) {
                state |= (1 << gameSceneState_bottomScreenCutscene);
            }
            else if (detectBottomScreenMobiCutscene() == nullptr) {
                state |= (1 << gameSceneState_topScreenCutscene);
            }
            break;

        case gameScene_InGameWithDouble3D:
            if (ShouldShowBottomScreen) {
                state |= (1 << gameSceneState_bottomScreenSora);
                state |= (1 << gameSceneState_showHud);
                break;
            }

        case gameScene_InGameWithMap:
            {
                bool _isMinimapVisible = isMinimapVisible();
                bool _isDialogVisible = isDialogVisible();
                bool _isMissionInformationVisibleOnTopScreen = isMissionInformationVisibleOnTopScreen();
                bool _isMissionInformationVisibleOnBottomScreen = isMissionInformationVisibleOnBottomScreen();
                bool _isMissionGaugeVisibleOnBottomScreen = isMissionGaugeVisibleOnBottomScreen();
                bool _isTargetVisibleOnBottomScreen = isTargetVisibleOnBottomScreen();
                bool _isCutsceneFromChallengeMissionVisible = isCutsceneFromChallengeMissionVisible();
                bool _isDialogPortraitLabelVisible = isDialogPortraitLabelVisible();

                if ((GameScene == gameScene_InGameWithMap && (!isCharacterControllable || _isDialogVisible)) ||
                    (GameScene == gameScene_InGameWithDouble3D && _isDialogVisible))
                {
                    state |= (1 << gameSceneState_dialogVisible);

                    if (_isCutsceneFromChallengeMissionVisible) {
                        state |= (1 << gameSceneState_cutsceneFromChallengeMission);
                    }

                    if (_isDialogPortraitLabelVisible) {
                        state |= (1 << gameSceneState_dialogPortraitLabelVisible);
                    }

                    break;
                }

                if (_isMissionInformationVisibleOnTopScreen) {
                    state |= (1 << gameSceneState_topScreenMissionInformationVisible);

                    break;
                }

                if (!_isMissionInformationVisibleOnTopScreen && ShowMissionInfo && _isMissionInformationVisibleOnBottomScreen) {
                    state |= (1 << gameSceneState_showBottomScreenMissionInformation);
                }

                if (_isCutsceneFromChallengeMissionVisible) {
                    state |= (1 << gameSceneState_cutsceneFromChallengeMission);

                    break;
                }

                if (!HideAllHUD) {
                    if (isCharacterControllable || GameScene == gameScene_InGameWithDouble3D) {
                        state |= (1 << gameSceneState_showHud);
                    }
                }

                if (GameScene == gameScene_InGameWithMap && _isMinimapVisible) {
                    if (ShowFullscreenMap) {
                        state |= (1 << gameSceneState_showFullscreenMap);
                    }

                    if (ShowMap) {
                        state |= (1 << gameSceneState_showMinimap);
                    }

                    if (ShowTarget && _isTargetVisibleOnBottomScreen) {
                        state |= (1 << gameSceneState_showTarget);
                    }

                    if (ShowMissionGauge && _isMissionGaugeVisibleOnBottomScreen) {
                        state |= (1 << gameSceneState_showMissionGauge);
                    }
                }
            }

            break;

        case gameScene_PauseMenu:
            if (PriorGameScene != gameScene_InGameWithDouble3D)
            {
                if (isMissionInformationVisibleOnBottomScreen()) {
                    state |= (1 << gameSceneState_showBottomScreenMissionInformation);
                }

                if (isMinimapVisible()) {
                    state |= (1 << gameSceneState_showMissionGauge);
                }
            }

            break;

        case gameScene_Tutorial:
            break;

        case gameScene_LoadingScreen:
            break;

        case gameScene_DeathScreen:
            break;
    }

    return state;
};

int PluginKingdomHeartsDays::renderer_screenLayout() {
    if (!SingleScreenMode) {
        return screenLayout_Top;
    }

    switch (GameScene) {
        case gameScene_SaveMenu:
        case gameScene_ConfigMenu:
        case gameScene_DayCounter:
        case gameScene_RoxasThoughts:
        case gameScene_InGameWithMap:
        case gameScene_PauseMenu:
        case gameScene_InGameWithDouble3D:
        case gameScene_DeathScreen:
        case gameScene_WorldSelector:
        case gameScene_Other:
            return screenLayout_Top;
        
        case gameScene_IntroLoadMenu:
        case gameScene_Tutorial:
        case gameScene_LoadingScreen:
            return screenLayout_Bottom;
        
        case gameScene_MultiplayerMissionReview:
            return screenLayout_BothVertical;

        case gameScene_Intro:
        case gameScene_TitleScreen:
        case gameScene_InGameMenu:
        case gameScene_Shop:
        case gameScene_TheEnd:
            return screenLayout_BothHorizontal;
        
        case gameScene_Cutscene:
            if (nds->ARM7Read8(getAnyByCart(IS_CREDITS_US, IS_CREDITS_EU, IS_CREDITS_JP, IS_CREDITS_JP_REV1)) == 0x10) {
                return screenLayout_BothHorizontal;
            }
            return (detectTopScreenMobiCutscene() == nullptr) ? screenLayout_BothHorizontal : ((detectBottomScreenMobiCutscene() == nullptr) ? screenLayout_Top : screenLayout_BothHorizontal);
    }

    return screenLayout_Top;
};

int PluginKingdomHeartsDays::renderer_brightnessMode() {
    if (!SingleScreenMode) {
        return brightnessMode_TopScreen;
    }

    if (_ShouldHideScreenForTransitions) {
        return brightnessMode_BlackScreen;
    }
    if (GameScene == gameScene_PauseMenu                ||
        GameScene == gameScene_InGameWithDouble3D       ||
        GameScene == gameScene_MultiplayerMissionReview ||
        GameScene == gameScene_Shop                     ||
        GameScene == gameScene_RoxasThoughts            ||
        GameScene == gameScene_DeathScreen              ||
        GameScene == gameScene_Other) {
        return brightnessMode_TopScreen;
    }
    if (GameScene == gameScene_IntroLoadMenu ||
        GameScene == gameScene_Tutorial      ||
        GameScene == gameScene_LoadingScreen) {
        return brightnessMode_BottomScreen;
    }
    if (GameScene == gameScene_Intro ||
        GameScene == gameScene_TheEnd) {
        return brightnessMode_Horizontal;
    }
    if (GameScene == gameScene_Cutscene) {
        return brightnessMode_Auto;
    }
    if (GameScene == gameScene_WorldSelector) {
        return brightnessMode_Auto;
    }
    return brightnessMode_Default;
}

float PluginKingdomHeartsDays::renderer_forcedAspectRatio()
{
    if (!SingleScreenMode &&
            GameScene != gameScene_InGameWithMap &&
            GameScene != gameScene_DeathScreen &&
            GameScene != gameScene_PauseMenu &&
            GameScene != gameScene_InGameWithDouble3D) {
        return (4.0/3);
    }
    return (GameScene == gameScene_DayCounter) ? (4.0/3) : AspectRatio;
};

bool PluginKingdomHeartsDays::renderer_showOriginalUI() {
    return false;
}

void PluginKingdomHeartsDays::applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress)
{
    bool shouldContinue = _superApplyHotkeyToInputMask(InputMask, HotkeyMask, HotkeyPress);
    if (!shouldContinue) {
        return;
    }

    if (GameScene == -1) {
        return;
    }

    if (GameScene == gameScene_LoadingScreen && FastForwardLoadingScreens) {
        *HotkeyMask |= (1<<4); // Fast Forward (skip loading screen)
    }
}

void PluginKingdomHeartsDays::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* AddonMask, u32* AddonPress)
{
    if (GameScene == -1) {
        return;
    }

    if ((*AddonPress) & (1 << HK_HUDToggle)) {
        hudToggle();
    }
    if ((*AddonPress) & (1 << HK_ReplacementTexturesToggle)) {
        replacementTexturesToggle();
    }
    if ((*AddonPress) & (1 << HK_FullscreenMapToggle)) {
        toggleFullscreenMap();
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithDouble3D) {
        if ((*AddonMask) & (1 << HK_AttackInteract)) {
            *InputMask &= ~(1<<0); // A
        }
        if ((*AddonMask) & (1 << HK_Jump)) {
            *InputMask &= ~(1<<1); // B
        }
        if ((*AddonMask) & (1 << HK_GuardCombo)) {
            *InputMask &= ~(1<<11); // Y
        }

        // Enabling X + D-Pad
        if ((*AddonMask) & ((1 << HK_CommandMenuLeft) | (1 << HK_CommandMenuRight) | (1 << HK_CommandMenuUp) | (1 << HK_CommandMenuDown)))
        {
            u32 dpadMenuAddress = getAnyByCart(INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP,
                                                   INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP_REV1);

            if (nds->ARM7Read8(dpadMenuAddress) & 0x02) {
                nds->ARM7Write8(dpadMenuAddress, nds->ARM7Read8(dpadMenuAddress) - 0x02);
            }
        }

        // So the DS arrow keys can be used to control the command menu
        if ((*AddonMask) & ((1 << HK_CommandMenuLeft) | (1 << HK_CommandMenuRight) | (1 << HK_CommandMenuUp) | (1 << HK_CommandMenuDown)))
        {
            *InputMask &= ~(1<<10); // X
            *InputMask |= (1<<5); // left
            *InputMask |= (1<<4); // right
            *InputMask |= (1<<6); // up
            *InputMask |= (1<<7); // down
            if (PriorPriorAddonMask & (1 << HK_CommandMenuLeft)) // Old D-pad left
                *InputMask &= ~(1<<5); // left
            if (PriorPriorAddonMask & (1 << HK_CommandMenuRight)) // Old D-pad right
                *InputMask &= ~(1<<4); // right
            if (PriorPriorAddonMask & (1 << HK_CommandMenuUp)) // Old D-pad up
                *InputMask &= ~(1<<6); // up
            if (PriorPriorAddonMask & (1 << HK_CommandMenuDown)) // Old D-pad down
                *InputMask &= ~(1<<7); // down
        }

        // R / Lock On
        {
            if ((*AddonMask) & (1 << HK_RLockOn)) {
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
            if ((*AddonMask) & (1 << HK_LSwitchTarget) || (*AddonMask) & (1 << HK_RSwitchTarget)) {
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
        if ((*AddonMask) & (1 << HK_CommandMenuLeft)) { // D-pad left
            *InputMask &= ~(1<<5); // left
        }
        if ((*AddonMask) & (1 << HK_CommandMenuRight)) { // D-pad right
            *InputMask &= ~(1<<4); // right
        }
        if ((*AddonMask) & (1 << HK_CommandMenuUp)) { // D-pad up
            *InputMask &= ~(1<<6); // up
        }
        if ((*AddonMask) & (1 << HK_CommandMenuDown)) { // D-pad down
            *InputMask &= ~(1<<7); // down
        }

        if ((*AddonMask) & (1 << HK_RLockOn)) {
            *InputMask &= ~(1<<8); // R
        }
    }

    PriorPriorAddonMask = PriorAddonMask;
    PriorAddonMask = (*AddonMask);

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;
}

bool PluginKingdomHeartsDays::overrideMouseTouchCoords_cameraControl(int width, int height, int& x, int& y, bool& touching) {
    int centerX = width/2;
    int centerY = height/2;
    float sensitivity = 10.0;

    if (abs(x - centerX) < 3) {
        x = centerX;
    }
    if (abs(y - centerY) < 3) {
        y = centerY;
    }

    if (x == centerX && y == centerY) {
        touching = false;
        x = 128;
        y = 96;
        return true;
    }

    touching = true;
    x = 128 + (int)((x - centerX)*sensitivity);
    y = 96 + (int)((y - centerY)*sensitivity);
    return true;
}
bool PluginKingdomHeartsDays::overrideMouseTouchCoords_singleScreen(int width, int height, int& x, int& y, bool& touching) {
    int X0 = 0;
    int Y0 = 0;
    int X1 = width;
    int Y1 = height;
    int trueWidth = width;
    int trueHeight = height;
    if (AspectRatio * height < width) {
        trueWidth = (int)(AspectRatio * height);
        X0 = (width - trueWidth)/2;
        X1 = X0 + trueWidth;
    }
    else if (width / AspectRatio < height) {
        trueHeight = (int)(width / AspectRatio);
        Y0 = (height - trueHeight)/2;
        Y1 = Y0 + trueHeight;
    }

    if (x < X0 || x > X1 || y < Y0 || y > Y1) {
        return true;
    }

    x = (255*(x - X0))/trueWidth;
    y = (192*(y - Y0))/trueHeight;
    return true;
}
bool PluginKingdomHeartsDays::overrideMouseTouchCoords_horizontalDualScreen(int width, int height, bool invert, int& x, int& y, bool& touching) {
    if (AspectRatio == 0)
    {
        return false;
    }

    int X0 = 0;
    int Y0 = 0;
    int X1 = width;
    int Y1 = height;
    int trueWidth = width;
    int trueHeight = height;
    if (AspectRatio * height < width) {
        trueWidth = (int)(AspectRatio * height);
        X0 = (width - trueWidth)/2;
        X1 = X0 + trueWidth;
    }
    else if (width / AspectRatio < height) {
        trueHeight = (int)(width / AspectRatio);
        Y0 = (height - trueHeight)/2;
        Y1 = Y0 + trueHeight;
    }

    width = trueWidth;
    height = trueHeight;
    trueWidth = trueWidth/2;
    X0 = X0 + (invert ? 0 : trueWidth);
    float innerAspectRatio = 4.0/3;
    if (innerAspectRatio * trueHeight * 2 < width) {
        trueWidth = (int)(innerAspectRatio * trueHeight);
        X0 = X0 + (width - trueWidth*2)/2;
        X1 = X0 + trueWidth;
    }
    else if (trueWidth / innerAspectRatio < height) {
        trueHeight = (int)(trueWidth / innerAspectRatio);
        Y0 = Y0 + (height - trueHeight)/2;
        Y1 = Y0 + trueHeight;
    }

    if (trueWidth == 0 || trueHeight == 0) {
        return false;
    }

    x = (255*(x - X0))/trueWidth;
    y = (191*(y - Y0))/trueHeight;
    if (x < 0 || x > 255 || y < 0 || y > 191) {
        return true;
    }

    return true;
}
bool PluginKingdomHeartsDays::overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching) {
    if (GameScene == gameScene_InGameWithMap) {
#if MOUSE_CURSOR_AS_CAMERA_ENABLED
        return overrideMouseTouchCoords_cameraControl(width, height, x, y, touching);
#endif
    }
    if (renderer_screenLayout() == screenLayout_BothHorizontal) {
        return overrideMouseTouchCoords_horizontalDualScreen(width, height, false, x, y, touching);
    }
    return false;
}

void PluginKingdomHeartsDays::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask)
{
    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithDouble3D) {
        _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, CameraSensitivity, false);
    }
}

bool PluginKingdomHeartsDays::shouldRumble() {
    return false;
}

void PluginKingdomHeartsDays::hudToggle()
{
    HUDState = HUDState + 1;
    if (HUDState >= 3) {
        HUDState = 0;
    }
    if (HUDState == 0) { // exploration mode
        ShowMap = true;
        ShowFullscreenMap = false;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = false;
        HideAllHUD = false;
    }
    else if (HUDState == 1) { // complete mode
        ShowMap = true;
        ShowFullscreenMap = false;
        ShowTarget = true;
        ShowMissionGauge = true;
        ShowMissionInfo = true;
        HideAllHUD = false;
    }
    else { // zero hud
        ShowMap = false;
        ShowFullscreenMap = false;
        ShowTarget = false;
        ShowMissionGauge = false;
        ShowMissionInfo = false;
        HideAllHUD = true;
    }
}

void PluginKingdomHeartsDays::toggleFullscreenMap()
{
    ShowFullscreenMap = !ShowFullscreenMap;
}

const char* PluginKingdomHeartsDays::getGameSceneName()
{
    switch (GameScene) {
        case gameScene_Intro: return "Game scene: Intro";
        case gameScene_TitleScreen: return "Game scene: Title screen";
        case gameScene_IntroLoadMenu: return "Game scene: Intro load menu";
        case gameScene_SaveMenu: return "Game scene: Save menu";
        case gameScene_ConfigMenu: return "Game scene: Config menu";
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
        case gameScene_WorldSelector: return "Game scene: World selector";
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

    bool foundAny = false;
    for (int x = 0; x < 256; x+=4) {
        for (int y = 0; y < 192; y+=4) {
            if (has2DOnTopOf3DAt(buffer, x, y)) {
                foundAny = true;
                u32 color = getPixel(buffer, x, y, 0) & 0xFFFFFF;
                if (!(color == 0 || color == 0x000080 || color == 0x010000 || (color & 0xFFFFE0) == 0x018000)) {
                    return false;
                }
            }
        }
    }

    return foundAny;
}

u32* PluginKingdomHeartsDays::topScreen2DTexture()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    if (GameScene == gameScene_InGameWithDouble3D && nds->PowerControl9 >> 15 == 1) {
        FrontBuffer = FrontBuffer ? 0 : 1;
    }
    return nds->GPU.Framebuffer[FrontBuffer][0].get();
}

u32* PluginKingdomHeartsDays::bottomScreen2DTexture()
{
    int FrontBuffer = nds->GPU.FrontBuffer;
    if (GameScene == gameScene_InGameWithDouble3D && nds->PowerControl9 >> 15 == 1) {
        FrontBuffer = FrontBuffer ? 0 : 1;
    }
    return nds->GPU.Framebuffer[FrontBuffer][1].get();
}

bool PluginKingdomHeartsDays::isBottomScreen2DTextureBlack()
{
    return isBufferBlack(bottomScreen2DTexture());
}

bool PluginKingdomHeartsDays::isDialogVisible()
{
    u32* buffer = topScreen2DTexture();
    for (int y = 161; y >= 141; y--) {
        if (has2DOnTopOf3DAt(buffer, 128, y)) {
            return true;
        }
    }
    return false;
}

bool PluginKingdomHeartsDays::isMinimapVisible() {
    u32 pixel = getPixel(bottomScreen2DTexture(), 99, 53, 0);
    if (GameScene == gameScene_PauseMenu) {
        return ((pixel >> 0) & 0x3F) == 0x1F && ((pixel >> 8) & 0x3F) == 0x1F && ((pixel >> 16) & 0x3F) == 0x1F;
    }
    return ((pixel >> 0) & 0x3F) == 0x3E && ((pixel >> 8) & 0x3F) == 0x3E && ((pixel >> 16) & 0x3F) == 0x3E;
}

bool PluginKingdomHeartsDays::isMissionInformationVisibleOnTopScreen()
{
    u32* buffer = topScreen2DTexture();
    return (has2DOnTopOf3DAt(buffer, 0, 0) && has2DOnTopOf3DAt(buffer, 128, 0) && has2DOnTopOf3DAt(buffer, 254, 0)) ||
           (has2DOnTopOf3DAt(buffer, 0, 8) && has2DOnTopOf3DAt(buffer, 128, 8) && has2DOnTopOf3DAt(buffer, 254, 8));
}

bool PluginKingdomHeartsDays::isMissionInformationVisibleOnBottomScreen()
{
    u32* buffer = bottomScreen2DTexture();
    u32 pixel = getPixel(buffer, 5, 4, 0);
    if (((pixel >> 0) & 0x3F) >= 15 && ((pixel >> 8) & 0x3F) >= 15 && ((pixel >> 16) & 0x3F) >= 15)
    {
        u32 pixel2 = getPixel(buffer, 128, 4, 0);
        if (!(((pixel2 >> 0) & 0x3F) >= 15 && ((pixel2 >> 8) & 0x3F) >= 15 && ((pixel2 >> 16) & 0x3F) >= 15))
        {
            bool onlyBlack = true;
            for (int x = 0; x < 128; x++) {
                u32 pixel3 = getPixel(buffer, x, 16, 0);
                if (!(((pixel3 >> 0) & 0x3F) < 5 && ((pixel3 >> 8) & 0x3F) < 5 && ((pixel3 >> 16) & 0x3F) < 5))
                {
                    onlyBlack = false;
                }
            }
            return !onlyBlack;
        }
    }
    return false;
}

bool PluginKingdomHeartsDays::isMissionGaugeVisibleOnBottomScreen()
{
    u32* buffer = bottomScreen2DTexture();
    bool onlyBlack = true;
    for (int x = 10; x < 128; x++) {
        u32 pixel3 = getPixel(buffer, x, 188, 0);
        if (!(((pixel3 >> 0) & 0x3F) < 5 && ((pixel3 >> 8) & 0x3F) < 5 && ((pixel3 >> 16) & 0x3F) < 5))
        {
            onlyBlack = false;
        }
    }
    return !onlyBlack;
}

#define DIFF(val1, val2) ((val1 > val2) ? (val1 - val2) : (val2 - val1))

bool PluginKingdomHeartsDays::isTargetVisibleOnBottomScreen()
{
    u32* buffer = bottomScreen2DTexture();
    bool onlyGrayscale = true;
    for (int x = 48; x < 88; x++) {
        u32 pixel3 = getPixel(buffer, x, 56, 0);
        u32 r = (pixel3 >> 0) & 0x3F;
        u32 g = (pixel3 >> 8) & 0x3F;
        u32 b = (pixel3 >> 16) & 0x3F;
        if (!((DIFF(r, g) < 5) && (DIFF(r, b) < 5) && (DIFF(g, b) < 5)))
        {
            onlyGrayscale = false;
        }
    }
    return !onlyGrayscale;
}

#undef DIFF(val1, val2)

bool PluginKingdomHeartsDays::isCutsceneFromChallengeMissionVisible()
{
    u32* buffer = topScreen2DTexture();
    return has2DOnTopOf3DAt(buffer, 0,   2) &&  has2DOnTopOf3DAt(buffer, 64,  2) &&
          !has2DOnTopOf3DAt(buffer, 128, 2) && !has2DOnTopOf3DAt(buffer, 192, 2) &&
          !has2DOnTopOf3DAt(buffer, 255, 2) &&
           has2DOnTopOf3DAt(buffer, 0,   4) &&  has2DOnTopOf3DAt(buffer, 64,  4) &&
           has2DOnTopOf3DAt(buffer, 128, 4) &&  has2DOnTopOf3DAt(buffer, 192, 4) &&
           has2DOnTopOf3DAt(buffer, 255, 4) &&
           has2DOnTopOf3DAt(buffer, 0,   6) &&  has2DOnTopOf3DAt(buffer, 64,  6) &&
          !has2DOnTopOf3DAt(buffer, 128, 6) && !has2DOnTopOf3DAt(buffer, 192, 6) &&
          !has2DOnTopOf3DAt(buffer, 255, 6);
}

bool PluginKingdomHeartsDays::isDialogPortraitLabelVisible()
{
    u32 pixel = getPixel(topScreen2DTexture(), 250, 183, 0);
    return ((pixel >> 0) & 0x3F) < 5 && ((pixel >> 8) & 0x3F) < 5 && ((pixel >> 16) & 0x3F) < 5;
}

bool PluginKingdomHeartsDays::isLoadScreenDeletePromptVisible()
{
    u32* buffer = bottomScreen2DTexture();
    u32 pixel1 = getPixel(buffer, 206, 134, 0);
    u32 pixel2 = getPixel(buffer, 206, 140, 0);
    return ((pixel1 >> 0) & 0x3F) < 5 && ((pixel1 >> 8) & 0x3F) < 5 && ((pixel1 >> 16) & 0x3F) < 5 &&
           ((pixel2 >> 0) & 0x3F) < 5 && ((pixel2 >> 8) & 0x3F) < 5 && ((pixel2 >> 16) & 0x3F) < 5;
}

int PluginKingdomHeartsDays::dialogBoxHeight()
{
    u32* buffer = topScreen2DTexture();
    int x = 100;
    int topY = 0;
    int bottomY = 0;
    for (int y = 10; y <= 190; y++) {
        if (has2DOnTopOf3DAt(buffer, x, y)) {
            topY = y;
            break;
        }
    }
    for (int y = 190; y >= 10; y--) {
        if (has2DOnTopOf3DAt(buffer, x, y)) {
            bottomY = y + 1;
            break;
        }
    }
    return bottomY - topY;
}

bool PluginKingdomHeartsDays::has2DOnTopOf3DAt(u32* buffer, int x, int y)
{
    /*
     * If it matches that condition, there is no 2D on top of 3D
        (alphaColor.a == 0x0) ||
        (alphaColor.a == 0x1 && _3dpix.a > 0 && alphaColor.g == 0) ||
        (alphaColor.a == 0x2 && _3dpix.a > 0 && alphaColor.g < 4) ||
        (alphaColor.a == 0x3 && _3dpix.a > 0 && alphaColor.g < 4) ||
        (alphaColor.a == 0x4 && (_3dpix.a & 0x1F) == 0x1F)
    */

    u32 pixel = getPixel(buffer, x, y, 2);
    u32 pixelAlpha = (pixel >> (8*3)) & 0xFF;
    if (pixelAlpha > 0x4) {
        return true;
    }
    if (pixelAlpha == 0x4) {
        return false;
    }
    if (((pixel >> 8) & 0xFF) == 0) {
        return false;
    }

    u32 colorPixel = getPixel(buffer, x, y, 0);
    u32 colorPixelAlpha = (colorPixel >> (8*3)) & 0xFF;
    if (colorPixelAlpha == 0x20) {
        return false;
    }
    if (colorPixelAlpha == 0x04) {
        return false;
    }
    if (colorPixelAlpha == 0x07) {
        return false;
    }
    return true;
}

bool PluginKingdomHeartsDays::shouldRenderFrame()
{
    if (!_superShouldRenderFrame())
    {
        return false;
    }
    if (GameScene == gameScene_InGameWithDouble3D)
    {
        if (!SingleScreenMode) {
            return (nds->PowerControl9 >> 15 == 1);
        }
        return (nds->PowerControl9 >> 15 == 1) ? !ShouldShowBottomScreen : ShouldShowBottomScreen;
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

    u16 mainMenuOrIntroOrLoadMenuVal = nds->ARM7Read16(getAnyByCart(IS_MAIN_MENU_US, IS_MAIN_MENU_EU, IS_MAIN_MENU_JP, IS_MAIN_MENU_JP_REV1));
    bool isMainMenuOrIntroOrLoadMenu = mainMenuOrIntroOrLoadMenuVal == 0x2800 || mainMenuOrIntroOrLoadMenuVal == 0x2C00;
    bool isCutscene = nds->ARM7Read8(getAnyByCart(IS_CUTSCENE_US, IS_CUTSCENE_EU, IS_CUTSCENE_JP, IS_CUTSCENE_JP_REV1)) == 0x03;
    bool isCredits = nds->ARM7Read8(getAnyByCart(IS_CREDITS_US, IS_CREDITS_EU, IS_CREDITS_JP, IS_CREDITS_JP_REV1)) == 0x10;
    bool isUnplayableArea = nds->ARM7Read8(getAnyByCart(IS_PLAYABLE_AREA_US, IS_PLAYABLE_AREA_EU, IS_PLAYABLE_AREA_JP, IS_PLAYABLE_AREA_JP_REV1)) == 0x04;
    bool isLoadMenu = nds->ARM7Read8(getAnyByCart(IS_LOAD_MENU_US, IS_LOAD_MENU_EU, IS_LOAD_MENU_JP, IS_LOAD_MENU_JP_REV1)) != 0;
    bool isDaysCounter = nds->ARM7Read8(getAnyByCart(IS_DAYS_COUNTER_US, IS_DAYS_COUNTER_EU, IS_DAYS_COUNTER_JP, IS_DAYS_COUNTER_JP_REV1)) ==
        getAnyByCart(IS_DAYS_COUNTER_VALUE_US, IS_DAYS_COUNTER_VALUE_EU, IS_DAYS_COUNTER_VALUE_JP, IS_DAYS_COUNTER_VALUE_JP_REV1);
    bool isDeathCounter = nds->ARM7Read8(getAnyByCart(DEATH_SCREEN_ADDRESS_US, DEATH_SCREEN_ADDRESS_EU, DEATH_SCREEN_ADDRESS_JP, DEATH_SCREEN_ADDRESS_JP_REV1)) ==
        getAnyByCart(DEATH_SCREEN_VALUE_US, DEATH_SCREEN_VALUE_EU, DEATH_SCREEN_VALUE_JP, DEATH_SCREEN_VALUE_JP_REV1);
    bool isTutorial = nds->ARM7Read32(getAnyByCart(TUTORIAL_ADDRESS_US, TUTORIAL_ADDRESS_EU, TUTORIAL_ADDRESS_JP, TUTORIAL_ADDRESS_JP_REV1)) != 0;
    bool isPauseScreen = nds->ARM7Read8(getAnyByCart(PAUSE_SCREEN_ADDRESS_US, PAUSE_SCREEN_ADDRESS_EU, PAUSE_SCREEN_ADDRESS_JP, PAUSE_SCREEN_ADDRESS_JP_REV1)) != 0;
    bool isTheEnd = nds->ARM7Read8(getAnyByCart(THE_END_SCREEN_ADDRESS_US, THE_END_SCREEN_ADDRESS_EU, THE_END_SCREEN_ADDRESS_JP, THE_END_SCREEN_ADDRESS_JP_REV1)) == 0x60;

    isCharacterControllable = nds->ARM7Read8(getAnyByCart(IS_CHARACTER_CONTROLLABLE_US, IS_CHARACTER_CONTROLLABLE_EU, IS_CHARACTER_CONTROLLABLE_JP, IS_CHARACTER_CONTROLLABLE_JP_REV1)) == 0x01;

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
                    return gameScene_TitleScreen;
                }
            }

            if (GameScene == gameScene_TitleScreen)
            {
                mayBeMainMenu = nds->GPU.GPU3D.NumVertices < 15 && nds->GPU.GPU3D.NumPolygons < 15;
                if (mayBeMainMenu) {
                    return gameScene_TitleScreen;
                }
            }

            // Main menu
            if (mayBeMainMenu)
            {
                return gameScene_TitleScreen;
            }

            // Intro
            if (GameScene == -1 || GameScene == gameScene_Intro)
            {
                mayBeMainMenu = nds->GPU.GPU3D.NumVertices > 0 && nds->GPU.GPU3D.NumPolygons > 0;
                return mayBeMainMenu ? gameScene_TitleScreen : gameScene_Intro;
            }
        }
    }
    if (!wasSaveLoaded && (GameScene == -1 || GameScene == gameScene_Intro))
    {
        return gameScene_Intro;
    }
    if (isMainMenuOrIntroOrLoadMenu)
    {
        return gameScene_TitleScreen;
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
        u32 mainMenuView = getCurrentMainMenuView();
        if (mainMenuView == 6)
        {
            return gameScene_ConfigMenu;
        }
        if (mainMenuView == 7)
        {
            return gameScene_SaveMenu;
        }
        if (mainMenuView == 8)
        {
            return gameScene_WorldSelector;
        }

        return gameScene_InGameMenu;
    }

    if (nds->GPU.GPU3D.RenderNumPolygons < 20)
    {
        if (isDaysCounter)
        {
            return gameScene_DayCounter;
        }

        if (nds->GPU.GPU2D_B.MasterBrightness == 32784) // TODO: KH Replace with memory detection
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

u32 PluginKingdomHeartsDays::getAspectRatioAddress()
{
    return getAnyByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP, ASPECT_RATIO_ADDRESS_JP_REV1);
}

u32 PluginKingdomHeartsDays::getMobiCutsceneAddress(CutsceneEntry* entry)
{
    return getAnyByCart(entry->usAddress, entry->euAddress, entry->jpAddress, entry->jpAddress - 0x200);
}

CutsceneEntry* PluginKingdomHeartsDays::getMobiCutsceneByAddress(u32 cutsceneAddressValue)
{
    if (cutsceneAddressValue == 0) {
        return nullptr;
    }

    CutsceneEntry* cutscene1 = nullptr;
    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getMobiCutsceneAddress(entry) == cutsceneAddressValue - cutscenesAddressOffset) {
            cutscene1 = entry;
        }
    }

    return cutscene1;
}

u32 PluginKingdomHeartsDays::detectTopScreenMobiCutsceneAddress()
{
    return getAnyByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP, CUTSCENE_ADDRESS_JP_REV1);
}

u32 PluginKingdomHeartsDays::detectBottomScreenMobiCutsceneAddress()
{
    return getAnyByCart(CUTSCENE_ADDRESS_2_US, CUTSCENE_ADDRESS_2_EU, CUTSCENE_ADDRESS_2_JP, CUTSCENE_ADDRESS_2_JP_REV1);
}

bool PluginKingdomHeartsDays::isCutsceneGameScene()
{
    return GameScene == gameScene_Cutscene;
}

bool PluginKingdomHeartsDays::didMobiCutsceneEnded()
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

std::filesystem::path PluginKingdomHeartsDays::patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath) {
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

std::string PluginKingdomHeartsDays::replacementCutsceneFilePath(CutsceneEntry* cutscene) {
    std::string filename = "hd" + std::string(cutscene->MmName) + ".mp4";
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path fullPath = _assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    filename = "hd" + std::string(cutscene->DsName) + ".mp4";
    fullPath = _assetsFolderPath / "cutscenes" / "cinematics" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    if (!KH_15_25_Remix_Location.empty()) {
        std::filesystem::path collectionPath = KH_15_25_Remix_Location;
        std::filesystem::path newEpicFolderPath = collectionPath / "EPIC" / "Mare" / "MOVIE" / "Days" / "en";
        if (std::filesystem::exists(newEpicFolderPath)) {
            std::filesystem::path newEpicFullPath = patchReplacementCutsceneIfNeeded(cutscene, newEpicFolderPath);
            if (newEpicFullPath != "") {
                return newEpicFullPath.string();
            }
        }
        std::filesystem::path newSteamFolderPath = collectionPath / "STEAM" / "Mare" / "MOVIE" / "Days" / "dt";
        if (std::filesystem::exists(newSteamFolderPath)) {
            std::filesystem::path newSteamFullPath = patchReplacementCutsceneIfNeeded(cutscene, newSteamFolderPath);
            if (newSteamFullPath != "") {
                return newSteamFullPath.string();
            }
        }
    }

    return "";
}

bool PluginKingdomHeartsDays::isUnskippableMobiCutscene(CutsceneEntry* cutscene) {
    return isSaveLoaded() && strcmp(cutscene->DsName, "843") == 0;
}

u16 PluginKingdomHeartsDays::getMidiBgmId() {
    return nds->ARM7Read16(getAnyByCart(SONG_ID_ADDRESS_US, SONG_ID_ADDRESS_EU, SONG_ID_ADDRESS_JP, SONG_ID_ADDRESS_JP_REV1));
}

u8 PluginKingdomHeartsDays::getMidiBgmState() {
    u32 SONG_STATE_ADDRESS = getAnyByCart(SONG_ID_ADDRESS_US, SONG_ID_ADDRESS_EU, SONG_ID_ADDRESS_JP, SONG_ID_ADDRESS_JP_REV1) + 0x06;
    // See enum EMidiState for details of the state
    return nds->ARM7Read8(SONG_STATE_ADDRESS);
}

u16 PluginKingdomHeartsDays::getMidiBgmToResumeId() {
    // This byte is set by the audio engine when a "Field" track is getting stopped and we are playing a "Battle" track.
    // This tells us that the "Field" track will resume playing when the "Battle" track ends.
    u32 SONG_SECOND_SLOT_ADDRESS = getAnyByCart(SONG_ID_ADDRESS_US, SONG_ID_ADDRESS_EU, SONG_ID_ADDRESS_JP, SONG_ID_ADDRESS_JP_REV1) + 0x02;
    return nds->ARM7Read8(SONG_SECOND_SLOT_ADDRESS);
}

s16 PluginKingdomHeartsDays::getSongIdInSongTable(u16 bgmId) {
    auto found = std::find_if(BgmEntries.begin(), BgmEntries.end(), [&bgmId](const auto& e) {
        return e.dsId == bgmId; });
    if(found != BgmEntries.end()) {
        return found->loadingTableId;
    }

    return -1;
}

u32 PluginKingdomHeartsDays::getSseqTableAddress()
{
    const u32 songTableAddr = getAnyByCart(SSEQ_TABLE_ADDRESS_US, SSEQ_TABLE_ADDRESS_EU, SSEQ_TABLE_ADDRESS_JP, SSEQ_TABLE_ADDRESS_JP_REV1);
    const u32 songTableExpectedRefVal = getAnyByCart(0x020dd668, 0x020de448, 0x020dc7c8, 0x020dc748);
    const u32 songTableRefVal = nds->ARM9Read32(songTableAddr - 0x20);

    if (songTableRefVal != songTableExpectedRefVal) // patched rom support
    {
        for (s32 offset = -0x100; offset <= 0x100; offset += 0x4) {
            if (nds->ARM9Read32(songTableAddr + offset - 0x20) == songTableExpectedRefVal)
            {
                return songTableAddr + offset;
            }
        }
    }
    return songTableAddr;
}

u32 PluginKingdomHeartsDays::getMidiSequenceAddress(u16 bgmId) {
    const u32 songTableAddr = getSseqTableAddress();

    s16 idInTable = getSongIdInSongTable(bgmId);
    if (idInTable >= 0) {
        const u32 entryAddr = songTableAddr + (idInTable * 16) + 4;
        return nds->ARM9Read32(entryAddr);
    }

    return 0;
}

u16 PluginKingdomHeartsDays::getMidiSequenceSize(u16 bgmId) {
    const u32 songTableAddr = getSseqTableAddress();

    s16 idInTable = getSongIdInSongTable(bgmId);
    if (idInTable >= 0) {
        const u32 songSizeAddr = songTableAddr + (idInTable * 16);
        return nds->ARM9Read32(songSizeAddr);
    }

    return 0;
}

u32 PluginKingdomHeartsDays::getStreamBgmAddress() {
    return getAnyByCart(STRM_ADDRESS_US, STRM_ADDRESS_EU, STRM_ADDRESS_JP, STRM_ADDRESS_JP_REV1);
}

u16 PluginKingdomHeartsDays::getStreamBgmCustomIdFromDsId(u8 dsId, u32 numSamples) {
    auto found = std::find_if(StreamedBgmEntries.begin(), StreamedBgmEntries.end(), [&dsId, &numSamples](const auto& e) {
        return e.dsId == dsId && e.numSamples == numSamples; });
    if(found != StreamedBgmEntries.end()) {
        return found->customId;
    }

    return BGM_INVALID_ID;
}


u8 PluginKingdomHeartsDays::getMidiBgmVolume() {
    u32 SONG_MASTER_VOLUME_ADDRESS = getAnyByCart(SONG_ID_ADDRESS_US, SONG_ID_ADDRESS_EU, SONG_ID_ADDRESS_JP, SONG_ID_ADDRESS_JP_REV1) + 0x07;
    // Usually 0x7F during gameplay and 0x40 when game is paused
    return nds->ARM7Read8(SONG_MASTER_VOLUME_ADDRESS);
}

u32 PluginKingdomHeartsDays::getBgmFadeOutDuration() {
    // Caution: this RAM value should be queried on the first frame when the "Stopping" state was set, otherwise fadeout is already in progress!
    u32 SONG_FADE_OUT_PROGRESS_ADDRESS = getAnyByCart(SONG_ID_ADDRESS_US, SONG_ID_ADDRESS_EU, SONG_ID_ADDRESS_JP, SONG_ID_ADDRESS_JP_REV1) + 0x0A;
    u8 progress = nds->ARM7Read8(SONG_FADE_OUT_PROGRESS_ADDRESS);
    // converts value in game frames to milliseconds + some smoothing
    float valueMs = (progress / 30.0f);
    if (progress >= 59) { valueMs *= 1.2; }
    else if (progress >= 29) { valueMs *= 1.5; }
    else { valueMs *= 2; }

    return std::round(valueMs * 1000);
}

std::string PluginKingdomHeartsDays::getBackgroundMusicName(u16 bgmId) {
    auto found = std::find_if(BgmEntries.begin(), BgmEntries.end(), [&bgmId](const auto& e) {
        return e.dsId == bgmId; });
    if(found != BgmEntries.end()) {
        return found->Name;
    }

    return "Unknown";
}

int PluginKingdomHeartsDays::delayBeforeStartReplacementBackgroundMusic(u16 bgmId) {
    // Delay patch only required with HD cutscene replacement
    if (_RunningReplacementCutscene) {
        if (bgmId == 22 || bgmId == 39) {
            if (CutsceneEntry* topCutscene = detectTopScreenMobiCutscene()) {
                std::string cutsceneId(topCutscene->DsName);
                if (bgmId == 22 && cutsceneId == "848") {
                    // Delay for "Musique pour la tristesse de Xion" during the "Xion's End" cutscene
                    return 12800;
                } else if (bgmId == 39 && cutsceneId == "843") {
                    // Delay for "Dearly Beloved (Reprise)" at the end of "End credits"
                    return 376000;
                }
            }
        }
    }
    return 0;
}

void PluginKingdomHeartsDays::refreshMouseStatus() {
#if !MOUSE_CURSOR_AS_CAMERA_ENABLED
    return;
#endif
    if (GameScene == gameScene_InGameWithMap && !_MouseCursorIsGrabbed) {
        _ShouldGrabMouseCursor = true;
    }
    if (PriorGameScene == gameScene_InGameWithMap && PriorGameScene != GameScene && _MouseCursorIsGrabbed) {
        _ShouldReleaseMouseCursor = true;
    }
}

std::string PluginKingdomHeartsDays::localizationFilePath(std::string language) {
    std::string filename = language + ".ini";
    std::string assetsRegionSubfolderName = assetsRegionSubfolder();
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path fullPath = _assetsFolderPath / "localization" / assetsRegionSubfolderName / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    return "";
}

u32 PluginKingdomHeartsDays::getCurrentMission()
{
    return nds->ARM7Read8(getAnyByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
}

// The states below also happen in multiple other places outside the main menu menus
// 0 -> none
// 1 -> main menu root
// 2 -> panel
// 3 -> roxas's diary
// 4 -> enemy profile
// 5 -> tutorials
// 6 -> config (replaced by gameScene_ConfigMenu)
// 7 -> save (replaced by gameScene_SaveMenu)
// 8 -> world selector (replaced by gameScene_WorldSelector)
// 9 -> character selection
// 10 -> mission review
u32 PluginKingdomHeartsDays::getCurrentMainMenuView()
{
    if (GameScene == -1)
    {
        return 0;
    }

    u8 val1 = nds->ARM7Read8(getAnyByCart(CURRENT_INGAME_MENU_VIEW_1_US, CURRENT_INGAME_MENU_VIEW_1_EU, CURRENT_INGAME_MENU_VIEW_1_JP, CURRENT_INGAME_MENU_VIEW_1_JP_REV1));
    u8 val2 = nds->ARM7Read8(getAnyByCart(CURRENT_INGAME_MENU_VIEW_2_US, CURRENT_INGAME_MENU_VIEW_2_EU, CURRENT_INGAME_MENU_VIEW_2_JP, CURRENT_INGAME_MENU_VIEW_2_JP_REV1));
    if (val1 == 0x10 && val2 == 0x01) {
        return 1; // main menu root
    }
    if (val1 == 0x02 && val2 == 0x10) {
        return 2; // panel
    }
    if (val1 == 0x01 && val2 == 0x02) {
        // TODO: That specific part probably can be improved
        u8 val3 = nds->ARM7Read8(getAnyByCart(CURRENT_INGAME_MENU_VIEW_US, CURRENT_INGAME_MENU_VIEW_EU, CURRENT_INGAME_MENU_VIEW_JP, CURRENT_INGAME_MENU_VIEW_JP_REV1));
        if (val3 == 0x07) return 4; // roxas's diary / enemy profile
        if (val3 == 0x06) return 5; // tutorials
        return 6; // config
    }
    if (val1 == 0x02 && val2 == 0x01) {
        return 7; // save
    }
    if (val1 == 0x20 && val2 == 0x40) {
        return 8; // world selector
    }
    if (val1 == 0x03 && val2 == 0x70) {
        return 9; // character selection
    }
    if (val1 == 0x01 && val2 == 0x10) {
        return 10; // mission review
    }

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

    u8 world = nds->ARM7Read8(getAnyByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    u8 map = nds->ARM7Read8(getAnyByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
    u32 fullMap = world;
    fullMap = (fullMap << 4*2) | map;

    if (Map != fullMap) {
        priorMap = Map;
        Map = fullMap;
    }

    if (Map == 128) { // cutscene; TODO: IDK if that still applies, or even if supporting that is still relevant
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

    printf("Game scene: %d\n", GameScene);
    printf("Game scene state: %d\n", GameSceneState);
    printf("Current map: %d\n", getCurrentMap());
    printf("Current main menu view: %d\n", getCurrentMainMenuView());
    printf("Is save loaded: %d\n", isSaveLoaded() ? 1 : 0);
    printf("\n");
}

}
