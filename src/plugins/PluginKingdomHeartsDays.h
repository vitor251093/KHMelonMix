#ifndef KHDAYS_PLUGIN_H
#define KHDAYS_PLUGIN_H

#include "Plugin.h"
#include "../NDS.h"

#define SWITCH_TARGET_PRESS_FRAME_LIMIT   100
#define SWITCH_TARGET_TIME_BETWEEN_SWITCH 20
#define LOCK_ON_PRESS_FRAME_LIMIT         100

namespace Plugins
{
using namespace melonDS;

class PluginKingdomHeartsDays : public Plugin
{
public:
    PluginKingdomHeartsDays(u32 gameCode);

    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) {return gameCode == usGamecode || gameCode == euGamecode || gameCode == jpGamecode;};
    bool isUsaCart()    { return GameCode == usGamecode; };
    bool isEuropeCart() { return GameCode == euGamecode; };
    bool isJapanCart()  { return GameCode == jpGamecode; };
    bool isJapanCartRev1() { return GameCode == jpGamecode && nds != nullptr && nds->GetNDSCart() != nullptr && nds->GetNDSCart()->GetROM()[0x1E] == 1; };

    void loadLocalization();
    void onLoadROM() override;
    void onLoadState() override;

    std::string assetsFolder();
    std::string assetsRegionSubfolder();
    std::string tomlUniqueIdentifier();

    std::vector<ShapeData2D> renderer_2DShapes(int gameScene, int gameSceneState);
    std::vector<ShapeData3D> renderer_3DShapes(int gameScene, int gameSceneState);
    int renderer_gameSceneState();
    int renderer_screenLayout();
    int renderer_brightnessMode();
    float renderer_forcedAspectRatio();
    bool renderer_showOriginalUI();

    void applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    bool shouldRumble();

    bool overrideMouseTouchCoords_cameraControl(int width, int height, int& x, int& y, bool& touching);
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
    bool PausedInGame = false;
    bool isCharacterControllable = false;

    bool IsBottomScreen2DTextureBlack;
    bool IsTopScreen2DTextureBlack;
    u32 priorMap;
    u32 Map;

    bool ShowMap;
    bool ShowTarget;
    bool ShowMissionGauge;
    bool ShowMissionInfo;
    bool HideAllHUD;

    std::map<u32, GLuint[5]> CompGpu3DLoc{};
    std::map<u32, int[5]> CompGpu3DLastValues{};

    // game scene detection utils
    bool _muchOlderHad3DOnTopScreen = false;
    bool _muchOlderHad3DOnBottomScreen = false;
    bool _olderHad3DOnTopScreen = false;
    bool _olderHad3DOnBottomScreen = false;
    bool _had3DOnTopScreen = false;
    bool _had3DOnBottomScreen = false;

    // should render frame utils
    bool _hasVisible3DOnBottomScreen = false;
    bool _ignore3DOnBottomScreen = false;
    bool _priorIgnore3DOnBottomScreen = false;
    bool _priorPriorIgnore3DOnBottomScreen = false;

    // apply addon to input mask utils
    u32 PriorAddonMask = 0, PriorPriorAddonMask = 0;
    u32 LastSwitchTargetPress = SWITCH_TARGET_PRESS_FRAME_LIMIT;
    u32 LastLockOnPress = LOCK_ON_PRESS_FRAME_LIMIT;
    bool SwitchTargetPressOnHold = false;

    std::array<CutsceneEntry, 46> Cutscenes;

    std::string KH_15_25_Remix_Location = "";
    std::string TextLanguage = "";

    int detectGameScene();

    u32 getMobiCutsceneAddress(CutsceneEntry* entry);
    CutsceneEntry* getMobiCutsceneByAddress(u32 cutsceneAddressValue);
    u32 detectTopScreenMobiCutsceneAddress();
    u32 detectBottomScreenMobiCutsceneAddress();
    bool isCutsceneGameScene();
    bool didMobiCutsceneEnded();
    bool canReturnToGameAfterReplacementCutscene();

    // Music replacement system
    std::array<BgmEntry, 38> BgmEntries;

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
    int delayBeforeStartReplacementBackgroundMusic(u16 bgmId) override;

    s16 getSongIdInSongTable(u16 bgmId);

    struct StreamedBgmEntry
    {
        u8 dsId = 0;
        u8 customId = 0;
        char Name[40];
        u32 numSamples = 0;
    };

    std::array<StreamedBgmEntry, 2> StreamedBgmEntries;

    void refreshMouseStatus();

    u32 getCurrentMission();
    u32 getCurrentMainMenuView();
    u32 getCurrentMap();
    bool isSaveLoaded();

    bool isBufferBlack(unsigned int* buffer);
    u32* topScreen2DTexture();
    u32* bottomScreen2DTexture();
    bool isBottomScreen2DTextureBlack();

    bool ShouldShowBottomScreen = false;
    bool isDialogVisible();
    bool isMinimapVisible();
    bool isMissionInformationVisibleOnTopScreen();
    bool isMissionInformationVisibleOnBottomScreen();
    bool isCutsceneFromChallengeMissionVisible();
    bool isDialogPortraitLabelVisible();
    bool isLoadScreenDeletePromptVisible();
    bool has2DOnTopOf3DAt(u32* buffer, int x, int y);

    void hudToggle();
    void debugLogs(int gameScene);
};
}

#endif
