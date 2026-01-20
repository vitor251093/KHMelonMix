#include "Plugin.h"

#include "Plugin_GPU_OpenGL_shaders.h"
#include "AudioUtils.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>

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
#define RAM_SEARCH_MAX_RESULTS 50
#define RAM_SEARCH_LIMIT_MIN 0
#define RAM_SEARCH_LIMIT_MAX 0x19FFFF
// #define RAM_SEARCH_LIMIT_MAX 0x19FFFF
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

u16 Plugin::BGM_INVALID_ID = 0xFFFF;

void Plugin::onLoadROM() {
    loadBgmRedirections();

    stopBackgroundMusic(0);
    _SoundtrackState = EMidiState::Stopped;
}

void Plugin::onLoadState() {
    texturesIndex.clear();

    stopBackgroundMusic(0);
    _SoundtrackState = EMidiState::Stopped;
};

std::filesystem::path Plugin::gameAssetsFolderPath()
{
    std::string assetsFolderName = gameFolderName();

    std::filesystem::path assetsPath = _AssetsFolderPath;
    if (assetsPath.empty())
    {
        const char* assetsPathEnv = std::getenv("MELON_MIX_ASSETS");
        if (assetsPathEnv != nullptr)
        {
            assetsPath = std::filesystem::path(assetsPathEnv);
            _AssetsFolderPath = assetsPath;
        }
    }
    if (assetsPath.empty())
    {
#ifdef __APPLE__
        Class nsBundleClass = (Class)objc_getClass("NSBundle");
        SEL mainBundleSel = sel_registerName("mainBundle");
        SEL bundlePathSel = sel_registerName("bundlePath");
        SEL utf8StringSel = sel_registerName("UTF8String");

        id bundle = ((id(*)(Class, SEL))objc_msgSend)(nsBundleClass, mainBundleSel);
        id bundlePath = ((id(*)(id, SEL))objc_msgSend)(bundle, bundlePathSel);
        const char* pathCString = ((const char* (*)(id, SEL))objc_msgSend)(bundlePath, utf8StringSel);

        assetsPath = std::filesystem::path(pathCString) / "Contents";
#else
        assetsPath = std::filesystem::current_path();
#endif

        if (!std::filesystem::exists(assetsPath / "assets") &&
             std::filesystem::exists(assetsPath / "Image" / "melon" / "assets"))
        {
            // Fallback for Refined Launcher
            assetsPath = assetsPath / "Image" / "melon";
        }

        assetsPath = assetsPath / "assets";
        _AssetsFolderPath = assetsPath;
    }

    if (!std::filesystem::exists(assetsPath))
    {
        try {
            std::filesystem::create_directory(assetsPath);
        }
        catch (const std::runtime_error& ignored) {
            printf("Failed to create assets folder. Replacement assets are unavailable");
        }
    }

    return assetsPath / assetsFolderName;
}

const char* Plugin::gpuOpenGL_FS()
{
    bool disable = !EnhancedGraphics;
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
}

