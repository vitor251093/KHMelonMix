#include "PluginKingdomHeartsReCoded.h"

#include "PluginKingdomHeartsReCoded_GPU_OpenGL_shaders.h"
#include "PluginKingdomHeartsReCoded_GPU3D_OpenGL_shaders.h"

namespace Plugins
{

u32 PluginKingdomHeartsReCoded::usGamecode = 1161382722;
u32 PluginKingdomHeartsReCoded::euGamecode = 1345932098;
u32 PluginKingdomHeartsReCoded::jpGamecode = 1245268802;

#define ASPECT_RATIO_ADDRESS_US 0x0202A810
#define ASPECT_RATIO_ADDRESS_EU 0x0202A824
#define ASPECT_RATIO_ADDRESS_JP 0x0202A728

// 0x00 => intro and main menu
#define IS_MAIN_MENU_US 0x02060c94
#define IS_MAIN_MENU_EU 0x0205fdc4
#define IS_MAIN_MENU_JP 0x02060aa0

#define PAUSE_SCREEN_ADDRESS_US 0x020569d0
#define PAUSE_SCREEN_ADDRESS_EU 0x020569d0
#define PAUSE_SCREEN_ADDRESS_JP 0x020567f0

#define DIALOG_SCREEN_ADDRESS_US 0x0219e9a8
#define DIALOG_SCREEN_ADDRESS_EU 0x0219e9c8
#define DIALOG_SCREEN_ADDRESS_JP 0x0219e9a8 // TODO: KH

#define DIALOG_SCREEN_VALUE_US 0x00000000
#define DIALOG_SCREEN_VALUE_EU 0x00000000
#define DIALOG_SCREEN_VALUE_JP 0x00000000 // TODO: KH

#define PAUSE_SCREEN_VALUE_TRUE_PAUSE 0x01

// 0x03 => cutscene; 0x01 => not cutscene
#define IS_CUTSCENE_US 0x02056e90
#define IS_CUTSCENE_EU 0x02056e90
#define IS_CUTSCENE_JP 0x02056cb0

// 0x01 => cutscene with skip button, 0x03 => regular cutscene, 0x08 => cutscene with static images, 0x10 => in-game, main menu
#define GAME_STATE_ADDRESS_US 0x02056f4a
#define GAME_STATE_ADDRESS_EU 0x02056f4a
#define GAME_STATE_ADDRESS_JP 0x02056d6a

// 0x04 => playable (example: ingame); 0x03 => world selection; 0x02 => not playable (menus)
#define IS_PLAYABLE_AREA_US 0x0205a8c0
#define IS_PLAYABLE_AREA_EU 0x0205a8c0
#define IS_PLAYABLE_AREA_JP 0x0205a6e0

#define TYPE_OF_BATTLE_ADDRESS_US 0x020b5608 // or 0x020b5620
#define TYPE_OF_BATTLE_ADDRESS_EU 0x020b5608 // TODO: KH
#define TYPE_OF_BATTLE_ADDRESS_JP 0x020b5608 // TODO: KH

#define CUTSCENE_ADDRESS_US 0x020b7db8
#define CUTSCENE_ADDRESS_EU 0x020b7e08
#define CUTSCENE_ADDRESS_JP 0x020b7858

#define MINIMAP_1_CENTER_X_ADDRESS_US 0x023d8054
#define MINIMAP_1_CENTER_X_ADDRESS_EU 0x023d8054 // TODO: KH
#define MINIMAP_1_CENTER_X_ADDRESS_JP 0x023d8054 // TODO: KH

#define MINIMAP_1_CENTER_Y_ADDRESS_US 0x023d8058
#define MINIMAP_1_CENTER_Y_ADDRESS_EU 0x023d8058 // TODO: KH
#define MINIMAP_1_CENTER_Y_ADDRESS_JP 0x023d8058 // TODO: KH

#define MINIMAP_2_CENTER_X_ADDRESS_US 0x023d6c68
#define MINIMAP_2_CENTER_X_ADDRESS_EU 0x023d6c68 // TODO: KH
#define MINIMAP_2_CENTER_X_ADDRESS_JP 0x023d6c68 // TODO: KH

#define MINIMAP_2_CENTER_Y_ADDRESS_US 0x023d6c6c
#define MINIMAP_2_CENTER_Y_ADDRESS_EU 0x023d6c6c // TODO: KH
#define MINIMAP_2_CENTER_Y_ADDRESS_JP 0x023d6c6c // TODO: KH

#define MINIMAP_3_CENTER_X_ADDRESS_US 0x023d8081
#define MINIMAP_3_CENTER_X_ADDRESS_EU 0x023d8081 // TODO: KH
#define MINIMAP_3_CENTER_X_ADDRESS_JP 0x023d8081 // TODO: KH

#define MINIMAP_3_CENTER_Y_ADDRESS_US 0x023d8085
#define MINIMAP_3_CENTER_Y_ADDRESS_EU 0x023d8085 // TODO: KH
#define MINIMAP_3_CENTER_Y_ADDRESS_JP 0x023d8085 // TODO: KH

#define MINIMAP_4_CENTER_X_ADDRESS_US 0x023c6a94
#define MINIMAP_4_CENTER_X_ADDRESS_EU 0x023c6a94 // TODO: KH
#define MINIMAP_4_CENTER_X_ADDRESS_JP 0x023c6a94 // TODO: KH

#define MINIMAP_4_CENTER_Y_ADDRESS_US 0x023c6a98
#define MINIMAP_4_CENTER_Y_ADDRESS_EU 0x023c6a98 // TODO: KH
#define MINIMAP_4_CENTER_Y_ADDRESS_JP 0x023c6a98 // TODO: KH

#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US 0x02198310
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU 0x021991b0
#define INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP 0x02198310 // TODO: KH

#define INGAME_MENU_COMMAND_LIST_SETTING_VALUE_US 0x200
#define INGAME_MENU_COMMAND_LIST_SETTING_VALUE_EU 0x200
#define INGAME_MENU_COMMAND_LIST_SETTING_VALUE_JP 0x002 // TODO: KH

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
    gameScene_InGameMenu,               // 6
    gameScene_InGameSaveMenu,           // 7
    gameScene_PauseMenu,                // 8
    gameScene_Tutorial,                 // 9
    gameScene_InGameWithDouble3D,       // 10
    gameScene_Shop,                     // 11
    gameScene_LoadingScreen,            // 12
    gameScene_CutsceneWithStaticImages, // 13
    gameScene_WorldSelection,           // 14
    gameScene_InGameDialog,             // 15
    gameScene_InGameOlympusBattle,      // 16
    gameScene_Other2D,                  // 17
    gameScene_Other                     // 18
};

