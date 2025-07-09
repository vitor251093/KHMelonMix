#include "PluginTemplateLua.h"

#define ASPECT_RATIO_ADDRESS_US      0
#define ASPECT_RATIO_ADDRESS_EU      0
#define ASPECT_RATIO_ADDRESS_JP      0

namespace Plugins
{

u32 PluginTemplateLua::usGamecode = 0;
u32 PluginTemplateLua::euGamecode = 0;
u32 PluginTemplateLua::jpGamecode = 0;

#define getAnyByCart(usAddress,euAddress,jpAddress) (isUsaCart() ? (usAddress) : (isEuropeCart() ? (euAddress) : (jpAddress)))

PluginTemplateLua::PluginTemplateLua(u32 gameCode)
{
    GameCode = gameCode;

    hudToggle();
}


static int luaGameScene;

int PluginTemplateLua::detectGameScene()
{
    if (nds == nullptr)
    {
        return GameScene;
    }

    return luaGameScene;
}

static std::vector<ShapeData2D> BuiltShapes;
//Push shape data onto the global BuiltShapes array
int PushShape(ShapeData2D shape){
    BuiltShapes.push_back(shape);
    return BuiltShapes.size()-1;
}

static std::vector<ShapeData2D> CurrentShapes;
int SetShapes(std::vector<int> shapes){
    CurrentShapes.clear();
    for (int const& index: shapes){
        if (index<BuiltShapes.size() and index>=0){
            CurrentShapes.push_back(BuiltShapes[index]);
        } else {
            CurrentShapes.clear();
            printf("Shape Index not found..."); // This can happen if the lua script uses an invalid index by mistake 
            //TODO: throw error to LUA console...
            return -1;
        }
    }
    return 0;
}

void setLuaGameScene(int gamescene){
    luaGameScene = gamescene;   
}

int PluginTemplateLua::renderer_gameSceneState(){
    int gamescene = luaGameScene;
    return gamescene;
}

std::vector<ShapeData2D> PluginTemplateLua::renderer_2DShapes(int gameScene, int gameSceneState) {    
    std::vector<ShapeData2D> shapes = CurrentShapes;
    /*
    for(int i =0;i<shapes.size();i++){
        printf("colorToAlpha:%x,%x,%x,%x\n",shapes[i].colorToAlpha.w,shapes[i].colorToAlpha.x,shapes[i].colorToAlpha.y,shapes[i].colorToAlpha.z);
        printf("effects:%i\n",shapes[i].effects);
        printf("fadeBorderSize:%x,%x,%x,%x\n",shapes[i].fadeBorderSize.w,shapes[i].fadeBorderSize.x,shapes[i].fadeBorderSize.y,shapes[i].fadeBorderSize.z);
        printf("opacity:%f\n",shapes[i].opacity);
        printf("singleColorToAlpha:%x,%x,%x,%x\n",shapes[i].singleColorToAlpha.w,shapes[i].singleColorToAlpha.x,shapes[i].singleColorToAlpha.y,shapes[i].singleColorToAlpha.z);
        printf("sourceScale:%x,%x\n",shapes[i].sourceScale.x,shapes[i].sourceScale.y);
        printf("squareCornersModifier:%x,%x,%x,%x\n",shapes[i].squareCornersModifier.w,shapes[i].squareCornersModifier.x,shapes[i].squareCornersModifier.y,shapes[i].squareCornersModifier.z);
        printf("squareFinalCoords:%x,%x,%x,%x\n",shapes[i].squareFinalCoords.w,shapes[i].squareFinalCoords.x,shapes[i].squareFinalCoords.y,shapes[i].squareFinalCoords.z);
        printf("squareInitialCoords:%x,%x,%x,%x\n",shapes[i].squareInitialCoords.w,shapes[i].squareInitialCoords.x,shapes[i].squareInitialCoords.y,shapes[i].squareInitialCoords.z);
    }
    */
    return shapes;
}

int PluginTemplateLua::renderer_screenLayout() {
    return screenLayout_Top;
};

int PluginTemplateLua::renderer_brightnessMode() {
    return brightnessMode_TopScreen;
}

bool PluginTemplateLua::renderer_showOriginalUI() {
    return false;
}

void PluginTemplateLua::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    
}
void PluginTemplateLua::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, 3, true);
}

u32 PluginTemplateLua::getAspectRatioAddress() {
    return getAnyByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);
}

}