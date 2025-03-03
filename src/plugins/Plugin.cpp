#include "Plugin.h"

#include "Plugin_GPU_OpenGL_shaders.h"
#include "Plugin_GPU3D_OpenGL_shaders.h"

#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>

#ifdef __APPLE__
#include <objc/objc.h>
#include <objc/NSObject.h>
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#include "../OpenGLSupport.h"

#define RAM_SEARCH_ENABLED true
// #define RAM_SEARCH_SIZE 8
// #define RAM_SEARCH_SIZE 16
#define RAM_SEARCH_SIZE 32
#define RAM_SEARCH_LIMIT_MIN 0
// #define RAM_SEARCH_LIMIT_MAX 0x09FFFF
#define RAM_SEARCH_LIMIT_MAX 0x19FFFF
// #define RAM_SEARCH_LIMIT_MAX 0x3FFFFF
#define RAM_SEARCH_INTERVAL_MARGIN 0x050

// #define RAM_SEARCH_EXACT_VALUE     0x05B07E00
// #define RAM_SEARCH_EXACT_VALUE_MIN 0x05B07E00
// #define RAM_SEARCH_EXACT_VALUE_MAX 0x05BEE334

// WARNING: THE MACRO BELOW CAN ONLY BE USED ALONGSIDE RAM_SEARCH_EXACT_VALUE* MACROS,
// OTHERWISE IT WILL DO NOTHING BUT MAKE SEARCH IMPOSSIBLE, AND DECREASE THE FRAMERATE
#define RAM_SEARCH_EVERY_SINGLE_FRAME false

#if RAM_SEARCH_SIZE == 32
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read32(addr)
#elif RAM_SEARCH_SIZE == 16
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read16(addr)
#else
#define RAM_SEARCH_READ(nds,addr) nds->ARM7Read8(addr)
#endif

namespace Plugins
{

std::filesystem::path Plugin::assetsFolderPath()
{
    std::string assetsFolderName = assetsFolder();
#ifdef __APPLE__
    Class nsBundleClass = (Class)objc_getClass("NSBundle");
    SEL mainBundleSel = sel_registerName("mainBundle");
    SEL bundlePathSel = sel_registerName("bundlePath");
    SEL utf8StringSel = sel_registerName("UTF8String");

    id bundle = ((id(*)(Class, SEL))objc_msgSend)(nsBundleClass, mainBundleSel);
    id bundlePath = ((id(*)(id, SEL))objc_msgSend)(bundle, bundlePathSel);
    const char* pathCString = ((const char* (*)(id, SEL))objc_msgSend)(bundlePath, utf8StringSel);
    
    std::filesystem::path currentPath = std::filesystem::path(pathCString) / "Contents";
#else
    std::filesystem::path currentPath = std::filesystem::current_path();
#endif
    return currentPath / "assets" / assetsFolderName;
}

const char* Plugin::gpuOpenGL_FS()
{
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kCompositorFS_Plugin;
}

void Plugin::gpuOpenGL_FS_initVariables(GLuint CompShader) {
    GLint blockIndex = glGetUniformBlockIndex(CompShader, "ShapeBlock2D");
    glUniformBlockBinding(CompShader, blockIndex, 1);

    GLuint uboBuffer;
    glGenBuffers(1, &uboBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShapeData2D) * SHAPES_DATA_ARRAY_SIZE, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboBuffer);
    CompUboLoc[CompShader] = uboBuffer;

    CompGpuLoc[CompShader][0] = glGetUniformLocation(CompShader, "currentAspectRatio");
    CompGpuLoc[CompShader][1] = glGetUniformLocation(CompShader, "forcedAspectRatio");
    CompGpuLoc[CompShader][2] = glGetUniformLocation(CompShader, "hudScale");
    CompGpuLoc[CompShader][3] = glGetUniformLocation(CompShader, "showOriginalHud");
    CompGpuLoc[CompShader][4] = glGetUniformLocation(CompShader, "screenLayout");
    CompGpuLoc[CompShader][5] = glGetUniformLocation(CompShader, "brightnessMode");
    CompGpuLoc[CompShader][6] = glGetUniformLocation(CompShader, "shapeCount");

    for (int index = 0; index <= 6; index ++) {
        CompGpuLastValues[CompShader][index] = -1;
    }
}

#define UPDATE_GPU_VAR(storage,value,updated) if (storage != (value)) { storage = (value); updated = true; }

