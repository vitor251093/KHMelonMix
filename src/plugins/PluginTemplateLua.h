
#ifndef PLUGIN_TEMPLATE_LUA_H
#define PLUGIN_TEMPLATE_LUA_H

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins
{
using namespace melonDS;

int Push2DShape(ShapeData2D shape);
int Push3DShape(ShapeData3D shape);
int Set2DShapes(std::vector<int> shapes);
int Set3DShapes(std::vector<int> shapes);
void resetShapeBuffers();
void addGameCode(u32 gameCode);
void clearGameCodes();
void setLuaGameScene(int gamescene);
bool checkGameCodes(u32 gameCode);
int run_ShapeBuilderTests();

class PluginTemplateLua : public Plugin
{
public:
    PluginTemplateLua(u32 gameCode);
    static u32 usGamecode;
    static u32 euGamecode;
    static u32 jpGamecode;
    static bool isCart(u32 gameCode) { return checkGameCodes(gameCode);};
    bool isUsaCart()        { return GameCode == usGamecode; };
    bool isEuropeCart()     { return GameCode == euGamecode; };
    bool isJapanCart()      { return GameCode == jpGamecode; };

    std::string gameFolderName() {
        return std::to_string(GameCode);
    }
    int renderer_gameSceneState();
    std::vector<ShapeData2D> renderer_composition();
    std::vector<ShapeData2D> renderer_topScreen_2DShapes();
    std::vector<ShapeData3D> renderer_topScreen_3DShapes();
    int renderer_screenLayout();
    int renderer_brightnessMode();
    bool renderer_showOriginalUI();

    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);

    const char* getGameSceneName() {
        return "";
    }

    u32 getAspectRatioAddress();

private:
    int detectGameScene();
};
}

#endif