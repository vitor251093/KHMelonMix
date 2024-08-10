#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true

#define DEBUG_MODE_ENABLED false
#define DEBUG_LOG_FILE_ENABLED false

#include "NDS.h"

#include "OpenGLSupport.h"

namespace Plugins
{
using namespace melonDS;

struct CutsceneEntry
{
    char DsName[12];
    char Name[40];
    int usAddress;
    int euAddress;
    int jpAddress;
};

class Plugin
{
public:
    virtual ~Plugin() { };

    u32 GameCode = 0;
    static bool isCart(u32 gameCode) {return true;};

    virtual void setNds(melonDS::NDS* Nds) = 0;

    virtual std::string assetsFolder() = 0;

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual const char* gpu3DOpenGL_VS_Z() { return nullptr; };

    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) { };
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) { };
    virtual void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags) { };
    virtual void gpu3DOpenGL_VS_Z_updateVariables(u32 flags) { };

    virtual void onLoadState() { };

    virtual u32 applyHotkeyToInputMask(u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) = 0;
    virtual void applyTouchKeyMask(u32 TouchKeyMask) = 0;

    virtual bool ShouldTerminateIngameCutscene() = 0;
    virtual bool StoppedIngameCutscene() = 0;
    virtual bool ShouldStartReplacementCutscene() = 0;
    virtual bool StartedReplacementCutscene() = 0;
    virtual bool RunningReplacementCutscene() = 0;
    virtual bool ShouldStopReplacementCutscene() = 0;
    virtual bool ShouldReturnToGameAfterCutscene() = 0;
    virtual bool ShouldUnmuteAfterCutscene() = 0;
    virtual CutsceneEntry* CurrentCutscene() = 0;
    virtual std::string CutsceneFilePath(CutsceneEntry* cutscene) = 0;
    virtual void onIngameCutsceneIdentified(CutsceneEntry* cutscene) = 0;
    virtual void onTerminateIngameCutscene() = 0;
    virtual void onReturnToGameAfterCutscene() = 0;
    virtual void onReplacementCutsceneStarted() = 0;
    virtual void onReplacementCutsceneEnd() = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldRenderFrame() = 0;

    virtual bool refreshGameScene() = 0;

    virtual void setAspectRatio(float aspectRatio) = 0;

    void log(const char* log) {
        printf("%s\n", log);

        if (DEBUG_LOG_FILE_ENABLED) {
            std::string fileName = std::string("debug.log");
            Platform::FileHandle* logf = Platform::OpenFile(fileName, Platform::FileMode::Append);
            Platform::FileWrite(log, strlen(log), 1, logf);
            Platform::FileWrite("\n", 1, 1, logf);
            Platform::CloseFile(logf);
        }
    }
};
}

#endif
