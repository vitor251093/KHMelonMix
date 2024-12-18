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

    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };

    void loadLocalization();
    void onLoadROM();

    std::string assetsFolder();
    std::string tomlUniqueIdentifier();

    const char* gpuOpenGL_FS();
    void gpuOpenGL_FS_initVariables(GLuint CompShader);
    void gpuOpenGL_FS_updateVariables(GLuint CompShader);

    const char* gpu3DOpenGLClassic_VS_Z();
    void gpu3DOpenGLClassic_VS_Z_initVariables(GLuint prog, u32 flags);
    void gpu3DOpenGLClassic_VS_Z_updateVariables(u32 flags);

    void gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32 scaledPositions[10][2], melonDS::Polygon* polygon);

    void onLoadState();

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress);
    
    bool overrideMouseTouchCoords_singleScreen(int width, int height, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords_horizontalDualScreen(int width, int height, bool invert, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching);
    void applyTouchKeyMask(u32 TouchKeyMask, u16* touchX, u16* touchY, bool* isTouching);

    std::string replacementCutsceneFilePath(CutsceneEntry* cutscene);
    std::string localizationFilePath(std::string language);
    std::string textureFilePath(std::string texture);
    std::filesystem::path patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    bool isUnskippableMobiCutscene(CutsceneEntry* cutscene);

    std::string replacementBackgroundMusicFilePath(std::string name);

    const char* getGameSceneName();

    bool shouldRenderFrame();

    u32 getAspectRatioAddress();

    void loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
    {
        _superLoadConfigs(getBoolConfig, getStringConfig);

        std::string root = tomlUniqueIdentifier();

        KH_15_25_Remix_Location = getStringConfig(root + ".Kingdom_Hearts_HD_1_5_2_5_Remix_Location");
        TextLanguage = getStringConfig(root + ".Language");
    }
private:
    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    u32 priorMap;
    u32 Map;
    int UIScale = 4;
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
    std::string TextLanguage = "";

    int detectGameScene();

    u8 getU8ByCart(u8 usAddress, u8 euAddress, u8 jpAddress);
    u32 getU32ByCart(u32 usAddress, u32 euAddress, u32 jpAddress);
    std::string getStringByCart(std::string usAddress, std::string euAddress, std::string jpAddress);
    bool getBoolByCart(bool usAddress, bool euAddress, bool jpAddress);

    u32 getMobiCutsceneAddress(CutsceneEntry* entry);
    CutsceneEntry* getMobiCutsceneByAddress(u32 cutsceneAddressValue);
    u32 detectTopScreenMobiCutsceneAddress();
    bool isCutsceneGameScene();
    bool didMobiCutsceneEnded();
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