enum
{
    HK_HUDToggle,
    HK_RLockOn,
    HK_LSwitchTarget,
    HK_RSwitchTarget,
    HK_CommandMenuLeft,
    HK_CommandMenuRight,
    HK_CommandMenuUp,
    HK_CommandMenuDown
};

PluginKingdomHeartsReCoded::PluginKingdomHeartsReCoded(u32 gameCode)
{
    GameCode = gameCode;

    hudToggle();

    priorMap = -1;
    Map = 0;
    UIScale = 4;

    // game scene detection utils (extra: minimap coordinates)
    MinimapCenterX = 128;
    MinimapCenterY = 96;

    // game scene detection utils
    _muchOlderHad3DOnTopScreen = false;
    _muchOlderHad3DOnBottomScreen = false;
    _olderHad3DOnTopScreen = false;
    _olderHad3DOnBottomScreen = false;
    _had3DOnTopScreen = false;
    _had3DOnBottomScreen = false;

    // apply addon to input mask utils
    for (int i = 0; i < PRIOR_ADDON_MASK_SIZE; i++) {
        PriorAddonMask[i] = 0;
    }
    LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;

    customKeyMappingNames = {
        "HK_HUDToggle",
        "HK_RLockOn",
        "HK_LSwitchTarget",
        "HK_RSwitchTarget",
        "HK_CommandMenuLeft",
        "HK_CommandMenuRight",
        "HK_CommandMenuUp",
        "HK_CommandMenuDown"
    };
    customKeyMappingLabels = {
        "[KH] HUD Toggle",
        "[KH] (R1) R / Lock On",
        "[KH] (L2) Switch Target",
        "[KH] (R2) Switch Target",
        "[KH] Command Menu - Left",
        "[KH] Command Menu - Right",
        "[KH] Command Menu - Up",
        "[KH] Command Menu - Down"
    };

    Cutscenes = std::array<Plugins::CutsceneEntry, 15> {{
        {"OP",     "501",         "501_",                       0x04bb3a00, 0x04c1be00, 0x04b04200, 2+1},
        {"Secret", "593",         "593_",                       0x05e9b400, 0x05f03800, 0x05db0200, 2},
        {"w1_ED",  "510_PLUS_mm", "510_unaccountable_accounts", 0x06784800, 0x067ecc00, 0x06699e00, 2},
        {"w1_OP",  "502",         "502_",                       0x06e43800, 0x06eabc00, 0x06d58e00, 4+2},
        {"w2_ED",  "510_PLUS_mm", "510_",                       0x07c4ee00, 0x07cb7200, 0x07b64400, 2},
        {"w2_OP",  "512_PLUS_mm", "512_",                       0x08548600, 0x085b0a00, 0x0845f800, 2},
        {"w3_ED",  "524",         "524_",                       0x08706200, 0x0876e600, 0x0861D400, 2},
        {"w4_ED",  "531",         "531_",                       0x09503000, 0x0956b400, 0x0941a200, 2},
        {"w5_ED",  "539_PLUS_mm", "539_",                       0x09990800, 0x099f8c00, 0x098a7a00, 2},
        {"w6_ED",  "549",         "549_",                       0x0a3c9400, 0x0a431800, 0x0a2e0600, 2},
        {"w7_ED",  "572",         "572_",                       0x0ac3aa00, 0x0aca2e00, 0x0ab51c00, 2},
        {"w8_ED1", "590_PLUS_mm", "590_",                       0x0b150400, 0x0b1b8800, 0x0b067600, 2},
        {"w8_ED2", "592",         "592_",                       0x0bcd3400, 0x0bd3b800, 0x0bbea600, 2},
        {"w8_ED3", "576_PLUS_mm", "576_",                       0x0c216800, 0x0c27ec00, 0x0c12cc00, 2},
        {"w8_OP",  "573_PLUS_mm", "573_",                       0x0c426000, 0x0c48e400, 0x0c33c400, 2},
    }};
}

