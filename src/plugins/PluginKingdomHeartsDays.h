#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsDays : public Plugin
{
public:
    PluginKingdomHeartsDays(u32 gameCode);

    u32 GameCode;
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    u32 getGameCode() { return GameCode; };
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };
    bool isJapanCartRev1() { return GameCode == jpGamecode && nds != nullptr && nds->GetNDSCart() != nullptr && nds->GetNDSCart()->GetROMLength() == 268435456; };

    void loadLocalization();
    void onLoadROM();

    std::string assetsFolder();
    std::string assetsRegionSubfolder();

    const char* gpuOpenGL_FS();
    const char* gpu3DOpenGLClassic_VS_Z();
    void gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32* x, s32* y, s32 z, s32* rgb);

    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);
    void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGL_VS_Z_updateVariables(u32 flags);

    void onLoadState();

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    bool applyTouchKeyMask(u32 TouchKeyMask);

    bool shouldExportTextures() {
        return ExportTextures;
    }
    bool shouldStartInFullscreen() {
        return FullscreenOnStartup;
    }

    std::string replacementCutsceneFilePath(CutsceneEntry* cutscene);
    std::string LocalizationFilePath(std::string language);
    std::filesystem::path patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    bool isUnskippableMobiCutscene(CutsceneEntry* cutscene);

    int DelayBeforeStartReplacementBackgroundMusic();
    std::string replacementBackgroundMusicFilePath(std::string name);

    const char* getGameSceneName();

    bool shouldRenderFrame();

    void setAspectRatio(float aspectRatio);

    void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        std::string root = getStringByCart("KHDays_US", "KHDays_EU", "KHDays_JP", "KHDays_JPRev1");

        KH_15_25_Remix_Location = getStringConfig(root + ".Kingdom_Hearts_HD_1_5_2_5_Remix_Location");
        KHDaysLanguage = getStringConfig(root + ".Language");
        DisableEnhancedGraphics = getBoolConfig(root + ".DisableEnhancedGraphics");
        ExportTextures = getBoolConfig(root + ".ExportTextures");
        FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    }
private:
    bool PausedInGame;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    u32 priorMap;
    u32 Map;
    int UIScale = 4;
    float AspectRatio;
    bool ShowMap;
    bool ShowTarget;
    bool ShowMissionGauge;
    bool ShowMissionInfo;
    bool HideAllHUD;

    std::map<GLuint, GLuint[11]> CompGpuLoc{};
    std::map<u32, GLuint[4]> CompGpu3DLoc{};

    bool _muchOlderHad3DOnTopScreen;
    bool _muchOlderHad3DOnBottomScreen;
    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    bool _hasVisible3DOnBottomScreen;
    bool _ignore3DOnBottomScreen;
    bool _priorIgnore3DOnBottomScreen;
    bool _priorPriorIgnore3DOnBottomScreen;

    u32 PriorHotkeyMask, PriorPriorHotkeyMask;
    u32 LastLockOnPress, LastSwitchTargetPress;
    bool SwitchTargetPressOnHold;

    std::array<CutsceneEntry, 46> Cutscenes;

    std::string KH_15_25_Remix_Location = "";
    std::string KHDaysLanguage = "";
    bool DisableEnhancedGraphics = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;

    int detectGameScene();

    u32 getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress, u32 jpRev1Address);
    std::string getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress, std::string jpRev1Address);
    bool getBoolByCart(bool usAddress, bool euAddress, bool jpAddress, bool jpRev1Address);

    u32 getMobiCutsceneAddress(CutsceneEntry* entry);
    u32 detectTopScreenMobiCutsceneAddress();
    u32 detectBottomScreenMobiCutsceneAddress();
    CutsceneEntry* detectTopScreenMobiCutscene();
    CutsceneEntry* detectBottomScreenMobiCutscene();
    bool isCutsceneGameScene();
    bool didMobiCutsceneEnded();
    bool canReturnToGameAfterReplacementCutscene();

    u16 detectMidiBackgroundMusic();
    void refreshBackgroundMusic();

    u32 getCurrentMission();
    u32 getCurrentMainMenuView();
    u32 getCurrentMap();
    bool isSaveLoaded();

    bool isBufferBlack(unsigned int* buffer);
    bool isTopScreen2DTextureBlack();
    bool isBottomScreen2DTextureBlack();
    void hudToggle();
    void debugLogs(int gameScene);
};
}

#endif