void Plugin::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    float forcedAspectRatio = renderer_forcedAspectRatio() / (4.f / 3.f);
    bool showOriginalHud = renderer_showOriginalUI();
    int screenLayout = renderer_screenLayout();
    int brightnessMode = renderer_brightnessMode();
    int gameSceneState = renderer_gameSceneState();

    bool updated = false;
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][0], (int)(aspectRatio*1000), updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][1], (int)(forcedAspectRatio*1000), updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][2], UIScale, updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][3], showOriginalHud ? 1 : 0, updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][4], screenLayout, updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][5], brightnessMode, updated);

    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][6], GameScene, updated);
    UPDATE_GPU_VAR(CompGpuLastValues[CompShader][7], gameSceneState, updated);

    if (updated) {
        std::vector<ShapeData2D> shapes = renderer_2DShapes(GameScene, gameSceneState);

#if DEBUG_MODE_ENABLED
        printf("Updating shapes. New shape count: %d\n", shapes.size());
        printf("Updated conditions: scene %d - state %d\n", GameScene, gameSceneState);
#endif

        glUniform1f(CompGpuLoc[CompShader][0], aspectRatio);
        glUniform1f(CompGpuLoc[CompShader][1], forcedAspectRatio);
        glUniform1i(CompGpuLoc[CompShader][2], CompGpuLastValues[CompShader][2]);
        glUniform1i(CompGpuLoc[CompShader][3], CompGpuLastValues[CompShader][3]);
        glUniform1i(CompGpuLoc[CompShader][4], CompGpuLastValues[CompShader][4]);
        glUniform1i(CompGpuLoc[CompShader][5], CompGpuLastValues[CompShader][5]);
        glUniform1i(CompGpuLoc[CompShader][6], shapes.size());

        shapes.resize(SHAPES_DATA_ARRAY_SIZE);
        auto shadersData = shapes.data();
        glBindBuffer(GL_UNIFORM_BUFFER, CompUboLoc[CompShader]);
        void* unibuf = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        if (unibuf) memcpy(unibuf, shadersData, sizeof(ShapeData2D) * shapes.size());
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
}

const char* Plugin::gpu3DOpenGLClassic_VS_Z() {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return nullptr;
    }

    return kRenderVS_Z_Plugin;
};

void Plugin::gpu3DOpenGLClassic_VS_Z_initVariables(GLuint CompShader, u32 flags)
{
    GLint blockIndex = glGetUniformBlockIndex(CompShader, "ShapeBlock3D");
    glUniformBlockBinding(CompShader, blockIndex, 2);

    if (CompUbo3DLocInit != true) {
#if DEBUG_MODE_ENABLED
        printf("Initializing 3D shapes UBO\n");
#endif
        GLuint uboBuffer;
        glGenBuffers(1, &uboBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(ShapeData3D) * SHAPES_DATA_ARRAY_SIZE, nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboBuffer);
        CompUbo3DLoc[CompShader] = uboBuffer;
        CompUbo3DLocInit = true;
    }

    CompGpu3DLoc[CompShader][flags][0] = glGetUniformLocation(CompShader, "currentAspectRatio");
    CompGpu3DLoc[CompShader][flags][1] = glGetUniformLocation(CompShader, "hudScale");
    CompGpu3DLoc[CompShader][flags][2] = glGetUniformLocation(CompShader, "shapeCount");

    for (int index = 0; index <= 2; index ++) {
        CompGpu3DLastValues[CompShader][flags][index] = -1;
    }
}

void Plugin::gpu3DOpenGLClassic_VS_Z_updateVariables(GLuint CompShader, u32 flags)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);
    int gameSceneState = renderer_gameSceneState();
    
    bool updated = false;
    UPDATE_GPU_VAR(CompGpu3DLastValues[CompShader][flags][0], (int)(aspectRatio*1000), updated);
    UPDATE_GPU_VAR(CompGpu3DLastValues[CompShader][flags][1], UIScale, updated);

    UPDATE_GPU_VAR(CompGpu3DLastValues[CompShader][flags][2], GameScene, updated);
    UPDATE_GPU_VAR(CompGpu3DLastValues[CompShader][flags][3], gameSceneState, updated);

    if (updated) {
        std::vector<ShapeData3D> shapes = renderer_3DShapes(GameScene, gameSceneState);

#if DEBUG_MODE_ENABLED
        printf("Updating 3D shapes. New shape count: %d\n", shapes.size());
#endif

        glUniform1f(CompGpu3DLoc[CompShader][flags][0], aspectRatio);
        glUniform1i(CompGpu3DLoc[CompShader][flags][1], CompGpu3DLastValues[CompShader][flags][1]);
        glUniform1i(CompGpu3DLoc[CompShader][flags][2], shapes.size());

        shapes.resize(SHAPES_DATA_ARRAY_SIZE);
        auto shadersData = shapes.data();
        glBindBuffer(GL_UNIFORM_BUFFER, CompUbo3DLoc[CompShader]);
        void* unibuf = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        if (unibuf) memcpy(unibuf, shadersData, sizeof(ShapeData3D) * shapes.size());
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }
}

#undef UPDATE_GPU_VAR