void PluginKingdomHeartsReCoded::loadLocalization() {
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
    else if (false) {
        int firstAddr = 0;
        int lastAddr = 0;
        bool validCharFound = false;
        bool forbCharFound = false;
        for (int addr = 0x06A66638; addr < 0x06C49D0C; addr++) { // TODO: KH Those are the Days addresses
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

void PluginKingdomHeartsReCoded::onLoadROM() {
    loadLocalization();

    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
}

std::string PluginKingdomHeartsReCoded::assetsFolder() {
    return "recoded";
}

std::string PluginKingdomHeartsReCoded::tomlUniqueIdentifier() {
    return getStringByCart("KHReCoded_US", "KHReCoded_EU", "KHReCoded_JP");
}

const char* PluginKingdomHeartsReCoded::gpuOpenGL_FS() {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kCompositorFS_KhReCoded;
};

void PluginKingdomHeartsReCoded::gpuOpenGL_FS_initVariables(GLuint CompShader) {
    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "PriorGameScene");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "GameScene");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "KHUIScale");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "TopScreenAspectRatio");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "ShowMap");
    CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "MinimapCenterX");
    CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "MinimapCenterY");
    CompGpuLoc[CompShader][7] = glGetUniformLocation(CompShader, "HideAllHUD");
    CompGpuLoc[CompShader][8] = glGetUniformLocation(CompShader, "DSCutsceneState");
}

void PluginKingdomHeartsReCoded::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    CutsceneEntry* tsCutscene = detectTopScreenMobiCutscene();
    int dsCutsceneState = (tsCutscene == nullptr ? 0 : tsCutscene->dsScreensState);

    glUniform1i(CompGpuLoc[CompShader][0], PriorGameScene);
    glUniform1i(CompGpuLoc[CompShader][1], GameScene);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1f(CompGpuLoc[CompShader][3], aspectRatio);
    glUniform1i(CompGpuLoc[CompShader][4], ShowMap ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][5], MinimapCenterX);
    glUniform1i(CompGpuLoc[CompShader][6], MinimapCenterY);
    glUniform1i(CompGpuLoc[CompShader][7], HideAllHUD ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][8], dsCutsceneState);
}

const char* PluginKingdomHeartsReCoded::gpu3DOpenGLClassic_VS_Z() {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kRenderVS_Z_KhReCoded;
};

void PluginKingdomHeartsReCoded::gpu3DOpenGLClassic_VS_Z_initVariables(GLuint prog, u32 flags)
{
    CompGpu3DLoc[flags][0] = glGetUniformLocation(prog, "TopScreenAspectRatio");
    CompGpu3DLoc[flags][1] = glGetUniformLocation(prog, "GameScene");
    CompGpu3DLoc[flags][2] = glGetUniformLocation(prog, "KHUIScale");
}

void PluginKingdomHeartsReCoded::gpu3DOpenGLClassic_VS_Z_updateVariables(u32 flags)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);
    glUniform1f(CompGpu3DLoc[flags][0], aspectRatio);
    glUniform1i(CompGpu3DLoc[flags][1], GameScene);
    glUniform1i(CompGpu3DLoc[flags][2], UIScale);
}

