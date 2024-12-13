#ifndef KHRECODED_PLUGIN_H
#define KHRECODED_PLUGIN_H

#include "Plugin.h"
#include "../NDS.h"

#define PRIOR_HOTKEY_MASK_SIZE 15

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsReCoded : public Plugin
{
public:
    PluginKingdomHeartsReCoded(u32 gameCode);

    u32 GameCode;
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    u32 getGameCode() { return GameCode; };
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };

    void loadLocalization();
    void onLoadROM();

    std::string assetsFolder();

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

    std::string CutsceneFilePath(CutsceneEntry* cutscene);
    std::string LocalizationFilePath(std::string language);
    std::filesystem::path patchCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    bool isUnskippableCutscene(CutsceneEntry* cutscene);

    std::string BackgroundMusicFilePath(std::string name);

    const char* getGameSceneName();

    bool shouldRenderFrame();

    void setAspectRatio(float aspectRatio);

    void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        std::string root = getStringByCart("KHReCoded_US", "KHReCoded_EU", "KHReCoded_JP");

        KH_15_25_Remix_Location = getStringConfig(root + ".Kingdom_Hearts_HD_1_5_2_5_Remix_Location");
        DisableEnhancedGraphics = getBoolConfig(root + ".DisableEnhancedGraphics");
        ExportTextures = getBoolConfig(root + ".ExportTextures");
        FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    }
private:
    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    u32 priorMap;
    u32 Map;
    int UIScale = 4;
    float AspectRatio;
    bool ShowMap;
    int MinimapCenterX;
    int MinimapCenterY;
    bool HideAllHUD;

    std::map<GLuint, GLuint[10]> CompGpuLoc{};
    std::map<u32, GLuint[3]> CompGpu3DLoc{};

    bool _muchOlderHad3DOnTopScreen;
    bool _muchOlderHad3DOnBottomScreen;
    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    u32 PriorHotkeyMask[PRIOR_HOTKEY_MASK_SIZE];
    u32 LastLockOnPress, LastSwitchTargetPress, LastScreenTogglePress;

    std::array<CutsceneEntry, 15> Cutscenes;
    std::string KH_15_25_Remix_Location = "";
    bool DisableEnhancedGraphics = false;
    bool ExportTextures = false;
    bool FullscreenOnStartup = false;

    int detectGameScene();

    u32 getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress);
    std::string getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress);
    bool getBoolByCart(bool usAddress, bool euAddress, bool jpAddress);

    u32 getCutsceneAddress(CutsceneEntry* entry);
    u32 detectTopScreenCutsceneAddress();
    CutsceneEntry* detectTopScreenCutscene();
    bool isCutsceneGameScene();
    bool didIngameCutsceneEnded();
    bool canReturnToGameAfterReplacementCutscene();

    u32 getCurrentMission();
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
