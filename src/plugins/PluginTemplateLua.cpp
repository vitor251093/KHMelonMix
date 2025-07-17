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

static std::vector<u32> luaGameCodes;
void addGameCode(u32 gameCode)
{
    luaGameCodes.push_back(gameCode);
    printf("Codes Size:%i\n",luaGameCodes.size());
}

void clearGameCodes()
{
    printf("Clearing Codes!\n");
    luaGameCodes.clear();
}

bool checkGameCodes(u32 gameCode)
{
    printf("Test: %i\n",gameCode);
    if (luaGameCodes.size() == 0) {
        printf("No Codes\n");
        return false;
    }
    if (luaGameCodes[0] == 0xFFFFFFFF) {
        printf("All Codes\n");
        return true; // GameCode "0xFFFFFFFF" means any game
    }
    bool found = std::find(std::begin(luaGameCodes), std::end(luaGameCodes), gameCode) != std::end(luaGameCodes);
    printf("Found: %i\n",found);
    return found;
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

void resetShapeBuffers(){
    Current3DShapes.clear();
    Current2DShapes.clear();
    _3DShapeBuffer.clear();
    _2DShapeBuffer.clear();
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
    return shapes;
}

int run_ShapeBuilderTests(){
    float aspectRatio = 1.1;
    int hudScale = 5;
    float scale = 192.0/(192 - 31 + 48);
    std::vector<ShapeData2D> shapes;
    std::vector<ShapeData3D> shapes3D;
    int allGood = 1;
    printf("Starting 2D Shape Test.\n");
    //Test each method in sequence
    shapes.push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .placeAtCorner(corner_TopLeft)
            .build(aspectRatio));
            
    shapes.push_back(ShapeBuilder2D::square()
            .sourceScale(scale)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .sourceScale(1000.0,scale)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .hudScale(hudScale)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .preserveDsScale()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .fromPosition(251, 144)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .withSize(256, 48)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .withMargin(3.0, 6.0, 0.0, 0.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .withMargin(3.0, 6.0, 0.0, 0.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .withMargin(0.0, 0.0, 8.0, 4.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .fadeBorderSize(5.0, 5.0, 5.0, 5.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .invertGrayScaleColors()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .repeatAsBackground()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .repeatAsBackgroundVertically()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .force()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .mirror(mirror_X)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .cropSquareCorners(0.0, 4.0, 0.0, 0.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .squareBorderRadius(10.0, 10.0, 5.0, 5.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .squareBorderRadius(7.0)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .opacity(0.90)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .colorToAlpha(0x8, 0x30, 0xaa)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .singleColorToAlpha(0x8, 0x30, 0xaa)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .singleColorToAlpha(0x8, 0x30, 0xaa, 0.9)
            .build(aspectRatio));

    // Example Test shapes
    shapes.push_back(ShapeBuilder2D::square()
            .fromPosition(100, 0)
            .withSize(20, 16)
            .placeAtCorner(corner_TopRight)
            .sourceScale(1000.0, scale)
            .hudScale(hudScale)
            .preserveDsScale()
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(8, 32)
            .withSize(240, 104)
            .placeAtCorner(corner_Center)
            .sourceScale(1.7)
            .fadeBorderSize(5.0, 5.0, 5.0, 5.0)
            .opacity(0.90)
            .hudScale(hudScale)
            .build(aspectRatio));

    shapes.push_back(ShapeBuilder2D::square()
            .fromBottomScreen()
            .fromPosition(5, 166)
            .withSize(119, 3)
            .placeAtCorner(corner_TopLeft)
            .withMargin(131.0, 6.0, 0.0, 0.0)
            .cropSquareCorners(0.0, 4.0, 0.0, 0.0)
            .mirror(mirror_X)
            .hudScale(hudScale)
            .build(aspectRatio));

    for (int i = 0; i<shapes.size();i++){
        if (i>=_2DShapeBuffer.size()){
            printf("Ran out of 2DShapes in buffer at test number %i\n",i);
            allGood = -1;
            break;
        }
        ShapeData2D cppShape = shapes[i];
        ShapeData2D luaShape = _2DShapeBuffer[i];
        int cmpr = memcmp(&cppShape, &luaShape, sizeof(ShapeData2D));
        if (cmpr!=0){
            printf("2DShapeBuilder Test number %i failed...\n",i);
            allGood = -1;
        }
    }
    printf("Starting 3D Shape Test.\n");
    //Test each method in sequence        
    shapes3D.push_back(ShapeBuilder3D::square()
            .polygonAttributes(1058996416)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .negatePolygonAttributes(2031808)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .color(0x0830AA)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .negateColor(0xFFFFFF)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .textureParam(777777)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .negatedTextureParam(942331720)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .polygonMode()
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .adjustAspectRatioOnly()
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .polygonVertexesCount(4)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .placeAtCorner(corner_BottomLeft)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .zRange(-1.0, -0.000001)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .zValue(0.5)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .sourceScale(1.5)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .sourceScale(aspectRatio*aspectRatio, 1.0)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .hudScale(SCREEN_SCALE)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .preserveDsScale()
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .fromPosition(128, 140)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .withSize(100, 16)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .withMargin(10.0, 0.0, 0.0, 0.5)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .hide()
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .logger()
            .build(aspectRatio));

    // Example Test shapes        
    shapes3D.push_back(ShapeBuilder3D::square()
            .withSize(256, 192)
            .placeAtCorner(corner_Center)
            .sourceScale(aspectRatio*aspectRatio, 1.0)
            .hudScale(SCREEN_SCALE)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .fromPosition(128, 140)
            .withSize(128, 52)
            .placeAtCorner(corner_BottomRight)
            .zRange(-1.0, 0.0)
            .hudScale(hudScale)
            .build(aspectRatio));

    shapes3D.push_back(ShapeBuilder3D::square()
            .polygonMode()
            .polygonAttributes(2031808)
            .fromPosition(0, 69)
            .withSize(80, 124)
            .placeAtCorner(corner_BottomLeft)
            .withMargin(10.0, 0.0, 0.0, 0.5)
            .zRange(-1.0, -1.0)
            .negateColor(0xFFFFFF)
            .hudScale(hudScale)
            .build(aspectRatio));
    
    printf("aspectRatio%f\n",aspectRatio);
    printf("aspectRatio*aspectRatio%f\n",aspectRatio*aspectRatio);
    
    for (int i = 0; i<shapes3D.size();i++){
        if (i>=_3DShapeBuffer.size()){
            printf("Ran out of 3DShapes in buffer at test number %i\n",i);
            allGood = -1;
            break;
        }
        ShapeData3D cppShape = shapes3D[i];
        ShapeData3D luaShape = _3DShapeBuffer[i];
        int cmpr = memcmp(&cppShape, &luaShape, sizeof(ShapeData3D));
        if (cmpr!=0){
            printf("3DShapeBuilder Test number %i failed...\n",i);
            allGood = -1;
        }
    }
    if (allGood == 1){
        printf("All 2D/3D ShapeBuilder Test passed!\n");
    }
    return allGood;
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