void PluginKingdomHeartsReCoded::gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32 scaledPositions[10][2], melonDS::Polygon* polygon) {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return;
    }

    float aspectRatio = AspectRatio / (4.f / 3.f);
    u32 attr = polygon->Attr;

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_PauseMenu) {
        u32 aimAttr1 = 1058996416;
        u32 aimAttr2 = 1042219200;
        u32 greenAimSmallSquare = 1025441984;
        u32 greenAimBigSquare = 2033856;
        if (polygon->NumVertices == 4 && (attr == aimAttr1 || attr == aimAttr2 || attr == greenAimSmallSquare || attr == greenAimBigSquare)) {
            s32 z = polygon->Vertices[0]->Position[2];
            float _z = ((float)z)/(1 << 22);
            if (_z < 0) {
                u32 x0 = std::min({(int)scaledPositions[0][0], (int)scaledPositions[1][0], (int)scaledPositions[2][0], (int)scaledPositions[3][0]});
                u32 x1 = std::max({(int)scaledPositions[0][0], (int)scaledPositions[1][0], (int)scaledPositions[2][0], (int)scaledPositions[3][0]});
                float xCenter = (x0 + x1)/2.0;

                scaledPositions[0][0] = (u32)(xCenter + (s32)(((float)scaledPositions[0][0] - xCenter)/aspectRatio));
                scaledPositions[1][0] = (u32)(xCenter + (s32)(((float)scaledPositions[1][0] - xCenter)/aspectRatio));
                scaledPositions[2][0] = (u32)(xCenter + (s32)(((float)scaledPositions[2][0] - xCenter)/aspectRatio));
                scaledPositions[3][0] = (u32)(xCenter + (s32)(((float)scaledPositions[3][0] - xCenter)/aspectRatio));
            }
        }
    }

    for (int vertexIndex = 0; vertexIndex < polygon->NumVertices; vertexIndex++)
    {
        s32* x = &scaledPositions[vertexIndex][0];
        s32* y = &scaledPositions[vertexIndex][1];
        s32 z = polygon->Vertices[vertexIndex]->Position[2];
        s32* rgb = polygon->Vertices[vertexIndex]->FinalColor;

        int resolutionScale = ScreenWidth/256;
        float commandMenuLeftMargin = 6.7;
        float commandMenuBottomMargin = 0.5;

        float iuTexScale = (6.0)/UIScale;

        float _x = (float)(*x);
        float _y = (float)(*y);
        float _z = ((float)z)/(1 << 22);

        if (HideAllHUD)
        {
            if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_PauseMenu || GameScene == gameScene_InGameOlympusBattle)
            {
                if (_x >= 0 && _x <= ScreenWidth &&
                    _y >= 0 && _y <= ScreenHeight &&
                    _z < 0) {
                    _x = 0;
                    _y = 0;
                }
            }
        }
        else
        {
            if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_PauseMenu || GameScene == gameScene_InGameOlympusBattle)
            {
                if (_x >= 0 && _x <= (5.0/16)*(ScreenWidth) &&
                    _y >= (1.0/8)*(ScreenHeight) && _y <= (ScreenHeight) &&
                    _z == (s32)(-1.000) &&
                    rgb[0] < 200) {

                    _x = (_x)/(iuTexScale*aspectRatio) + commandMenuLeftMargin*resolutionScale;
                    _y = ScreenHeight - ((ScreenHeight - _y)/(iuTexScale)) - commandMenuBottomMargin*resolutionScale;
                }
            }

            if (GameScene == gameScene_InGameOlympusBattle)
            {
                if (_x >= (ScreenWidth/2)        && _x <= (ScreenWidth)  &&
                    _y >= (2.0/3)*(ScreenHeight) && _y <= (ScreenHeight)) {

                    _x = ScreenWidth - ((ScreenWidth - _x)/(iuTexScale*aspectRatio));
                    _y = ScreenHeight - ((ScreenHeight - _y)/(iuTexScale));
                }
            }
        }

        *x = (s32)(_x);
        *y = (s32)(_y);
    }
};

void PluginKingdomHeartsReCoded::onLoadState()
{
    texturesIndex.clear();

    loadLocalization();

    GameScene = gameScene_InGameWithMap;
}

void PluginKingdomHeartsReCoded::applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress)
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
}

