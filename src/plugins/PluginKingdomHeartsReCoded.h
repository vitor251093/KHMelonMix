#ifndef KHRECODED_PLUGIN_H
#define KHRECODED_PLUGIN_H

#include "Plugin.h"
#include "../NDS.h"

#define PRIOR_ADDON_MASK_SIZE 15

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
    void onLoadROM() override;

    std::string assetsFolder();
    std::string tomlUniqueIdentifier();

    std::vector<ShapeData2D> renderer_2DShapes(int gameScene, int gameSceneState);
    std::vector<ShapeData3D> renderer_3DShapes(int gameScene, int gameSceneState);
    int renderer_gameSceneState();
    int renderer_screenLayout();
    int renderer_brightnessMode();
    float renderer_forcedAspectRatio();
    bool renderer_showOriginalUI();

    void onLoadState() override;

    void applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    
    bool overrideMouseTouchCoords_singleScreen(int width, int height, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords_horizontalDualScreen(int width, int height, bool invert, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching);
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);

    std::string replacementCutsceneFilePath(CutsceneEntry* cutscene);
    std::string localizationFilePath(std::string language);
    std::filesystem::path patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    bool isUnskippableMobiCutscene(CutsceneEntry* cutscene);

    const char* getGameSceneName();

    bool shouldRenderFrame();

    u32 getAspectRatioAddress();

    void loadConfigs(
        std::function<bool(std::string)> getBoolConfig,
        std::function<int(std::string)> getIntConfig,
        std::function<std::string(std::string)> getStringConfig
    )
    {
        _superLoadConfigs(getBoolConfig, getIntConfig, getStringConfig);

        std::string root = tomlUniqueIdentifier();

        KH_15_25_Remix_Location = getStringConfig(root + ".Kingdom_Hearts_HD_1_5_2_5_Remix_Location");
        TextLanguage = getStringConfig(root + ".Language");
    }
private:
    bool IsTopScreen2DTextureBlack;
    u32 priorMap;
    u32 Map;
    bool ShowMap;
    bool ShowFullscreenMap;
    int MinimapCenterX = 128;
    int MinimapCenterY = 96;
    int MinimapFrameTick;
    bool HideAllHUD;
    u32 lastMainMenuView;

    std::map<GLuint, GLuint[10]> CompGpuLoc{};
    std::map<u32, GLuint[3]> CompGpu3DLoc{};
    std::map<u32, int[3]> CompGpu3DLastValues{};

    bool _muchOlderHad3DOnTopScreen;
    bool _muchOlderHad3DOnBottomScreen;
    bool _olderHad3DOnTopScreen;
    bool _olderHad3DOnBottomScreen;
    bool _had3DOnTopScreen;
    bool _had3DOnBottomScreen;

    u32 PriorAddonMask[PRIOR_ADDON_MASK_SIZE];
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

    u8 getFloorLevel();
    u32 getCurrentMission();
    u32 getCurrentMainMenuView();
    u32 getCurrentMap();
    bool isSaveLoaded();

    // Music replacement system
    std::array<BgmEntry, 41> BgmEntries;

    bool isBackgroundMusicReplacementImplemented() const override { return true; }
    u16 getMidiBgmId() override;
    u16 getMidiBgmToResumeId() override;
    u32 getMidiSequenceAddress(u16 bgmId) override;
    u16 getMidiSequenceSize(u16 bgmId) override;
    u32 getStreamBgmAddress() override;
    u16 getStreamBgmCustomIdFromDsId(u8 dsId, u32 numSamples) override;
    u8 getMidiBgmState() override;
    u8 getMidiBgmVolume() override;
    u32 getBgmFadeOutDuration() override;
    std::string getBackgroundMusicName(u16 bgmId) override;

    s16 getSongIdInSongTable(u16 bgmId);

    struct StreamedBgmEntry
    {
        u8 dsId = 0;
        u8 customId = 0;
        char Name[40];
        u32 numSamples = 0;
    };

    std::array<StreamedBgmEntry, 1> StreamedBgmEntries;
    u8 _MutedStreamBlocksCount = 0;
    void onStreamBgmReplacementStarted() override;
    void muteStreamedMusic() override;

    bool isBufferBlack(unsigned int* buffer);
    u32* topScreen2DTexture();
    u32* bottomScreen2DTexture();
    bool isTopScreen2DTextureBlack();
    bool isBottomScreen2DTextureBlack();

    bool isResultScreenVisible();
    bool isMissionInformationVisibleOnTopScreen();
    bool isDialogVisible();
    bool isMinimapVisible();
    bool isBugSector();
    bool isChallengeMeterVisible();
    bool isCommandMenuVisible();
    bool isHealthVisible();
    ivec2 minimapCenter();
    bool has2DOnTopOf3DAt(u32* buffer, int x, int y);

    void hudToggle();
    void toggleFullscreenMap();
    void debugLogs(int gameScene);
};
}

#endif