void Plugin::gpu3DOpenGLCompute_applyChangesToPolygon(int ScreenWidth, int ScreenHeight, s32 scaledPositions[10][2], melonDS::Polygon* polygon) {
    bool disable = DisableEnhancedGraphics;
    if (disable) {
        return;
    }

    float aspectRatio = AspectRatio / (4.f / 3.f);
    int resolutionScale = ScreenWidth/256;

    int gameSceneState = renderer_gameSceneState();
    std::vector<ShapeData3D> shapes = renderer_3DShapes(GameScene, gameSceneState);

    for (int shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
    {
        ShapeData3D shape = shapes[shapeIndex];
        bool loggerModeEnabled = (shape.effects & 0x4) != 0;

        bool attrMatch = true;
        for (int i = 0; i < 4; i++) {
            attrMatch = attrMatch && (shape.polygonAttributes[i] == 0 || shape.polygonAttributes[i] == polygon->Attr);
        }
        for (int i = 0; i < 4; i++) {
            attrMatch = attrMatch && (shape.negatedPolygonAttributes[i] == 0 || shape.negatedPolygonAttributes[i] != polygon->Attr);
        }

        // polygon mode
        if ((shape.effects & 0x1) != 0) {
            if (shape.polygonVertexesCount == 0 || shape.polygonVertexesCount == polygon->NumVertices) {
                if (attrMatch) {
                    s32 z = polygon->Vertices[0]->Position[2];
                    float _z = ((float)z)/(1 << 22);
                    if (_z >= shape.zRange.x && _z <= shape.zRange.y) 
                    {
                        u32 x0 = (int)scaledPositions[0][0];
                        u32 x1 = (int)scaledPositions[0][0];
                        for (int vIndex = 1; vIndex < polygon->NumVertices; vIndex++) {
                            x0 = std::min((int)x0, (int)scaledPositions[vIndex][0]);
                            x1 = std::max((int)x1, (int)scaledPositions[vIndex][0]);
                        }
                        float xCenter = (x0 + x1)/2.0;

                        for (int vIndex = 0; vIndex < polygon->NumVertices; vIndex++) {
                            scaledPositions[vIndex][0] = (u32)(xCenter + (s32)(((float)scaledPositions[vIndex][0] - xCenter)/aspectRatio));
                        }
                    }
                }
            }
        }
    }

    for (int vertexIndex = 0; vertexIndex < polygon->NumVertices; vertexIndex++)
    {
        s32* x = &scaledPositions[vertexIndex][0];
        s32* y = &scaledPositions[vertexIndex][1];
        s32 z = polygon->Vertices[vertexIndex]->Position[2];
        s32* rgb = polygon->Vertices[vertexIndex]->FinalColor;

        float _x = (float)(*x);
        float _y = (float)(*y);
        float _z = ((float)z)/(1 << 22);

        for (int shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
        {
            ShapeData3D shape = shapes[shapeIndex];

            // vertex mode
            if ((shape.effects & 0x1) == 0) {
                bool loggerModeEnabled = (shape.effects & 0x4) != 0;

                bool attrMatchEqual = false;
                bool attrMatchEqual2 = false;
                bool attrMatchNeg = false;
                for (int i = 0; i < 4; i++) {
                    if (shape.polygonAttributes[i] != 0) {
                        attrMatchEqual = true;
                        if (shape.polygonAttributes[i] == polygon->Attr) {
                            attrMatchEqual2 = true;
                            break;
                        }
                    }
                }
                for (int i = 0; i < 4; i++) {
                    if (shape.negatedPolygonAttributes[i] != 0 && shape.negatedPolygonAttributes[i] == polygon->Attr) {
                        attrMatchNeg = true;
                        break;
                    }
                }
                bool attrMatch = (attrMatchEqual ? attrMatchEqual2 : true) && !attrMatchNeg;

                bool colorMatchEqual = false;
                bool colorMatchEqual2 = false;
                bool colorMatchNeg = false;
                for (int i = 0; i < shape.colorCount; i++) {
                    colorMatchEqual = true;
                    if ((((shape.color[i] >> 8) & 0xFF) == (rgb[0] >> 1))
                    && (((shape.color[i] >> 4) & 0xFF) == (rgb[1] >> 1))
                    && (((shape.color[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
                        colorMatchEqual2 = true;
                        break;
                    }
                }
                for (int i = 0; i < shape.negatedColorCount; i++) {
                    if ((((shape.negatedColor[i] >> 8) & 0xFF) == (rgb[0] >> 1))
                    && (((shape.negatedColor[i] >> 4) & 0xFF) == (rgb[1] >> 1))
                    && (((shape.negatedColor[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
                        colorMatchNeg = true;
                        break;
                    }
                }
                bool colorMatch = (colorMatchEqual ? !colorMatchEqual2 : true) && !colorMatchNeg;

                float iuTexScale = SCREEN_SCALE/shape.hudScale;
                float xScaleInv = iuTexScale/shape.sourceScale.x;
                float yScaleInv = iuTexScale/shape.sourceScale.y;

                if (_x >= shape.squareInitialCoords.x*resolutionScale && _x <= (shape.squareInitialCoords.x + shape.squareInitialCoords.z)*resolutionScale &&
                    _y >= shape.squareInitialCoords.y*resolutionScale && _y <= (shape.squareInitialCoords.y + shape.squareInitialCoords.w)*resolutionScale &&
                    _z >= shape.zRange.x && _z <= shape.zRange.y && attrMatch && colorMatch)
                {
                    // hide vertex
                    if ((shape.effects & 0x2) != 0)
                    {
                        _x = 0;
                        _y = 0;
                    }
                    else {
                        switch (shape.corner)
                        {
                            case corner_PreservePosition:
                                break;

                            case corner_Center:
                                _x = ScreenWidth/2 + (_x - ScreenWidth/2)/(xScaleInv*aspectRatio);
                                _y = ScreenHeight/2 + (_y - ScreenHeight/2)/(yScaleInv);
                                break;
                            
                            case corner_TopLeft:
                                _x = _x/(xScaleInv*aspectRatio);
                                _y = _y/(yScaleInv);
                                break;
                            
                            case corner_Top:
                                _x = ScreenWidth/2 + (_x - ScreenWidth/2)/(xScaleInv*aspectRatio);
                                _y = _y/(yScaleInv);
                                break;

                            case corner_TopRight:
                                _x = ScreenWidth - (ScreenWidth - _x)/(xScaleInv*aspectRatio);
                                _y = _y/(yScaleInv);
                                break;

                            case corner_Right:
                                _x = ScreenWidth - (ScreenWidth - _x)/(xScaleInv*aspectRatio);
                                _y = ScreenHeight/2 + (_y - ScreenHeight/2)/(yScaleInv);
                                break;

                            case corner_BottomRight:
                                _x = ScreenWidth - (ScreenWidth - _x)/(xScaleInv*aspectRatio);
                                _y = ScreenHeight - ((ScreenHeight - _y)/(yScaleInv));
                                break;

                            case corner_Bottom:
                                _x = ScreenWidth/2 + (_x - ScreenWidth/2)/(xScaleInv*aspectRatio);
                                _y = ScreenHeight - ((ScreenHeight - _y)/(yScaleInv));
                                break;

                            case corner_BottomLeft:
                                _x = _x/(xScaleInv*aspectRatio);
                                _y = ScreenHeight - ((ScreenHeight - _y)/(yScaleInv));
                                break;

                            case corner_Left:
                                _x = _x/(xScaleInv*aspectRatio);
                                _y = ScreenHeight/2 + (_y - ScreenHeight/2)/(yScaleInv);
                                break;
                        }

                        _x = _x + (shape.margin.x*resolutionScale - shape.margin.z*resolutionScale)/aspectRatio;
                        _y = _y + shape.margin.y*resolutionScale - shape.margin.w*resolutionScale;
                    }

                    *x = (s32)(_x);
                    *y = (s32)(_y);
                    break;
                }
            }
        }
    }
};

bool Plugin::togglePause()
{
    if (_RunningReplacementCutscene) {
        if (_PausedReplacementCutscene) {
            _ShouldUnpauseReplacementCutscene = true;
        }
        else {
            _ShouldPauseReplacementCutscene = true;
        }
        return true;
    }
    if (_RunningReplacementBgmMusic) {
        if (_PausedReplacementBgmMusic) {
            _ShouldUnpauseReplacementBgmMusic = true;
        }
        else {
            _ShouldPauseReplacementBgmMusic = true;
        }
    }
    return false;
}

bool Plugin::_superApplyHotkeyToInputMask(u32* InputMask, u32* HotkeyMask, u32* HotkeyPress)
{
    ramSearch(nds, *HotkeyPress);

    if (_IsUnskippableCutscene)
    {
        *InputMask = 0xFFF;
        return false;
    }

    if (_RunningReplacementCutscene && !_PausedReplacementCutscene && (_SkipDsCutscene || (~(*InputMask)) & (1 << 3)) && _CanSkipHdCutscene) { // Start (skip HD cutscene)
        _SkipDsCutscene = true;
        if (!_ShouldTerminateIngameCutscene) { // can only skip after DS cutscene was skipped
            _SkipDsCutscene = false;
            _CanSkipHdCutscene = false;
            _ShouldStopReplacementCutscene = true;
            *InputMask |= (1<<3);
        }
        else {
            if (_StartPressCount == 0) {
                bool requiresDoubleStart = (_CurrentCutscene->dsScreensState & 4) == 4;
                if (requiresDoubleStart) {
                    _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT*2 + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT;
                }
                else {
                    _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT;
                }
            }
        }
    }

    if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene) {
        if (_StartPressCount > 0) {
            _StartPressCount--;

            bool requiresDoubleStart = (_CurrentCutscene->dsScreensState & 4) == 4;
            if (requiresDoubleStart) {
                if (_StartPressCount < CUTSCENE_SKIP_START_FRAMES_COUNT || _StartPressCount > CUTSCENE_SKIP_START_FRAMES_COUNT + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT) {
                    *InputMask &= ~(1<<3); // Start (skip DS cutscene)
                }
            }
            else {
                *InputMask &= ~(1<<3); // Start (skip DS cutscene)
            }
        }
    }

    return true;
}

void Plugin::applyHotkeyToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    bool shouldContinue = _superApplyHotkeyToInputMask(InputMask, HotkeyMask, HotkeyPress);
    if (!shouldContinue) {
        return;
    }
}

void Plugin::_superApplyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask, u16 sensitivity, bool resetOnEdge)
{
    u16 rStrength = 4 - sensitivity;
    u16 right = ((~TouchKeyMask) & 0xF) >> rStrength;
    u16 left  = (((~TouchKeyMask) >> 4)  & 0xF) >> rStrength;
    u16 down  = (((~TouchKeyMask) >> 8)  & 0xF) >> rStrength;
    u16 up    = (((~TouchKeyMask) >> 12) & 0xF) >> rStrength;

    u16 TouchX = *touchX;
    u16 TouchY = *touchY;
    bool noMovement = left == 0 && right == 0 && up == 0 && down == 0;
    if (noMovement) {
        if (_LastTouchScreenMovementWasByPlugin) {
            *isTouching = false;
            _LastTouchScreenMovementWasByPlugin = false;
            return;
        }
        return;
    }

    bool resetTouchScreen = false;
    if (*isTouching == false) {
        if (noMovement) {
            return;
        }

        TouchX = 256/2;
        TouchY = 192/2;
        *isTouching = true;
    }

    if (left)
    {
        if (TouchX <= left)
        {
            resetTouchScreen = resetOnEdge;
        }
        else
        {
            TouchX -= left;
        }
    }
    if (right)
    {
        if (TouchX + right >= 255)
        {
            resetTouchScreen = resetOnEdge;
        }
        else
        {
            TouchX += right;
        }
    }
    if (down)
    {
        if (TouchY <= down)
        {
            resetTouchScreen = resetOnEdge;
        }
        else
        {
            TouchY -= down;
        }
    }
    if (up)
    {
        if (TouchY + up >= 191)
        {
            resetTouchScreen = resetOnEdge;
        }
        else
        {
            TouchY += up;
        }
    }

    if (resetTouchScreen)
    {
        *isTouching = false;
    }
    else {
        *touchX = TouchX;
        *touchY = TouchY;
    }
    _LastTouchScreenMovementWasByPlugin = true;
}

void Plugin::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, 3, true);
}

std::string trim(const std::string& str) {
    // Find the first non-whitespace character from the beginning
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return ""; // Return empty string if no non-whitespace character is found
    }

    // Find the first non-whitespace character from the end
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    // Return the substring that excludes leading and trailing whitespace
    return str.substr(start, end - start + 1);
}
std::string Plugin::textureIndexFilePath() {
    std::string filename = "index.ini";
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    std::filesystem::path fullPath = texturesFolder / filename;

    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    return fullPath.string();
}
std::map<std::string, std::string> Plugin::getTexturesIndex() {
    if (!texturesIndex.empty()) {
        return texturesIndex;
    }

    std::map<std::string, std::string> _texturesIndex;
    std::string indexFilePath = textureIndexFilePath();
    if (indexFilePath.empty()) {
        return texturesIndex;
    }

    Platform::FileHandle* f = Platform::OpenLocalFile(indexFilePath.c_str(), Platform::FileMode::ReadText);
    if (f) {
        char linebuf[1024];
        char entryname[32];
        char entryval[1024];
        while (!Platform::IsEndOfFile(f))
        {
            if (!Platform::FileReadLine(linebuf, 1024, f))
                break;

            int ret = sscanf(linebuf, "%31[A-Za-z_0-9\\-]=%[^\t\r\n]", entryname, entryval);
            entryname[31] = '\0';
            if (ret < 2) continue;

            std::string entrynameStr = trim(std::string(entryname));
            std::string entryvalStr = trim(std::string(entryval));
            if (!entrynameStr.empty() && entrynameStr.compare(0, 1, ";") != 0 && entrynameStr.compare(0, 1, "[") != 0) {
                _texturesIndex[entrynameStr] = entryvalStr;
            }
        }
    }
    texturesIndex = _texturesIndex;
    return texturesIndex;
}
std::string Plugin::textureFilePath(std::string texture) {
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    if (!std::filesystem::exists(_assetsFolderPath)) {
        std::filesystem::create_directory(_assetsFolderPath);
    }

    std::map<std::string, std::string> texturesIndex = getTexturesIndex();
    if (texturesIndex.count(texture)) {
        std::filesystem::path fullPath = texturesFolder / texturesIndex[texture];
        if (!std::filesystem::exists(fullPath)) {
            errorLog("Texture %s was supposed to be replaced by %s, but it doesn't exist", texture.c_str(), fullPath.string().c_str());
            return "";
        }

        return fullPath.string();
    }

    std::filesystem::path fullPath = texturesFolder / (texture + ".png");
    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    texturesIndex[texture] = texture + ".png";
    return fullPath.string();
}
std::string Plugin::tmpTextureFilePath(std::string texture) {
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path tmpFolderPath = _assetsFolderPath / "textures_tmp";

    if (shouldExportTextures() && !std::filesystem::exists(tmpFolderPath)) {
        std::filesystem::create_directory(tmpFolderPath);
    }

    std::filesystem::path fullPathTmp = tmpFolderPath / (texture + ".png");
    return fullPathTmp.string();
}

bool Plugin::ShouldTerminateIngameCutscene() {return _ShouldTerminateIngameCutscene;}
bool Plugin::StoppedIngameCutscene() {
    if (_StoppedIngameCutscene) {
        _StoppedIngameCutscene = false;
        return true;
    }
    return false;
}
bool Plugin::ShouldStartReplacementCutscene() {
    if (_ShouldStartReplacementCutscene) {
        _ShouldStartReplacementCutscene = false;
        return true;
    }
    return false;
}
bool Plugin::StartedReplacementCutscene() {
    if (_StartedReplacementCutscene) {
        _StartedReplacementCutscene = false;
        return true;
    }
    return false;
}
bool Plugin::RunningReplacementCutscene() {return _RunningReplacementCutscene;}
bool Plugin::ShouldPauseReplacementCutscene() {
    if (_ShouldPauseReplacementCutscene) {
        _ShouldPauseReplacementCutscene = false;
        _PausedReplacementCutscene = true;
        return true;
    }
    return false;
}
bool Plugin::ShouldUnpauseReplacementCutscene() {
    if (_ShouldUnpauseReplacementCutscene) {
        _ShouldUnpauseReplacementCutscene = false;
        _PausedReplacementCutscene = false;
        return true;
    }
    return false;
}
bool Plugin::ShouldStopReplacementCutscene() {
    if (_ShouldStopReplacementCutscene) {
        _ShouldStopReplacementCutscene = false;
        return true;
    }
    return false;
}
bool Plugin::ShouldReturnToGameAfterCutscene() {return _ShouldReturnToGameAfterCutscene;}
bool Plugin::ShouldUnmuteAfterCutscene() {
    if (_ShouldUnmuteAfterCutscene) {
        _ShouldUnmuteAfterCutscene = false;
        return true;
    }
    return false;
}
CutsceneEntry* Plugin::CurrentCutscene() {return _CurrentCutscene;};

CutsceneEntry* Plugin::detectTopScreenMobiCutscene()
{
    if (GameScene == -1)
    {
        return nullptr;
    }

    u32 cutsceneAddressValue = 0;
    u32 cutsceneAddress = detectTopScreenMobiCutsceneAddress();
    if (cutsceneAddress != 0) {
        cutsceneAddressValue = nds->ARM7Read32(cutsceneAddress);
        if (cutsceneAddressValue == 0 || (cutsceneAddressValue - (cutsceneAddressValue & 0xFF)) == 0xea000000) {
            cutsceneAddressValue = 0;
        }
    }

    return getMobiCutsceneByAddress(cutsceneAddressValue);
}

CutsceneEntry* Plugin::detectBottomScreenMobiCutscene()
{
    if (GameScene == -1)
    {
        return nullptr;
    }

    u32 cutsceneAddressValue2 = 0;
    u32 cutsceneAddress2 = detectBottomScreenMobiCutsceneAddress();
    if (cutsceneAddress2 != 0) {
        cutsceneAddressValue2 = nds->ARM7Read32(cutsceneAddress2);
        if (cutsceneAddressValue2 == 0 || (cutsceneAddressValue2 - (cutsceneAddressValue2 & 0xFF)) == 0xea000000) {
            cutsceneAddressValue2 = 0;
        }
    }

    return getMobiCutsceneByAddress(cutsceneAddressValue2);
}
CutsceneEntry* Plugin::detectCutscene()
{
    CutsceneEntry* cutscene1 = detectTopScreenMobiCutscene();
    CutsceneEntry* cutscene2 = detectBottomScreenMobiCutscene();

    if (cutscene1 == nullptr && cutscene2 != nullptr) {
        cutscene1 = cutscene2;
    }

    return cutscene1;
}

void Plugin::refreshCutscene()
{
#if !REPLACEMENT_CUTSCENES_ENABLED
    return;
#endif

    bool isCutsceneScene = isCutsceneGameScene();
    CutsceneEntry* cutscene = detectCutscene();

    if (_ReplayLimitCount > 0) {
        _ReplayLimitCount--;
        if (cutscene != nullptr && cutscene->usAddress == _LastCutscene->usAddress) {
            cutscene = nullptr;
        }
    }

    
    if (cutscene != nullptr) {
        onIngameCutsceneIdentified(cutscene);
    }

    // Natural progression for all cutscenes
    if (_ShouldTerminateIngameCutscene && !_RunningReplacementCutscene && isCutsceneScene) {
        _ShouldStartReplacementCutscene = true;
    }

    if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene && didMobiCutsceneEnded()) {
        onTerminateIngameCutscene();
    }

    if (_ShouldReturnToGameAfterCutscene && canReturnToGameAfterReplacementCutscene()) {
        onReturnToGameAfterCutscene();
    }
}

void Plugin::onIngameCutsceneIdentified(CutsceneEntry* cutscene) {
    if (_CurrentCutscene != nullptr && _CurrentCutscene->usAddress == cutscene->usAddress) {
        return;
    }

    std::string path = replacementCutsceneFilePath(cutscene);
    if (path == "") {
        return;
    }

    if (_CurrentCutscene != nullptr) {
        _NextCutscene = cutscene;
        return;
    }

    printf("Preparing to load cutscene: %s\n", cutscene->Name);

    _CanSkipHdCutscene = true;
    _CurrentCutscene = cutscene;
    _NextCutscene = nullptr;
    _ShouldTerminateIngameCutscene = true;
    _IsUnskippableCutscene = isUnskippableMobiCutscene(cutscene);
}
void Plugin::onTerminateIngameCutscene() {
    if (_CurrentCutscene == nullptr) {
        return;
    }
    printf("Ingame cutscene terminated\n");
    _ShouldTerminateIngameCutscene = false;
    _StoppedIngameCutscene = true;

    if (_IsUnskippableCutscene) {
        _StoppedIngameCutscene = false;
    }
}
void Plugin::onReplacementCutsceneStarted() {
    printf("Cutscene started\n");
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = true;
    _RunningReplacementCutscene = true;
}

void Plugin::onReplacementCutsceneEnd() {
    printf("Replacement cutscene ended\n");
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldStopReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = true;
    _ShouldHideScreenForTransitions = false;
}
void Plugin::onReturnToGameAfterCutscene() {
    printf("Returning to the game\n");
    _StartPressCount = 0;
    _IsUnskippableCutscene = false;
    _ShouldStartReplacementCutscene = false;
    _StartedReplacementCutscene = false;
    _RunningReplacementCutscene = false;
    _ShouldReturnToGameAfterCutscene = false;
    _ShouldUnmuteAfterCutscene = true;

    _LastCutscene = _CurrentCutscene;
    _CurrentCutscene = nullptr;
    _ReplayLimitCount = 30;

    if (_NextCutscene == nullptr) {
        u32 cutsceneAddress = detectTopScreenMobiCutsceneAddress();
        if (cutsceneAddress != 0) {
            nds->ARM7Write32(cutsceneAddress, 0x0);
        }

        u32 cutsceneAddress2 = detectBottomScreenMobiCutsceneAddress();
        if (cutsceneAddress2 != 0) {
            nds->ARM7Write32(cutsceneAddress2, 0x0);
        }
    }
}

bool Plugin::ShouldStartReplacementBgmMusic() {
    if (_ShouldStartReplacementBgmMusic) {
        _ShouldStartReplacementBgmMusic = false;
        return true;
    }
    return false;
}
int Plugin::delayBeforeStartReplacementBackgroundMusic() {
    return 0;
}
bool Plugin::StartedReplacementBgmMusic() {
    if (_StartedReplacementBgmMusic) {
        _StartedReplacementBgmMusic = false;
        return true;
    }
    return false;
}
bool Plugin::RunningReplacementBgmMusic() {return _RunningReplacementBgmMusic;}
bool Plugin::ShouldPauseReplacementBgmMusic() {
    if (_ShouldPauseReplacementBgmMusic) {
        _ShouldPauseReplacementBgmMusic = false;
        _PausedReplacementBgmMusic = true;
        return true;
    }
    return false;
}
bool Plugin::ShouldUnpauseReplacementBgmMusic() {
    if (_ShouldUnpauseReplacementBgmMusic) {
        _ShouldUnpauseReplacementBgmMusic = false;
        _PausedReplacementBgmMusic = false;
        return true;
    }
    return false;
}
bool Plugin::ShouldStopReplacementBgmMusic() {
    if (_ShouldStopReplacementBgmMusic) {
        _ShouldStopReplacementBgmMusic = false;
        return true;
    }
    return false;
}
u16 Plugin::CurrentBackgroundMusic() {return _CurrentBackgroundMusic;};

void Plugin::onReplacementBackgroundMusicStarted() {
    printf("Background music started\n");
    _ShouldStartReplacementBgmMusic = false;
    _StartedReplacementBgmMusic = true;
    _RunningReplacementBgmMusic = true;
}

bool Plugin::ShouldGrabMouseCursor() {
    if (_ShouldGrabMouseCursor) {
        _ShouldGrabMouseCursor = false;
        _MouseCursorIsGrabbed = true;
        return true;
    }
    return false;
}
bool Plugin::ShouldReleaseMouseCursor() {
    if (_ShouldReleaseMouseCursor) {
        _ShouldReleaseMouseCursor = false;
        _MouseCursorIsGrabbed = false;
        return true;
    }
    return false;
}
bool Plugin::isMouseCursorGrabbed() {
    return _MouseCursorIsGrabbed;
}

bool Plugin::_superShouldRenderFrame()
{
    if (_ShouldTerminateIngameCutscene && _RunningReplacementCutscene)
    {
        return false;
    }

    return true;
}

bool Plugin::setGameScene(int newGameScene)
{
    bool updated = false;
    if (GameScene != newGameScene) 
    {
        updated = true;

        // Game scene
        PriorGameScene = GameScene;
        GameScene = newGameScene;
    }

    return updated;
}

bool Plugin::refreshGameScene()
{
    int newGameScene = detectGameScene();
    
    debugLogs(newGameScene);

    bool updated = setGameScene(newGameScene);

    refreshCutscene();

    refreshBackgroundMusic();

    refreshMouseStatus();

    return updated;
}

void Plugin::setAspectRatio(float aspectRatio)
{
    if (GameScene != -1)
    {
        int aspectRatioKey = (int)round(0x1000 * aspectRatio);

        u32 aspectRatioMenuAddress = getAspectRatioAddress();

        if (aspectRatioMenuAddress != 0 && nds->ARM7Read16(aspectRatioMenuAddress) == 0x00001555) {
            nds->ARM7Write16(aspectRatioMenuAddress, aspectRatioKey);
        }
    }

    AspectRatio = aspectRatio;
}
void Plugin::setInternalResolutionScale(int scale)
{
    InternalResolutionScale = scale;
}

void Plugin::_superLoadConfigs(
    std::function<bool(std::string)> getBoolConfig,
    std::function<int(std::string)> getIntConfig,
    std::function<std::string(std::string)> getStringConfig
)
{
    std::string root = tomlUniqueIdentifier();
    DisableEnhancedGraphics = getBoolConfig(root + ".DisableEnhancedGraphics");
    ExportTextures = getBoolConfig(root + ".ExportTextures");
    FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    UIScale = getIntConfig(root + ".HUDScale");
    UIScale = (UIScale == 0) ? 4 : UIScale;
}
void Plugin::loadConfigs(
    std::function<bool(std::string)> getBoolConfig,
    std::function<int(std::string)> getIntConfig,
    std::function<std::string(std::string)> getStringConfig
)
{
    _superLoadConfigs(getBoolConfig, getIntConfig, getStringConfig);
}

void Plugin::errorLog(const char* format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (size <= 0) {
        va_end(args);
        return;
    }

    std::string result(size, '\0');
    std::vsnprintf(&result[0], size + 1, format, args);

    va_end(args);

    const char* log = result.c_str();
    printf("%s\n", log);

    if (ERROR_LOG_FILE_ENABLED) {
        std::string fileName = std::string("error.log");
        Platform::FileHandle* logf = Platform::OpenFile(fileName, Platform::FileMode::Append);
        Platform::FileWrite(log, strlen(log), 1, logf);
        Platform::FileWrite("\n", 1, 1, logf);
        Platform::CloseFile(logf);
    }
}

void Plugin::ramSearch(melonDS::NDS* nds, u32 HotkeyPress) {
#if !RAM_SEARCH_ENABLED
    return;
#endif

    int byteSize = RAM_SEARCH_SIZE/8;
    u32 limitMin = RAM_SEARCH_LIMIT_MIN;
    u32 limitMax = RAM_SEARCH_LIMIT_MAX;
    if (RAM_SEARCH_EVERY_SINGLE_FRAME || HotkeyPress & (1 << 12)) { // HK_PowerButton (reset RAM search)
        if (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
            printf("Resetting RAM search\n");
        }
        for (u32 index = limitMin; index < limitMax; index+=byteSize) {
            u32 addr = (0x02000000 | index);
            u32 newVal = RAM_SEARCH_READ(nds, addr);
            MainRAMState[index] = true;
#ifdef RAM_SEARCH_EXACT_VALUE
            MainRAMState[index] = RAM_SEARCH_EXACT_VALUE == newVal;
#endif
#ifdef RAM_SEARCH_EXACT_VALUE_MIN
            if (newVal < RAM_SEARCH_EXACT_VALUE_MIN) {
                MainRAMState[index] = false;
            }
#endif
#ifdef RAM_SEARCH_EXACT_VALUE_MAX
            if (newVal > RAM_SEARCH_EXACT_VALUE_MAX) {
                MainRAMState[index] = false;
            }
#endif
            LastMainRAM[index] = newVal;
        }
    }
    if (HotkeyPress & (1 << 13)) { // HK_VolumeUp (filter RAM by equal values)
        printf("Filtering RAM by equal values\n");
        for (u32 index = limitMin; index < limitMax; index+=byteSize) {
            u32 addr = (0x02000000 | index);
            u32 newVal = RAM_SEARCH_READ(nds, addr);
            MainRAMState[index] = MainRAMState[index] && (LastMainRAM[index] == newVal);
            LastMainRAM[index] = newVal;
        }
    }
    if (HotkeyPress & (1 << 14)) { // HK_VolumeDown (filter RAM by different values)
        printf("Filtering RAM by different values\n");
        for (u32 index = limitMin; index < limitMax; index+=byteSize) {
            u32 addr = (0x02000000 | index);
            u32 newVal = RAM_SEARCH_READ(nds, addr);
            MainRAMState[index] = MainRAMState[index] && (LastMainRAM[index] != newVal);
            LastMainRAM[index] = newVal;
        }
    }
    if (RAM_SEARCH_EVERY_SINGLE_FRAME || HotkeyPress & (1 << 12) || HotkeyPress & (1 << 13) || HotkeyPress & (1 << 14)) {
        int total = 0;
        for (u32 index = limitMin; index < limitMax; index+=byteSize) {
            if (MainRAMState[index]) {
                total += 1;
            }
        }
        if (total > 0) {
            if (total < 50*(4/byteSize)) {
                for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                    u32 addr = (0x02000000 | index);
                    if (MainRAMState[index]) {
                        printf("0x%08x: %d\n", addr, LastMainRAM[index]);
                    }
                }
                printf("\n");
            }
            else {
                int validDistance = RAM_SEARCH_INTERVAL_MARGIN;
                u32 firstAddr = 0;
                u32 lastAddr = 0;
                for (u32 index = (limitMin == 0 ? byteSize : limitMin); index < limitMax; index += byteSize) {
                    u32 addr = (0x02000000 | index);
                    if (MainRAMState[index]) {
                        if (firstAddr == 0) {
                            firstAddr = addr;
                            lastAddr = addr;
                        }
                        else {
                            lastAddr = addr;
                        }
                    }
                    else {
                        if (firstAddr != 0 && lastAddr < (addr - byteSize*validDistance)) {
                            if (firstAddr == lastAddr) {
                                printf("0x%08x\n", firstAddr);
                            }
                            else {
                                printf("0x%08x - 0x%08x\n", firstAddr, lastAddr);
                            }
                            firstAddr = 0;
                            lastAddr = 0;
                        }
                    }
                }
                if (firstAddr != 0) {
                    if (firstAddr == lastAddr) {
                        printf("0x%08x\n", firstAddr);
                    }
                    else {
                        printf("0x%08x - 0x%08x\n", firstAddr, lastAddr);
                    }
                }
                printf("\n");
            }
        }
        if (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
            printf("Addresses matching the search: %d\n", total);
        }
    }
}

}