void PluginKingdomHeartsReCoded::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* AddonMask, u32* AddonPress)
{
    if (GameScene == -1) {
        return;
    }

    if ((*AddonPress) & (1 << HK_HUDToggle)) {
        hudToggle();
    }

    if (GameScene == gameScene_InGameWithMap || GameScene == gameScene_InGameWithDouble3D || GameScene == gameScene_InGameOlympusBattle) {
        // Enabling L + D-Pad
        if ((*AddonMask) & ((1 << HK_CommandMenuLeft) | (1 << HK_CommandMenuRight) | (1 << HK_CommandMenuUp) | (1 << HK_CommandMenuDown)))
        {
            u32 dpadMenuAddress = getU32ByCart(INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_US,
                                               INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_EU,
                                               INGAME_MENU_COMMAND_LIST_SETTING_ADDRESS_JP);
            u32 controlTypeOffset = getU32ByCart(INGAME_MENU_COMMAND_LIST_SETTING_VALUE_US,
                                                 INGAME_MENU_COMMAND_LIST_SETTING_VALUE_EU,
                                                 INGAME_MENU_COMMAND_LIST_SETTING_VALUE_JP);

            if ((nds->ARM7Read32(dpadMenuAddress) & controlTypeOffset) == 0) {
                nds->ARM7Write32(dpadMenuAddress, nds->ARM7Read32(dpadMenuAddress) | controlTypeOffset);
            }
        }

        // So the arrow keys can be used to control the command menu
        if ((*AddonMask) & ((1 << HK_CommandMenuLeft) | (1 << HK_CommandMenuRight) | (1 << HK_CommandMenuUp) | (1 << HK_CommandMenuDown)))
        {
            *InputMask &= ~(1<<9); // L
            *InputMask |= (1<<5); // left
            *InputMask |= (1<<4); // right
            *InputMask |= (1<<6); // up
            *InputMask |= (1<<7); // down
            if (PriorAddonMask[1] & (1 << HK_CommandMenuLeft)) // Old D-pad left
                *InputMask &= ~(1<<5); // left
            if (PriorAddonMask[1] & (1 << HK_CommandMenuRight)) // Old D-pad right
                *InputMask &= ~(1<<4); // right
            if (PriorAddonMask[1] & (1 << HK_CommandMenuUp)) // Old D-pad up
                *InputMask &= ~(1<<6); // up
            if (PriorAddonMask[1] & (1 << HK_CommandMenuDown)) // Old D-pad down
                *InputMask &= ~(1<<7); // down
        }

        // R / Lock On
        {
            if ((*AddonMask) & (1 << HK_RLockOn)) {
                *InputMask &= ~(1<<8); // R
                *InputMask &= ~(1<<9); // L
            }
        }

        // Switch Target
        {
            if ((*AddonMask) & (1 << HK_LSwitchTarget)) {
                *InputMask &= ~(1<<5); // left
                *InputMask &= ~(1<<8); // R
                *InputMask &= ~(1<<9); // L
            }

            if ((*AddonMask) & (1 << HK_RSwitchTarget)) {
                *InputMask &= ~(1<<4); // right
                *InputMask &= ~(1<<8); // R
                *InputMask &= ~(1<<9); // L
            }
        }
    }
    else {
        // So the arrow keys can be used as directionals
        if ((*AddonMask) & (1 << HK_CommandMenuLeft)) {
            *InputMask &= ~(1<<5); // left
        }
        if ((*AddonMask) & (1 << HK_CommandMenuRight)) {
            *InputMask &= ~(1<<4); // right
        }
        if ((*AddonMask) & (1 << HK_CommandMenuUp)) {
            *InputMask &= ~(1<<6); // up
        }
        if ((*AddonMask) & (1 << HK_CommandMenuDown)) {
            *InputMask &= ~(1<<7); // down
        }

        if ((*AddonMask) & (1 << HK_RLockOn)) {
            *InputMask &= ~(1<<8); // R
        }
    }

    if (GameScene == gameScene_InGameMenu) {
        // Toggle screens
        {
            bool clear = false;
            for (int i = PRIOR_ADDON_MASK_SIZE - 1; i >= 0; i--) {
                if (clear) {
                    PriorAddonMask[i] = PriorAddonMask[i] & ~((1<<2) | (1<<3));
                }
                if (PriorAddonMask[i] & (1 << 2) || PriorAddonMask[i] & (1 << 3)) {
                    clear = true;
                }
            }

            {
                if (PriorAddonMask[10] & (1 << HK_LSwitchTarget) || PriorAddonMask[9] & (1 << HK_LSwitchTarget) || PriorAddonMask[8] & (1 << HK_LSwitchTarget)) {
                    *InputMask &= ~(1<<10); // X
                }
                else if (PriorAddonMask[6] & (1 << HK_LSwitchTarget) || PriorAddonMask[5] & (1 << HK_LSwitchTarget) || PriorAddonMask[4] & (1 << HK_LSwitchTarget)) {
                    *InputMask &= ~(1<<9); // L
                }
                else if (PriorAddonMask[2] & (1 << HK_LSwitchTarget) || PriorAddonMask[1] & (1 << HK_LSwitchTarget) || PriorAddonMask[0] & (1 << HK_LSwitchTarget)) {
                    *InputMask &= ~(1<<10); // X
                }
            }

            {
                if (PriorAddonMask[10] & (1 << HK_RSwitchTarget) || PriorAddonMask[9] & (1 << HK_RSwitchTarget) || PriorAddonMask[8] & (1 << HK_RSwitchTarget)) {
                    *InputMask &= ~(1<<10); // X
                }
                else if (PriorAddonMask[6] & (1 << HK_RSwitchTarget) || PriorAddonMask[5] & (1 << HK_RSwitchTarget) || PriorAddonMask[4] & (1 << HK_RSwitchTarget)) {
                    *InputMask &= ~(1<<8); // R
                }
                else if (PriorAddonMask[2] & (1 << HK_RSwitchTarget) || PriorAddonMask[1] & (1 << HK_RSwitchTarget) || PriorAddonMask[0] & (1 << HK_RSwitchTarget)) {
                    *InputMask &= ~(1<<10); // X
                }
            }
        }
    }

    for (int i = PRIOR_ADDON_MASK_SIZE - 1; i > 0; i--) {
        PriorAddonMask[i] = PriorAddonMask[i - 1];
    }
    PriorAddonMask[0] = (*AddonMask);

    if (LastSwitchTargetPress < SWITCH_TARGET_PRESS_FRAME_LIMIT) LastSwitchTargetPress++;
    if (LastLockOnPress < LOCK_ON_PRESS_FRAME_LIMIT) LastLockOnPress++;
}

