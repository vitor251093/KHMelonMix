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

    std::string assetsFolder() override;
    std::string assetsRegionSubfolder();
    std::string tomlUniqueIdentifier() override;

    void renderer_beforeBuildingShapes() override;
    std::vector<ShapeData2D> renderer_2DShapes() override;
    std::vector<ShapeData3D> renderer_3DShapes() override;
    void renderer_afterBuildingShapes() override;
    int renderer_gameSceneState() override;
    int renderer_screenLayout() override;
    int renderer_brightnessMode() override;
    float renderer_forcedAspectRatio() override;
    bool renderer_showOriginalUI() override;

    void applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) override;
    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) override;
    bool shouldRumble() override;

    bool overrideMouseTouchCoords_cameraControl(int width, int height, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords_singleScreen(int width, int height, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords_horizontalDualScreen(int width, int height, bool invert, int& x, int& y, bool& touching);
    bool overrideMouseTouchCoords(int width, int height, int& x, int& y, bool& touching) override;
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) override;

    std::string replacementCutsceneFilePath(CutsceneEntry* cutscene) override;
    std::string localizationFilePath(std::string language) override;
    std::filesystem::path patchReplacementCutsceneIfNeeded(CutsceneEntry* cutscene, std::filesystem::path folderPath);
    bool isUnskippableMobiCutscene(CutsceneEntry* cutscene) override;

    const char* getGameSceneName() override;

    bool shouldRenderFrame() override;

    u32 getAspectRatioAddress() override;

    void loadConfigs(
        std::function<bool(std::string)> getBoolConfig,
        std::function<int(std::string)> getIntConfig,
        std::function<std::string(std::string)> getStringConfig
    ) override
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
    bool ShowFullscreenMap;
    bool ShowTarget;
    bool ShowMissionGauge;
    bool ShowMissionInfo;
    bool HideAllHUD;

    int fullscreenMapTransitionStep = 0;

    std::map<u32, GLuint[5]> CompGpu3DLoc{};
    std::map<u32, int[5]> CompGpu3DLastValues{};
    /*bool _double3DTopScreen2DTextureEnabled = false;
    u32 _double3DTopScreen2DTexture[256 * 192 * 4] = {};*/

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

    int detectGameScene() override;

    u32 getMobiCutsceneAddress(CutsceneEntry* entry);
    CutsceneEntry* getMobiCutsceneByAddress(u32 cutsceneAddressValue) override;
    u32 detectTopScreenMobiCutsceneAddress() override;
    u32 detectBottomScreenMobiCutsceneAddress() override;
    bool isCutsceneGameScene() override;
    bool didMobiCutsceneEnded() override;
    bool canReturnToGameAfterReplacementCutscene() override;

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

    void refreshMouseStatus() override;

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
    bool isMissionGaugeVisibleOnBottomScreen();
    bool isTargetVisibleOnBottomScreen();
    bool isCutsceneFromChallengeMissionVisible();
    bool isDialogPortraitLabelVisible();
    bool isLoadScreenDeletePromptVisible();
    int dialogBoxHeight();
    bool has2DOnTopOf3DAt(u32* buffer, int x, int y);

    void renderer_2DShapes_saveScreenMenu(std::vector<ShapeData2D>* shapes, float aspectRatio, int hudScale);
    void renderer_2DShapes_loadScreenMenu(std::vector<ShapeData2D>* shapes, float aspectRatio, int hudScale);
    void renderer_2DShapes_component_characterDialog(std::vector<ShapeData2D>* shapes, float aspectRatio, int hudScale);
    void renderer_2DShapes_component_targetView(std::vector<ShapeData2D>* shapes, float aspectRatio, int hudScale);
    void renderer_2DShapes_component_bottomMissionInformation(std::vector<ShapeData2D>* shapes, float aspectRatio, int hudScale);

    void hudToggle() override;
    void toggleFullscreenMap();
    void debugLogs(int gameScene) override;
};
}

#endif
