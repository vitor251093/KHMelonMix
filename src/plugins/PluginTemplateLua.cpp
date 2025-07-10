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

static std::vector<ShapeData2D> _2DShapeBuffer;
//Push shape data onto the global BuiltShapes array
int Push2DShape(ShapeData2D shape){
    _2DShapeBuffer.push_back(shape);
    return _2DShapeBuffer.size()-1;
}

static std::vector<ShapeData2D> Current2DShapes;
int Set2DShapes(std::vector<int> shapes){
    Current2DShapes.clear();
    for (int const& index: shapes){
        if (index<_2DShapeBuffer.size() and index>=0){
            Current2DShapes.push_back(_2DShapeBuffer[index]);
        } else {
            Current2DShapes.clear();
            printf("2DShape Index not found..."); // This can happen if the lua script uses an invalid index by mistake 
            //TODO: throw error to LUA console...
            return -1;
        }
    }
    return 0;
}

static std::vector<ShapeData3D> _3DShapeBuffer;
int Push3DShape(ShapeData3D shape){
    _3DShapeBuffer.push_back(shape);
    return _3DShapeBuffer.size()-1;
}

static std::vector<ShapeData3D> Current3DShapes;
int Set3DShapes(std::vector<int> shapes){
    Current3DShapes.clear();
    for (int const& index: shapes){
        if (index<_3DShapeBuffer.size() and index>=0){
            Current3DShapes.push_back(_3DShapeBuffer[index]);
        } else {
            Current3DShapes.clear();
            printf("3DShape Index not found..."); // This can happen if the lua script uses an invalid index by mistake 
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

std::vector<ShapeData2D> PluginTemplateLua::renderer_2DShapes() {    
    std::vector<ShapeData2D> shapes = Current2DShapes;
    return shapes;
}

std::vector<ShapeData3D> PluginTemplateLua::renderer_3DShapes() {
    std::vector<ShapeData3D> shapes = Current3DShapes;
    p1 = ShapeBuilder3D::square()
                    .fromPosition(128, 140)
                    .withSize(128, 52)
                    .placeAtCorner(corner_BottomRight)
                    .zRange(0.25, 0.50)
                    .hudScale(UIScale)
                    .build(aspectRatio));
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