bool PluginKingdomHeartsReCoded::overrideMouseTouchCoords_singleScreen(int width, int height, int& x, int& y, bool& touching) {
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
bool PluginKingdomHeartsReCoded::overrideMouseTouchCoords_horizontalDualScreen(int width, int height, bool invert, int& x, int& y, bool& touching) {
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

    x = (255*(x - X0))/trueWidth;
    y = (191*(y - Y0))/trueHeight;
    if (x < 0 || x > 255 || y < 0 || y > 191) {
        return true;
    }

    return true;
}
bool PluginKingdomHeartsReCoded::overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching) {
    if (GameScene == gameScene_InGameMenu) {
        return overrideMouseTouchCoords_horizontalDualScreen(width, height, false, x, y, touching);
    }
    return false;
}

void PluginKingdomHeartsReCoded::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask)
{
    if (GameScene == -1)
    {
        return;
    }

    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, 3, true);
}

void PluginKingdomHeartsReCoded::hudToggle()
{
    HUDState = (HUDState + 1) % 3;
    if (HUDState == 0) { // map mode
        ShowMap = true;
        HideAllHUD = false;
    }
    else if (HUDState == 1) { // no map mode
        ShowMap = false;
        HideAllHUD = false;
    }
    else { // zero hud
        ShowMap = false;
        HideAllHUD = true;
    }
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
        case gameScene_PauseMenu: return "Game scene: Pause menu";
        case gameScene_Tutorial: return "Game scene: Tutorial";
        case gameScene_InGameWithDouble3D: return "Game scene: Ingame (with cutscene)";
        case gameScene_Shop: return "Game scene: Shop";
        case gameScene_LoadingScreen: return "Game scene: Loading screen";
        case gameScene_CutsceneWithStaticImages: return "Game scene: Cutscene with static images";
        case gameScene_WorldSelection: return "Game scene: World selection";
        case gameScene_InGameDialog: return "Game scene: Ingame dialog";
        case gameScene_InGameOlympusBattle: return "Game scene: Ingame (Olympus battle)";
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
    if (!_superShouldRenderFrame())
    {
        return false;
    }

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

    int ingameState = nds->ARM7Read8(getU32ByCart(GAME_STATE_ADDRESS_US, GAME_STATE_ADDRESS_EU, GAME_STATE_ADDRESS_JP));
    bool isMainMenuOrIntroOrLoadMenu = nds->ARM7Read8(getU32ByCart(IS_MAIN_MENU_US, IS_MAIN_MENU_EU, IS_MAIN_MENU_JP)) == 0x00;
    bool isPauseScreen = nds->ARM7Read8(getU32ByCart(PAUSE_SCREEN_ADDRESS_US, PAUSE_SCREEN_ADDRESS_EU, PAUSE_SCREEN_ADDRESS_JP)) == PAUSE_SCREEN_VALUE_TRUE_PAUSE;
    bool isCutscene = nds->ARM7Read8(getU32ByCart(IS_CUTSCENE_US, IS_CUTSCENE_EU, IS_CUTSCENE_JP)) == 0x03;
    bool isInGameDialog = nds->ARM7Read32(getU32ByCart(DIALOG_SCREEN_ADDRESS_US, DIALOG_SCREEN_ADDRESS_EU, DIALOG_SCREEN_ADDRESS_JP)) ==
        getU32ByCart(DIALOG_SCREEN_VALUE_US, DIALOG_SCREEN_VALUE_EU, DIALOG_SCREEN_VALUE_JP);

    u8 gameState2 = nds->ARM7Read8(getU32ByCart(IS_PLAYABLE_AREA_US, IS_PLAYABLE_AREA_EU, IS_PLAYABLE_AREA_JP));
    bool isUnplayableArea = gameState2 == 0x01 || gameState2 == 0x02;
    bool isWorldSelection = gameState2 == 0x03;

    u32 minimapCenterXAddress = getU32ByCart(MINIMAP_1_CENTER_X_ADDRESS_US, MINIMAP_1_CENTER_X_ADDRESS_EU, MINIMAP_1_CENTER_X_ADDRESS_JP);
    u32 minimapCenterYAddress = getU32ByCart(MINIMAP_1_CENTER_Y_ADDRESS_US, MINIMAP_1_CENTER_Y_ADDRESS_EU, MINIMAP_1_CENTER_Y_ADDRESS_JP);
    u32 newMinimapCenterX = nds->ARM7Read32(minimapCenterXAddress) >> 3*4;
    u32 newMinimapCenterY = nds->ARM7Read32(minimapCenterYAddress) >> 3*4;
    if (newMinimapCenterX < 256 && newMinimapCenterX > 0 && newMinimapCenterY < 192 && newMinimapCenterY > 0) {
        MinimapCenterX = newMinimapCenterX;
        MinimapCenterY = newMinimapCenterY;
    }
    else {
        minimapCenterXAddress = getU32ByCart(MINIMAP_2_CENTER_X_ADDRESS_US, MINIMAP_2_CENTER_X_ADDRESS_EU, MINIMAP_2_CENTER_X_ADDRESS_JP);
        minimapCenterYAddress = getU32ByCart(MINIMAP_2_CENTER_Y_ADDRESS_US, MINIMAP_2_CENTER_Y_ADDRESS_EU, MINIMAP_2_CENTER_Y_ADDRESS_JP);
        newMinimapCenterX = nds->ARM7Read32(minimapCenterXAddress) >> 3*4;
        newMinimapCenterY = nds->ARM7Read32(minimapCenterYAddress) >> 3*4;
        if (newMinimapCenterX < 256 && newMinimapCenterX > 0 && newMinimapCenterY < 192 && newMinimapCenterY > 0) {
            MinimapCenterX = newMinimapCenterX;
            MinimapCenterY = newMinimapCenterY;
        }
        else {
            minimapCenterXAddress = getU32ByCart(MINIMAP_3_CENTER_X_ADDRESS_US, MINIMAP_3_CENTER_X_ADDRESS_EU, MINIMAP_3_CENTER_X_ADDRESS_JP);
            minimapCenterYAddress = getU32ByCart(MINIMAP_3_CENTER_Y_ADDRESS_US, MINIMAP_3_CENTER_Y_ADDRESS_EU, MINIMAP_3_CENTER_Y_ADDRESS_JP);
            newMinimapCenterX = nds->ARM7Read32(minimapCenterXAddress) >> 3*4;
            newMinimapCenterY = nds->ARM7Read32(minimapCenterYAddress) >> 3*4;
            if (newMinimapCenterX < 256 && newMinimapCenterX > 0 && newMinimapCenterY < 192 && newMinimapCenterY > 0) {
                MinimapCenterX = newMinimapCenterX;
                MinimapCenterY = newMinimapCenterY;
            }
            else {
                minimapCenterXAddress = getU32ByCart(MINIMAP_4_CENTER_X_ADDRESS_US, MINIMAP_4_CENTER_X_ADDRESS_EU, MINIMAP_4_CENTER_X_ADDRESS_JP);
                minimapCenterYAddress = getU32ByCart(MINIMAP_4_CENTER_Y_ADDRESS_US, MINIMAP_4_CENTER_Y_ADDRESS_EU, MINIMAP_4_CENTER_Y_ADDRESS_JP);
                newMinimapCenterX = nds->ARM7Read32(minimapCenterXAddress) >> 3*4;
                newMinimapCenterY = nds->ARM7Read32(minimapCenterYAddress) >> 3*4;
                MinimapCenterX = newMinimapCenterX;
                MinimapCenterY = newMinimapCenterY;
            }
        }
    }

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

    if (ingameState == 0x08)
    {
        return gameScene_CutsceneWithStaticImages;
    }

    if (isUnplayableArea)
    {
        return gameScene_InGameMenu;
    }
    if (isWorldSelection)
    {
        return gameScene_WorldSelection;
    }

    if (isPauseScreen)
    {
        return gameScene_PauseMenu;
    }
    else if (GameScene == gameScene_PauseMenu)
    {
        return PriorGameScene;
    }

    if (isInGameDialog)
    {
        return gameScene_InGameDialog;
    }

    u32 typeOfBattleAddr = getU32ByCart(TYPE_OF_BATTLE_ADDRESS_US, TYPE_OF_BATTLE_ADDRESS_EU, TYPE_OF_BATTLE_ADDRESS_JP);
    u32 typeOfBattle = nds->ARM7Read32(typeOfBattleAddr);
    if (typeOfBattle == 0) {
        return gameScene_InGameOlympusBattle;
    }

    // Regular gameplay
    return gameScene_InGameWithMap;
}

