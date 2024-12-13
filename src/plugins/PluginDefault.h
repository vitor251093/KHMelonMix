#ifndef PLUGIN_DEFAULT_H
#define PLUGIN_DEFAULT_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

class PluginDefault : public Plugin
{
public:
    PluginDefault(u32 gameCode) {GameCode = gameCode;};

    u32 GameCode = 0;
    u32 getGameCode() {
        return GameCode;
    };
    static bool isCart(u32 gameCode) {return true;};

    void setNds(melonDS::NDS* Nds) {}
    void onLoadROM() {}

    std::string assetsFolder() {
        return std::to_string(GameCode);
    }

    void onLoadState() {}
    bool togglePause() {return false;};

    void applyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress) {}
    bool applyTouchKeyMask(u32 TouchKeyMask) { return false; }
    const char* getGameSceneName() {
        return "";
    }

    bool shouldExportTextures() {return false;}
    bool shouldStartInFullscreen() {return false;}

    bool shouldRenderFrame() {
        return true;
    }
    bool ShouldTerminateIngameCutscene() {return false;}
    bool StoppedIngameCutscene() {return false;}
    bool ShouldStartReplacementCutscene() {return false;}
    bool StartedReplacementCutscene() {return false;}
    bool RunningReplacementCutscene() {return false;}
    bool ShouldPauseReplacementCutscene() {return false;}
    bool ShouldUnpauseReplacementCutscene() {return false;}
    bool ShouldStopReplacementCutscene() {return false;}
    bool ShouldReturnToGameAfterCutscene() {return false;}
    bool ShouldUnmuteAfterCutscene() {return false;}
    CutsceneEntry* CurrentCutscene() {return nullptr;}
    std::string CutsceneFilePath(CutsceneEntry* cutscene) {return "";}
    std::string LocalizationFilePath(std::string language) {return "";}
    void onIngameCutsceneIdentified(CutsceneEntry* cutscene) {}
    void onTerminateIngameCutscene() {}
    void onReturnToGameAfterCutscene() {}
    void onReplacementCutsceneStarted() {}
    void onReplacementCutsceneEnd() {}
    
    bool ShouldStartReplacementBgmMusic() {return false;}
    int DelayBeforeStartReplacementBgmMusic() {return 0;}
    bool StartedReplacementBgmMusic() {return false;}
    bool RunningReplacementBgmMusic() {return false;}
    bool ShouldPauseReplacementBgmMusic() {return false;}
    bool ShouldUnpauseReplacementBgmMusic() {return false;}
    bool ShouldStopReplacementBgmMusic() {return false;}
    u16 CurrentBackgroundMusic() {return 0;}
    std::string BackgroundMusicFilePath(std::string name) {return "";}
    void onReplacementBackgroundMusicStarted() {}

    bool refreshGameScene() {
        return false;
    }
    void setAspectRatio(float aspectRatio) {}
};
}

#endif