void Plugin::gpuOpenGL_FS_updateVariables(GLuint CompShader) {
    float aspectRatio = AspectRatio / (4.f / 3.f);
    float forcedAspectRatio = renderer_forcedAspectRatio() / (4.f / 3.f);
    bool showOriginalHud = renderer_showOriginalUI();
    int screenLayout = renderer_screenLayout();
    int brightnessMode = renderer_brightnessMode();

    glUniform1f(CompGpuLoc[CompShader][0], aspectRatio);
    glUniform1f(CompGpuLoc[CompShader][1], forcedAspectRatio);
    glUniform1i(CompGpuLoc[CompShader][2], UIScale);
    glUniform1i(CompGpuLoc[CompShader][3], showOriginalHud ? 1 : 0);
    glUniform1i(CompGpuLoc[CompShader][4], screenLayout);
    glUniform1i(CompGpuLoc[CompShader][5], brightnessMode);
    glUniform1i(CompGpuLoc[CompShader][6], current2DShapes.size());

    current2DShapes.resize(SHAPES_DATA_ARRAY_SIZE);
    auto shadersData = current2DShapes.data();
    glBindBuffer(GL_UNIFORM_BUFFER, CompUboLoc[CompShader]);
    void* unibuf = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    if (unibuf) memcpy(unibuf, shadersData, sizeof(ShapeData2D) * current2DShapes.size());
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

bool Plugin::gpuOpenGL_applyChangesToPolygonVertex(int resolutionScale, s32 scaledPositions[10][2], melonDS::Polygon* polygon, ShapeData3D shape, int vertexIndex)
{
    float aspectRatio = AspectRatio / (4.f / 3.f);

    s32* x = &scaledPositions[vertexIndex][0];
    s32* y = &scaledPositions[vertexIndex][1];
    s32 z = polygon->Vertices[vertexIndex]->Position[2];
    s32* rgb = polygon->Vertices[vertexIndex]->FinalColor;

    auto _x = (float)(*x);
    auto _y = (float)(*y);
    float _z = ((float)z)/(1 << 22);

    bool loggerModeEnabled = (shape.effects & 0x4) != 0;

    vec3 newValues = shape.compute3DCoordinatesOf3DSquareShapeInVertexMode(_x, _y, _z, polygon->Attr, polygon->TexParam, rgb, resolutionScale, aspectRatio);
    if (newValues.z == 1) {
        if (loggerModeEnabled) {
            printf("Old Position: %f - %f -- Attribute: %d -- New Position: %f - %f\n", _x, _y, polygon->Attr, newValues.x, newValues.y);
        }

        *x = (s32)(newValues.x);
        *y = (s32)(newValues.y);
        return true;
    }

    return false;
}

bool Plugin::gpuOpenGL_applyChangesToPolygon(int resolutionScale, s32 scaledPositions[10][2], melonDS::Polygon* polygon) {
    bool disable = !EnhancedGraphics;
    if (disable) {
        return false;
    }

    float aspectRatio = AspectRatio / (4.f / 3.f);

    bool atLeastOneLog = false;
    for (auto shape : current3DShapes)
    {
        bool loggerModeEnabled = (shape.effects & 0x4) != 0;

        // polygon mode
        if ((shape.effects & 0x1) != 0) {
            if (shape.polygonVertexesCount == 0 || shape.polygonVertexesCount == polygon->NumVertices) {
                if (shape.doesAttributeMatch(polygon->Attr) && shape.doesTextureParamMatch(polygon->TexParam)) {
                    u32 x0 = (int)scaledPositions[0][0];
                    u32 x1 = (int)scaledPositions[0][0];
                    u32 y0 = (int)scaledPositions[0][1];
                    u32 y1 = (int)scaledPositions[0][1];
                    for (int vIndex = 1; vIndex < polygon->NumVertices; vIndex++) {
                        x0 = std::min((int)x0, (int)scaledPositions[vIndex][0]);
                        x1 = std::max((int)x1, (int)scaledPositions[vIndex][0]);
                        y0 = std::min((int)y0, (int)scaledPositions[vIndex][1]);
                        y1 = std::max((int)y1, (int)scaledPositions[vIndex][1]);
                    }
                    s32 z = polygon->Vertices[0]->Position[2];
                    float _z = ((float)z)/(1 << 22);
                    if (shape.squareInitialCoords.x*resolutionScale <= x0 && x1 <= (shape.squareInitialCoords.x + shape.squareInitialCoords.z)*resolutionScale &&
                        shape.squareInitialCoords.y*resolutionScale <= y0 && y1 <= (shape.squareInitialCoords.y + shape.squareInitialCoords.w)*resolutionScale &&
                        _z >= shape.zRange.x && _z <= shape.zRange.y)
                    {
                        if (shape.doesColorMatch(polygon->Vertices[0]->FinalColor))
                        {
                            if (loggerModeEnabled) {
                                atLeastOneLog = true;
                                printf("Position: %d - %d -- Size: %d - %d\n", x0, y0, x1 - x0, y1 - y0);
                            }

                            if ((shape.effects & 0x8) != 0)
                            {
                                float xCenter = (x0 + x1)/2.0;
                                for (int vIndex = 0; vIndex < polygon->NumVertices; vIndex++) {
                                    scaledPositions[vIndex][0] = (u32)(xCenter + (s32)(((float)scaledPositions[vIndex][0] - xCenter)/aspectRatio));
                                }
                            }
                            else
                            {
                                for (int vIndex = 0; vIndex < polygon->NumVertices; vIndex++) {
                                    gpuOpenGL_applyChangesToPolygonVertex(resolutionScale, scaledPositions, polygon, shape, vIndex);
                                }
                            }

                            return true;
                        }
                    }
                }
            }
        }
    }

    bool changed = false;
    for (int vertexIndex = 0; vertexIndex < polygon->NumVertices; vertexIndex++)
    {
        for (auto shape : current3DShapes)
        {
            // vertex mode
            if ((shape.effects & 0x1) == 0)
            {
                bool loggerModeEnabled = (shape.effects & 0x4) != 0;
                bool thisChanged = gpuOpenGL_applyChangesToPolygonVertex(resolutionScale, scaledPositions, polygon, shape, vertexIndex);
                changed |= thisChanged;
                atLeastOneLog = atLeastOneLog || (loggerModeEnabled && thisChanged);
                if (thisChanged)
                {
                    break;
                }
            }
        }
    }
    if (atLeastOneLog) {
        printf("\n");
    }

    return changed;
}

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
    if (isBackgroundMusicPlaying()) {
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
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, CameraSensitivity, true);
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
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    std::filesystem::path fullPath = texturesFolder / filename;

    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    return fullPath.string();
}
std::map<std::string, TextureEntry>& Plugin::getTexturesIndex() {
    if (!texturesIndex.empty()) {
        return texturesIndex;
    }

    std::map<std::string, TextureEntry> _texturesIndex;
    std::string indexFilePath = textureIndexFilePath();
    if (indexFilePath.empty()) {
        return texturesIndex;
    }

    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    Platform::FileHandle* f = Platform::OpenLocalFile(indexFilePath.c_str(), Platform::FileMode::ReadText);
    if (f) {
        char linebuf[1024];
        char entryname[32];
        char entryval[1024];
        while (!Platform::IsEndOfFile(f))
        {
            if (!Platform::FileReadLine(linebuf, 1024, f))
                break;

            int ret = sscanf(linebuf, "%31[A-Za-z_0-9\\-.]=%[^\t\r\n]", entryname, entryval);
            entryname[31] = '\0';
            if (ret < 2) continue;

            std::string entrynameStr = trim(std::string(entryname));
            std::string entryvalStr = trim(std::string(entryval));
            if (!entrynameStr.empty() && entrynameStr.compare(0, 1, ";") != 0 && entrynameStr.compare(0, 1, "[") != 0) {
                std::string uniqueId = (entrynameStr.find('.') == std::string::npos) ? entrynameStr : entrynameStr.substr(0, entrynameStr.find('.'));
                if (!_texturesIndex.count(uniqueId)) {
                    _texturesIndex[uniqueId] = TextureEntry();
                }
                auto& texture = _texturesIndex[uniqueId];

                if (entrynameStr == uniqueId) {
                    texture.setPath(entryvalStr);

                    std::filesystem::path fullPath = texturesFolder / entryvalStr;
                    if (!std::filesystem::exists(fullPath)) {
                        errorLog("Texture %s was supposed to be replaced by %s, but it doesn't exist", uniqueId.c_str(), fullPath.string().c_str());
                    }
                    else {
                        texture.setFullPath(fullPath.string());
                    }
                }
                if (entrynameStr == (uniqueId + ".type")) {
                    texture.setType(entryvalStr);
                }

                auto frameEntryPrefix = uniqueId + ".frames.";
                if (entrynameStr.size() >= frameEntryPrefix.size() && entrynameStr.compare(0, frameEntryPrefix.size(), frameEntryPrefix) == 0)
                {
                    size_t firstDot = entrynameStr.find('.');
                    size_t secondDot = entrynameStr.find('.', firstDot + 1);
                    size_t thirdDot = entrynameStr.find('.', secondDot + 1);
                    if (firstDot != std::string::npos && secondDot != std::string::npos && thirdDot != std::string::npos) {
                        int frameIndexStr = std::stoi(entrynameStr.substr(secondDot + 1, thirdDot - secondDot - 1));

                        auto frameEntryTimeSuffix = uniqueId + ".time";
                        if (entrynameStr.size() >= frameEntryTimeSuffix.size() && entrynameStr.compare(entrynameStr.size() - frameEntryTimeSuffix.size(), frameEntryTimeSuffix.size(), frameEntryTimeSuffix)) {
                            texture.setFrameTime(frameIndexStr, std::stoi(entryvalStr));
                        }
                    }
                    else {
                        int frameIndexStr = std::stoi(entrynameStr.substr(entrynameStr.rfind('.') + 1));
                        texture.setFramePath(frameIndexStr, entryvalStr);

                        std::filesystem::path fullPath = texturesFolder / entryvalStr;
                        if (!std::filesystem::exists(fullPath)) {
                            errorLog("Texture %s was supposed to be replaced by %s, but it doesn't exist", uniqueId.c_str(), fullPath.string().c_str());
                        }
                        else {
                            texture.setFrameFullPath(frameIndexStr, fullPath.string());
                        }
                    }
                }
            }
        }
    }

    texturesIndex = _texturesIndex;
    return texturesIndex;
}
TextureEntry& Plugin::textureById(std::string texture) {
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    if (std::filesystem::exists(_assetsFolderPath.parent_path()) && !std::filesystem::exists(_assetsFolderPath)) {
        std::filesystem::create_directory(_assetsFolderPath);
    }

    std::map<std::string, TextureEntry>& texturesIndex = getTexturesIndex();
    if (texturesIndex.count(texture)) {
        return texturesIndex[texture];
    }

    texturesIndex[texture] = TextureEntry();
    texturesIndex[texture].setPath(texture + ".png");

    std::filesystem::path fullPath = texturesFolder / (texture + ".png");
    if (std::filesystem::exists(fullPath)) {
        texturesIndex[texture].getLastScene().fullPath = fullPath.string();
    }
    return texturesIndex[texture];
}
std::string Plugin::tmpTextureFilePath(std::string texture) {
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path tmpFolderPath = _assetsFolderPath / "textures_tmp";

    if (shouldExportTextures() && std::filesystem::exists(tmpFolderPath.parent_path()) && !std::filesystem::exists(tmpFolderPath)) {
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

std::vector<std::string> Plugin::audioPackNames() {
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path fullPath = _assetsFolderPath / "audio";
    if (!std::filesystem::exists(fullPath)) {
        return {};
    }

    return Platform::ContentsOfFolder(fullPath.string(), true, false);
}

std::string Plugin::getReplacementBackgroundMusicFilePath(u16 id) {
    std::string filekey = "bgm" + std::to_string(id);

    auto getFilepathIfExists = [&](auto& filename) -> std::string {
        std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
        if (SelectedAudioPack != "") {
            std::filesystem::path fullPath0 = _assetsFolderPath / "audio" / SelectedAudioPack / filename;
            if (std::filesystem::exists(fullPath0)) {
                return fullPath0.string();
            }
        }
        std::filesystem::path fullPath = _assetsFolderPath / "audio" / filename;
        if (std::filesystem::exists(fullPath)) {
            return fullPath.string();
        }
        return "";
    };

    auto redirector = _BgmRedirectors.find(filekey);
    if (redirector != _BgmRedirectors.end()) {
        return getFilepathIfExists(redirector->second);
    } else {
        static std::vector<std::string> handledFormats = { "wav", "flac"};
        for(auto& format : handledFormats) {
            std::string filename = filekey + "." + format;
            auto foundFile = getFilepathIfExists(filename);
            if (foundFile != "") {
                return foundFile;
            }
        }
    }

    return "";
}


void Plugin::loadBgmRedirections() {
    auto _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path iniFilePath = _assetsFolderPath / "audio" / "bgm.ini";
    if (SelectedAudioPack != "") {
        std::filesystem::path fullPath0 = _assetsFolderPath / "audio" / SelectedAudioPack / "bgm.ini";
        if (std::filesystem::exists(fullPath0)) {
            iniFilePath = fullPath0;
        }
    }
    Platform::FileHandle* file = Platform::OpenLocalFile(iniFilePath.string().c_str(), Platform::FileMode::ReadText);
    if (file) {
        _BgmRedirectors.clear();

        char linebuf[1024];
        char entryname[1024];
        char entryval[1024];

        auto trim_str = [](const char* str) -> std::string {
            if (!str)
                return "";

            const char* start = str;
            while (*start && (*start == ' ' || *start == '\t')) {
                start++;
            }

            if (!*start)
                return "";

            const char* end = str + strlen(str) - 1;
            while (end > start && (*end == ' ' || *end == '\t')) {
                end--;
            }

            return std::string(start, end - start + 1);
        };

        while (!Platform::IsEndOfFile(file))
        {
            if (!Platform::FileReadLine(linebuf, 1024, file))
                break;

            size_t len = strlen(linebuf);
            if (len > 0 && (linebuf[len-1] == '\n' || linebuf[len-1] == '\r')) {
                linebuf[len-1] = '\0';

                if (len > 1 && linebuf[len-2] == '\r') {
                    linebuf[len-2] = '\0';
                }
            }

            if (strlen(linebuf) == 0
                || linebuf[0] == '#'
                || linebuf[0] == ';') {
                continue;
            }

            if (sscanf(linebuf, "%[^=]=%[^\n]", entryname, entryval) == 2) {
                _BgmRedirectors[trim_str(entryname)] = trim_str(entryval);
            }
        }

        CloseFile(file);
    }
}

void Plugin::startBackgroundMusic(u16 bgmId, u8 bgmState) {
    if (bgmId != _CurrentBackgroundMusic) {
        // Previous bgm should have already been stopped, but just in case:
        stopBackgroundMusic(1000);

        std::string replacementBgmPath = getReplacementBackgroundMusicFilePath(bgmId);
        if (replacementBgmPath != "") {
            _PendingReplacmentBgmMusicStart = true;
            _CurrentBackgroundMusicFilepath = replacementBgmPath;
            _CurrentBackgroundMusic = bgmId;
            u16 bgmResumeId = getMidiBgmToResumeId();
            _ResumeBackgroundMusicPosition = (bgmResumeId == _CurrentBackgroundMusic && bgmResumeId != BGM_INVALID_ID);
            _CurrentBgmIsStream = false;
            auto muter = AudioUtils::SSEQMuter(nds, bgmId, getMidiSequenceAddress(bgmId), getMidiSequenceSize(bgmId));
            if (bgmState == EMidiState::PrePlay) {
                // "Safe" muting: the sequence was loaded but is not currently being read: it's safe to erase all the bytes
                muter.muteSongSequence();
            } else if (bgmState == EMidiState::Playing) {
                // "Tricky" muting: the sequence is already being read, it's dangerous to randomly erase stuff
                // Instead, we parse the SSEQ carefully and only adjust note velocities and send volume=0 events
                // Note: it's not possible to mute a long note already playing, we need to wait until that track resumes
                muter.muteSongSequenceV2();
            }
        } else {
            _CurrentBackgroundMusic = BGM_INVALID_ID;
        }
    }
}

void Plugin::stopBackgroundMusic(u16 fadeOutDuration) {
    if (_CurrentBackgroundMusic != BGM_INVALID_ID) {
        u16 resumeSlot = getMidiBgmToResumeId();
        _StoreBackgroundMusicPosition = (resumeSlot == _CurrentBackgroundMusic && resumeSlot != BGM_INVALID_ID);
        _ShouldStopReplacementBgmMusic = true;
        _BackgroundMusicToStop = _CurrentBackgroundMusic;
        _BgmFadeOutDurationMs = fadeOutDuration;
        _CurrentBackgroundMusic = BGM_INVALID_ID;
    }
}

std::string getMidiStateName(EMidiState state) {
    switch(state) {
    case EMidiState::Stopped: return "Stopped";
    case EMidiState::LoadSequence: return "LoadSequence";
    case EMidiState::PrePlay: return "PrePlay";
    case EMidiState::Playing: return "Playing";
    case EMidiState::Stopping: return "Stopping";
    default: return "Invalid";
    }
}

void Plugin::refreshBackgroundMusic() {
#if !REPLACEMENT_BGM_ENABLED
    return;
#endif

    if (!isBackgroundMusicReplacementImplemented()) {
        return;
    }

    u8 state = getMidiBgmState();
    u16 bgmId = getMidiBgmId();

    if (state != _SoundtrackState) {
        switch(state) {
        case EMidiState::Stopped: {
            if (!_CurrentBgmIsStream && _CurrentBackgroundMusic != BGM_INVALID_ID) {
                stopBackgroundMusic(0);
            }
            break;
        }
        case EMidiState::LoadSequence: {
            // Do nothing (used by the NDS to load the SSEQ MIDI into RAM)
            break;
        }
        case EMidiState::PrePlay: {
            // Loaded has finished but bgm is not marked as "Playing" yet
            startBackgroundMusic(bgmId, state);
            break;
        }
        case EMidiState::Playing: {
            // SSEQ is loaded and ready to play
            startBackgroundMusic(bgmId, state); // Just in case (useful when loading a state)
            if (_PendingReplacmentBgmMusicStart) {
                _ShouldStartReplacementBgmMusic = true;
                _BackgroundMusicDelayAtStart = delayBeforeStartReplacementBackgroundMusic(bgmId);
                _PendingReplacmentBgmMusicStart = false;
            }
            break;
        }
        case EMidiState::Stopping: {
            // Note: bgmId is already 0xFFFF at this point
            u32 fadeOutProgress = getBgmFadeOutDuration();
            stopBackgroundMusic(fadeOutProgress);
            break;
        }
        default: {
            break;
        }
        }
        _SoundtrackState = state;
    }

    u8 currVolume = getMidiBgmVolume();
    if (_BackgroundMusicVolume != currVolume) {
        _BackgroundMusicVolume = currVolume;
        _ShouldUpdateReplacementBgmMusicVolume = true;
    }
}

enum EBgmStreamState : u8 {
    StrmStopped = 0x00,
    StrmPlaying = 0x03,
    StrmStopping = 0x0B,
    StrmEnded = 0x08,
};

void Plugin::stopReplacementStreamBgm(u32 fadeOutDuration) {
    if (_CurrentBgmIsStream && _CurrentBackgroundMusic != BGM_INVALID_ID) {
        _ShouldStopReplacementBgmMusic = true;
        _BackgroundMusicToStop = _CurrentBackgroundMusic;
        _BgmFadeOutDurationMs = fadeOutDuration;
        _CurrentBackgroundMusic = BGM_INVALID_ID;
        _CurrentBgmIsStream = false;
        _BgmStreamMuted = false;
    }
}

void Plugin::refreshStreamedMusic() {
    const u32 strmHeaderAddress = getStreamBgmAddress();
    if (strmHeaderAddress == 0) {
        return;
    }

    u32 strmTag = nds->ARM9Read32(strmHeaderAddress);
    if (strmTag != 0x4D525453) { // STRM
        stopReplacementStreamBgm(800);
        _BgmStreamState = 0;
        return;
    }

    u8 streamDsId = nds->ARM9Read8(strmHeaderAddress + 0xAC);
    u8 streamState = nds->ARM9Read8(strmHeaderAddress + 0x68);
    if (streamState != _BgmStreamState) {
        switch(streamState) {
        case EBgmStreamState::StrmStopped: {
            stopReplacementStreamBgm(0);
            break;
        }
        case EBgmStreamState::StrmPlaying: {
            u32 numSamples = nds->ARM9Read32(strmHeaderAddress + 0x24);
            u16 streamBgmId = getStreamBgmCustomIdFromDsId(streamDsId, numSamples);
            if (streamBgmId != BGM_INVALID_ID) {
                std::string replacementStrmPath = getReplacementBackgroundMusicFilePath(streamBgmId);
                if (replacementStrmPath != "") {
                    bool bShouldPlay = (!_CurrentBgmIsStream
                        || _CurrentBackgroundMusic == BGM_INVALID_ID
                        || _CurrentBackgroundMusic != streamBgmId);
                    if (bShouldPlay) {
                        stopReplacementStreamBgm(0);
                        _ShouldStartReplacementBgmMusic = true;
                        _CurrentBackgroundMusicFilepath = replacementStrmPath;
                        _CurrentBackgroundMusic = streamBgmId;
                        _BackgroundMusicDelayAtStart = delayBeforeStartReplacementBackgroundMusic(streamBgmId);
                        _CurrentBgmIsStream = true;
                        onStreamBgmReplacementStarted();
                    }
                    _BgmStreamMuted = true;
                } else {
                    _CurrentBackgroundMusic = BGM_INVALID_ID;
                    _BgmStreamMuted = false;
                }
            } else {
                _BgmStreamMuted = false;
            }
            break;
        }
        case EBgmStreamState::StrmStopping:
        case EBgmStreamState::StrmEnded: {
            stopReplacementStreamBgm(800);
            break;
        }
        default: {
            _BgmStreamMuted = false;
            break;
        }
        }
        _BgmStreamState = streamState;
    }

    if (_BgmStreamMuted) {
        muteStreamedMusic();
    }
}

void Plugin::muteStreamedMusic() {
    const u32 strmHeaderAddress = getStreamBgmAddress();
    u32 volumeControlAddress = strmHeaderAddress + 0xB0;
    u8 currentVolume = nds->ARM9Read8(volumeControlAddress);
    if (currentVolume > 0) {
        nds->ARM7Write8(volumeControlAddress, 0);
    }
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
    refreshStreamedMusic();

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
    CameraSensitivity = getIntConfig(root + ".CameraSensitivity");
    CameraSensitivity = (CameraSensitivity == 0) ? DefaultCameraSensitivity : CameraSensitivity;
    EnhancedGraphics = !getBoolConfig(root + ".DisableEnhancedGraphics");
    SingleScreenMode = !getBoolConfig(root + ".DisableSingleScreenMode");
    DisableReplacementTextures = false;
    FastForwardLoadingScreens = getBoolConfig(root + ".FastForwardLoadingScreens");
    DaysDisableHisMemories = getBoolConfig(root + ".DaysDisableHisMemories");
    ExportTextures = getBoolConfig(root + ".ExportTextures");
    FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    UIScale = getIntConfig(root + ".HUDScale");
    UIScale = (UIScale == 0) ? 4 : UIScale;
    SelectedAudioPack = getStringConfig(root + ".AudioPack");
}
void Plugin::loadConfigs(
    std::function<bool(std::string)> getBoolConfig,
    std::function<int(std::string)> getIntConfig,
    std::function<std::string(std::string)> getStringConfig
)
{
    _superLoadConfigs(getBoolConfig, getIntConfig, getStringConfig);
}

void Plugin::buildShapes()
{
    renderer_beforeBuildingShapes();
    GameSceneState = renderer_gameSceneState();
    current2DShapes = renderer_2DShapes();
    current3DShapes = renderer_3DShapes();
    renderer_afterBuildingShapes();
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

    if constexpr (ERROR_LOG_FILE_ENABLED) {
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
        if constexpr (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
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
            if (total < RAM_SEARCH_MAX_RESULTS*(4/byteSize)) {
                for (u32 index = limitMin; index < limitMax; index+=byteSize) {
                    u32 addr = (0x02000000 | index);
                    if (MainRAMState[index]) {
                        printf("0x%08x: 0x%08x\n", addr, LastMainRAM[index]);
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
        if constexpr (!RAM_SEARCH_EVERY_SINGLE_FRAME) {
            printf("Addresses matching the search: %d\n", total);
        }
    }
}

}