u32 PluginKingdomHeartsReCoded::getAspectRatioAddress()
{
    return getU32ByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);
}

u32 PluginKingdomHeartsReCoded::getMobiCutsceneAddress(CutsceneEntry* entry)
{
    return getU32ByCart(entry->usAddress, entry->euAddress, entry->jpAddress);
}

CutsceneEntry* PluginKingdomHeartsReCoded::getMobiCutsceneByAddress(u32 cutsceneAddressValue)
{
    if (cutsceneAddressValue == 0) {
        return nullptr;
    }

    CutsceneEntry* cutscene1 = nullptr;
    for (CutsceneEntry* entry = &Cutscenes[0]; entry->usAddress; entry++) {
        if (getMobiCutsceneAddress(entry) == cutsceneAddressValue) {
            cutscene1 = entry;
        }
    }

    return cutscene1;
}

u8 PluginKingdomHeartsReCoded::getU8ByCart(u8 usAddress, u8 euAddress, u8 jpAddress)
{
    u8 cutsceneAddress = 0;
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

u32 PluginKingdomHeartsReCoded::getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress)
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

std::string PluginKingdomHeartsReCoded::getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress)
{
    std::string cutsceneAddress = "";
    if (isUsaCart()) {
        cutsceneAddress = usAddress;
    }
    else if (isEuropeCart()) {
        cutsceneAddress = euAddress;
    }
    else if (isJapanCart()) {
        cutsceneAddress = jpAddress;
    }
    return cutsceneAddress;
}

