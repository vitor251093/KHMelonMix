#ifndef PLUGIN_H
#define PLUGIN_H

#define REPLACEMENT_CUTSCENES_ENABLED true

#define DEBUG_MODE_ENABLED false
#define DEBUG_LOG_FILE_ENABLED  false

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

struct DialogueEntry
{
    char Name[40];
    int Mission;
    int skipNum;
    int sceneStart;
    int sceneEnd;
    bool startImmediate;
    int usStart;
    long long int usValues;
    char nextScene[40];
    int nextSceneStart;
    int nextSceneEnd;
    char nextScene2[40];
    int nextSceneStart2;
    int nextSceneEnd2;
    //char nextScene3[40];
    //int nextSceneStart3;
    //int nextSceneEnd3;
    //char nextScene4[40];
    //int nextSceneStart4;
    //int nextSceneEnd4;
};

class Plugin
{
public:
    virtual ~Plugin() { };

    u32 GameCode = 0;
    static bool isCart(u32 gameCode) {return true;};

    virtual std::string assetsFolder() = 0;

    virtual const char* gpuOpenGL_FS() { return nullptr; };
    virtual const char* gpu3DOpenGL_VS_Z() { return nullptr; };

    virtual void gpuOpenGL_FS_initVariables(GLuint CompShader) { };
    virtual void gpuOpenGL_FS_updateVariables(GLuint CompShader) { };
    virtual void gpu3DOpenGL_VS_Z_initVariables(GLuint prog, u32 flags) { };
    virtual void gpu3DOpenGL_VS_Z_updateVariables(u32 flags) { };

    virtual void onLoadState(melonDS::NDS* nds) { };

    virtual u32 applyHotkeyToInputMask(melonDS::NDS* nds, u32 InputMask, u32 HotkeyMask, u32 HotkeyPress) = 0;
    virtual void applyTouchKeyMask(melonDS::NDS* nds, u32 TouchKeyMask) = 0;

    virtual bool ShouldTerminateIngameCutscene() = 0;
    virtual bool ShouldStartReplacementCutscene() = 0;
    virtual bool StartedReplacementCutscene() = 0;
    virtual bool ShouldStopReplacementCutscene() = 0;
    virtual bool ShouldReturnToGameAfterCutscene() = 0;
    virtual CutsceneEntry* CurrentCutscene() = 0;
    virtual std::string CutsceneFilePath(CutsceneEntry* cutscene) = 0;
    virtual void onIngameCutsceneIdentified(melonDS::NDS* nds, CutsceneEntry* cutscene) = 0;
    virtual void onTerminateIngameCutscene(melonDS::NDS* nds) = 0;
    virtual void onReturnToGameAfterCutscene(melonDS::NDS* nds) = 0;
    virtual void onReplacementCutsceneStart(melonDS::NDS* nds) = 0;
    virtual void onReplacementCutsceneEnd(melonDS::NDS* nds) = 0;

    virtual const char* getGameSceneName() = 0;

    virtual bool shouldRenderFrame(melonDS::NDS* nds) = 0;

    virtual bool refreshGameScene(melonDS::NDS* nds) = 0;

    virtual void setAspectRatio(melonDS::NDS* nds, float aspectRatio) = 0;

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