bool PluginKingdomHeartsReCoded::getBoolByCart(bool usAddress, bool euAddress, bool jpAddress)
{
    bool cutsceneAddress = false;
    if (isUsaCart()) {
        cutsceneAddress = usAddress;
    }
    else if (isEuropeCart()) {
        cutsceneAddress = euAddress;
    }
    else if (isJapanCart()) {
        cutsceneAddress = jpAddress;
    }
    return cutsceneAddress;
}

u32 PluginKingdomHeartsReCoded::detectTopScreenMobiCutsceneAddress()
{
    return getU32ByCart(CUTSCENE_ADDRESS_US, CUTSCENE_ADDRESS_EU, CUTSCENE_ADDRESS_JP);
}

bool PluginKingdomHeartsReCoded::isCutsceneGameScene()
{
    return GameScene == gameScene_Cutscene;
}

bool PluginKingdomHeartsReCoded::didMobiCutsceneEnded()
{
    bool isCutsceneScene = GameScene == gameScene_Cutscene;
    if (!isCutsceneScene) {
        return true;
    }

    if (isSaveLoaded()) {
        // the old cutscene ended, and a new cutscene started
        return _NextCutscene != nullptr;
    }

    return false;
}

bool PluginKingdomHeartsReCoded::canReturnToGameAfterReplacementCutscene()
{
    if (isSaveLoaded()) {
        bool isCutsceneScene = GameScene == gameScene_Cutscene;
        // either:
        // 1. the cutscene is over
        // 2. the old cutscene ended, and a new cutscene started, so it needs to be skipped as well
        // 3. the cutscene is unskippable, so even if it didn't end, we need to return
        return !isCutsceneScene || _NextCutscene != nullptr || _IsUnskippableCutscene;
    }
    
    return true;
}

std::filesystem::path PluginKingdomHeartsReCoded::patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath) {
    std::string filename = "hd" + std::string(cutscene->MmName) + ".mp4";
    std::filesystem::path fullPath = folderPath / filename;
    if (!std::filesystem::exists(fullPath))
    {
        // TODO: KH Cutscene should be patched, if needed
    }
    if (!std::filesystem::exists(fullPath))
    {
        return "";
    }
    return fullPath;
}

std::string PluginKingdomHeartsReCoded::replacementCutsceneFilePath(CutsceneEntry* cutscene) {
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
        std::filesystem::path newEpicFolderPath = collectionPath / "EPIC" / "Mare" / "MOVIE" / "ReCoded" / "en";
        if (std::filesystem::exists(newEpicFolderPath)) {
            std::filesystem::path newEpicFullPath = patchReplacementCutsceneIfNeeded(cutscene, newEpicFolderPath);
            if (newEpicFullPath != "") {
                return newEpicFullPath.string();
            }
        }
        std::filesystem::path newSteamFolderPath = collectionPath / "STEAM" / "Mare" / "MOVIE" / "ReCoded" / "dt";
        if (std::filesystem::exists(newSteamFolderPath)) {
            std::filesystem::path newSteamFullPath = patchReplacementCutsceneIfNeeded(cutscene, newSteamFolderPath);
            if (newSteamFullPath != "") {
                return newSteamFullPath.string();
            }
        }
    }

    return "";
}

bool PluginKingdomHeartsReCoded::isUnskippableMobiCutscene(CutsceneEntry* cutscene) {
    return false;
    // return isSaveLoaded() && strcmp(cutscene->DsName, "843") == 0;
}

std::string PluginKingdomHeartsReCoded::replacementBackgroundMusicFilePath(std::string name) {
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

std::string PluginKingdomHeartsReCoded::localizationFilePath(std::string language) {
    std::string filename = language + ".ini";
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path fullPath = assetsFolderPath / "localization" / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    return "";
}

u32 PluginKingdomHeartsReCoded::getCurrentMission()
{
    return 0;
    // return nds->ARM7Read8(getU32ByCart(CURRENT_MISSION_US, CURRENT_MISSION_EU, CURRENT_MISSION_JP, CURRENT_MISSION_JP_REV1));
}

u32 PluginKingdomHeartsReCoded::getCurrentMap()
{
    return 0;

    // if (GameScene == -1)
    // {
    //     return 0;
    // }

    // u8 world = nds->ARM7Read8(getU32ByCart(CURRENT_WORLD_US, CURRENT_WORLD_EU, CURRENT_WORLD_JP, CURRENT_WORLD_JP_REV1));
    // u8 map = nds->ARM7Read8(getU32ByCart(CURRENT_MAP_FROM_WORLD_US, CURRENT_MAP_FROM_WORLD_EU, CURRENT_MAP_FROM_WORLD_JP, CURRENT_MAP_FROM_WORLD_JP_REV1));
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

void PluginKingdomHeartsReCoded::debugLogs(int gameScene)
{
    // PRINT_AS_32_BIT_HEX(0x020dacd0);
    // PRINT_AS_32_BIT_HEX(0x020b7db8);
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