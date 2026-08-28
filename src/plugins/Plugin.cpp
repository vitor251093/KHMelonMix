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
}

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
    float hudScale = (((float)UIScale) - 4) / 2 + 4;

    glUniform1f(CompGpuLoc[CompShader][0], aspectRatio);
    glUniform1f(CompGpuLoc[CompShader][1], forcedAspectRatio);
    glUniform1f(CompGpuLoc[CompShader][2], hudScale);
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

bool Plugin::gpuOpenGL_applyChangesToPolygonVertex(int resolutionScale, s32 scaledPositions[10][2], melonDS::Polygon* polygon, float xCenter, float yCenter, ShapeData3D shape, int vertexIndex)
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

    vec3 newValues = shape.compute3DCoordinatesOf3DSquareShapeInVertexMode(_x, _y, _z, xCenter, yCenter, polygon->Attr, polygon->TexParam, rgb, resolutionScale, aspectRatio);
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
                                printf("Position: %d - %d -- Size: %d - %d - New vertexes: %d\n", x0, y0, x1 - x0, y1 - y0, polygon->NumVertices);
                            }

                            float xCenter = (x0 + x1)/2.0;
                            float yCenter = (y0 + y1)/2.0;

                            if ((shape.effects & 0x8) != 0)
                            {
                                for (int vIndex = 0; vIndex < polygon->NumVertices; vIndex++) {
                                    scaledPositions[vIndex][0] = (u32)(xCenter + (s32)(((float)scaledPositions[vIndex][0] - xCenter)/aspectRatio));
                                }
                            }
                            else
                            {
                                for (int vIndex = 0; vIndex < polygon->NumVertices; vIndex++) {
                                    gpuOpenGL_applyChangesToPolygonVertex(resolutionScale, scaledPositions, polygon, xCenter, yCenter, shape, vIndex);
                                }
                            }

                            if (atLeastOneLog) {
                                printf("\n");
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
                bool thisChanged = gpuOpenGL_applyChangesToPolygonVertex(resolutionScale, scaledPositions, polygon, 0, 0, shape, vertexIndex);
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
    if (_IsReplacementCutsceneRunning) {
        if (_ShowingCutsceneSkipMenu) {
            // the Skip/Continue menu owns pause state; ignore the pause hotkey
            return true;
        }
        if (_ReplacementCutsceneIsPaused) {
            hideCutscenePauseMenuOverlay();
        }
        else {
            showCutscenePauseMenuOverlay(0);
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

    // New behavior: pressing Start during an HD cutscene opens a Skip/Continue menu
    // (pausing the video) instead of skipping immediately. Gated by a setting so the
    // classic instant-skip behavior can be restored.
    if (_IsReplacementCutsceneRunning) {
        const u32 BTN_A     = (1 << 0);
        const u32 BTN_START = (1 << 3);
        const u32 BTN_UP    = (1 << 6);
        const u32 BTN_DOWN  = (1 << 7);

        u32 pressed = (~(*InputMask)) & 0xFFF;
        u32 justPressed = pressed & ~_LastCutsceneMenuButtons; // rising edge only
        _LastCutsceneMenuButtons = pressed;

        if (!_ShowingCutsceneSkipMenu) {
            if (!_ReplacementCutsceneIsPaused && (justPressed & BTN_START)) {
                pauseReplacementCutsceneThroughPauseMenu();
                _CutsceneMenuSoundRequest = 1; // enter
            }
        }
        else {
            if (justPressed & (BTN_UP | BTN_DOWN)) {
                _CutsceneSkipMenuSelection = _CutsceneSkipMenuSelection == 0 ? 1 : 0;
                updateCutscenePauseMenuOverlaySelection(_CutsceneSkipMenuSelection);
                _CutsceneMenuSoundRequest = 2; // move
            }
            else if (justPressed & BTN_START) {
                // Start always means Continue (matches the in-game menu)
                _CutsceneMenuSoundRequest = 3; // continue
                resumeReplacementCutsceneThroughPauseMenu();
            }
            else if (justPressed & BTN_A) {
                _CutsceneMenuSoundRequest = 4; // select
                if (_CutsceneSkipMenuSelection == 1) {
                    skipIngamePrerenderedCutsceneThroughPauseMenu();
                }
                else {
                    resumeReplacementCutsceneThroughPauseMenu();
                }
            }
            // B (and everything else) does nothing while the menu is open
        }

        // While an HD cutscene is playing with the menu enabled, these buttons are
        // reserved for the menu (open / navigate / confirm / continue) and must
        // never reach the background DS - not only on the open/close frames but for
        // as long as they stay held afterwards. Otherwise a Start still held for a
        // few frames after continuing leaks through and pauses/disrupts the DS
        // cutscene. The skip sequence below re-asserts Start when it must feed it.
        *InputMask |= BTN_A | BTN_START | BTN_UP | BTN_DOWN;
    }

    if (isPauseMenuGameScene()) {
        const u32 BTN_A    = (1 << 0);
        const u32 BTN_B    = (1 << 1);
        const u32 BTN_UP   = (1 << 6);
        const u32 BTN_DOWN = (1 << 7);

        u32 pressed = (~(*InputMask)) & 0xFFF;
        u32 justPressed = pressed & ~_LastGamePauseMenuButtons;
        _LastGamePauseMenuButtons = pressed;

        if (justPressed & (BTN_UP | BTN_DOWN)) {
            _GamePauseMenuSelection = _GamePauseMenuSelection == 0 ? 1 : 0;
            if (updateGamePauseMenuOverlaySelection) updateGamePauseMenuOverlaySelection(_GamePauseMenuSelection);
        }
        if (justPressed & BTN_A) {
            onGamePauseMenuConfirmPressed();
        }
        if (justPressed & BTN_B) {
            onGamePauseMenuCancelPressed();
        }
    }

    if (_SkipDsCutscene) { // Start (skip HD cutscene)
        if (!_IsMobiCutsceneRunning && !_IsInEngineCutsceneRunning) { // can only skip after DS cutscene was skipped
            _SkipDsCutscene = false;
            if (_IsReplacementCutsceneRunning) {
                stopReplacementCutsceneAndResumeGameAfterSkippingIngamePrerenderedCutscene();
            }
            *InputMask |= (1<<3);
        }
        else {
            if (_StartPressCount == 0) {
                bool requiresDoubleStart = (_CutscenesQueue[0]->dsScreensState & 4) == 4;
                if (requiresDoubleStart) {
                    _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT*2 + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT;
                }
                else {
                    _StartPressCount = CUTSCENE_SKIP_START_FRAMES_COUNT;
                }
            }

            if (_IsInEngineCutsceneRunning) {
                // TODO: KH Press Start, press down, press A (only for Days)
            }
        }
    }

    if (_IsReplacementCutsceneRunning) {
        if (_StartPressCount > 0) {
            _StartPressCount--;

            bool requiresDoubleStart = (_CutscenesQueue[0]->dsScreensState & 4) == 4;
            if (requiresDoubleStart) {
                if (_StartPressCount < CUTSCENE_SKIP_START_FRAMES_COUNT || _StartPressCount > CUTSCENE_SKIP_START_FRAMES_COUNT + CUTSCENE_SKIP_INTERVAL_FRAMES_COUNT) {
                    *InputMask &= ~(1<<3); // Start (skip DS cutscene)
                }
            }
            else {
                *InputMask &= ~(1<<3); // Start (skip DS cutscene)
            }
        }

        if (isInEngineCutsceneGameScene() && _APressCount == 0)
        {
            _APressCount = DIALOG_SKIP_START_FRAMES_COUNT*2 + DIALOG_SKIP_INTERVAL_FRAMES_COUNT;
        }
        if (_APressCount > 0) {
            _APressCount--;

            bool requiresSmashingA = (_CutscenesQueue[0]->dsScreensState & 8) == 8;
            if (requiresSmashingA) {
                if (_APressCount < DIALOG_SKIP_START_FRAMES_COUNT || _APressCount > DIALOG_SKIP_START_FRAMES_COUNT + DIALOG_SKIP_INTERVAL_FRAMES_COUNT) {
                    *InputMask &= ~(1<<0); // A (skip DS cutscene)
                }
            }
            else {
                *InputMask &= ~(1<<0); // A (skip DS cutscene)
            }
        }
    }

    return true;
}

bool Plugin::_superApplyAddonKeysToCutsceneMenu(u32* AddonMask, u32* AddonPress, int upBit, int downBit)
{
    if (!_ShowingCutsceneSkipMenu) {
        return false;
    }

    u32 navPress = (*AddonPress) & (((u32)1 << upBit) | ((u32)1 << downBit));
    if (navPress != 0) {
        _CutsceneSkipMenuSelection = _CutsceneSkipMenuSelection == 0 ? 1 : 0;
        updateCutscenePauseMenuOverlaySelection(_CutsceneSkipMenuSelection == 0 ? 1 : 0);
        _CutsceneMenuSoundRequest = 2; // move
    }

    // While the menu is open it owns input; don't let command-menu keys reach the game.
    *AddonMask = 0;
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
        }
        return;
    }

    bool resetTouchScreen = false;
    if (*isTouching == false) {
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
const char* Plugin::skipUtf8Bom(const char* line) {
    if ((unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF)
    {
        return line + 3;
    }

    return line;
}
std::string Plugin::textureIndexFilePath() {
    std::string filename = "index.ini";
    std::filesystem::path _assetsFolderPath = gameAssetsFolderPath();
    std::filesystem::path texturesFolder = _assetsFolderPath / "textures";
    std::filesystem::path fullPath = texturesFolder / filename;

    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    // u8string(), not string(): this is handed to Platform::OpenLocalFile, which decodes it
    // as UTF-8. On Windows string() would yield the ANSI code page instead, and any accent
    // in the path would come out mangled.
    return fullPath.u8string();
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
        bool firstLine = true;
        while (!Platform::IsEndOfFile(f))
        {
            if (!Platform::FileReadLine(linebuf, 1024, f))
                break;

            const char* line = linebuf;
            if (firstLine)
            {
                firstLine = false;
                line = skipUtf8Bom(line);
            }

            // '-' must stay last in the scanset. Anywhere else the CRT reads it as a range
            // delimiter: glibc ignores the reversed "\-." range, but the UCRT swaps it into
            // '.'..'\\', which matches '=' and makes every line fail to parse.
            int ret = sscanf(line, "%31[A-Za-z0-9_.\\-]=%[^\t\r\n]", entryname, entryval);
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

        Platform::CloseFile(f);

        if (_texturesIndex.empty()) {
            errorLog("Texture index %s was read, but none of its lines could be parsed", indexFilePath.c_str());
        }
    }
    else {
        // textureIndexFilePath() only hands back a path it has just found on disk, so a
        // failure here means index.ini is present but unreadable, not that there is none.
        errorLog("Texture index %s exists but could not be opened", indexFilePath.c_str());
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

bool Plugin::IsReplacementCutsceneRunning() {return _IsReplacementCutsceneRunning;}

CutsceneEntry* Plugin::CurrentCutscene() {return _CutscenesQueue[0];};

CutsceneEntry* Plugin::detectTopScreenCutscene()
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

    CutsceneEntry* cutscene1 = getMobiCutsceneByAddress(cutsceneAddressValue);
    if (cutscene1 != nullptr)
    {
        return cutscene1;
    }

    cutsceneAddressValue = detectTopScreenInEngineCutsceneId();
    return getInEngineCutsceneById(cutsceneAddressValue);
}

CutsceneEntry* Plugin::detectBottomScreenCutscene()
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
std::string Plugin::pauseMenuTitle()
{
    return Plugins::menuStrings(cutsceneMenuLanguage()).title;
}

std::vector<std::string> Plugin::cutsceneMenuButtonLabels()
{
    const Plugins::MenuStrings& strings = Plugins::menuStrings(cutsceneMenuLanguage());
    return { strings.cont, strings.skip };
}

CutsceneEntry* Plugin::detectCutscene()
{
    CutsceneEntry* cutscene1 = detectTopScreenCutscene();
    CutsceneEntry* cutscene2 = detectBottomScreenCutscene();

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

    if (startReplacementCutscene == nullptr)
    {
        return;
    }

    CutsceneEntry* cutscene = detectCutscene();

    if (_DidReplacementCutsceneFailedToPlay || (!_IsReplacementCutsceneRunning && !_IsMobiCutsceneRunning &&
            !_IsInEngineCutsceneRunning && _IsIngameOrReplacementCutsceneRunning && canReturnToGameAfterReplacementCutscene())) {
        _APressCount = 0;
        _StartPressCount = 0;

        if (_DidReplacementCutsceneFailedToPlay)
        {
            _IsReplacementCutsceneRunning = false;
            _IsMobiCutsceneRunning = false;
            _IsInEngineCutsceneRunning = false;
        }

        _IsIngameOrReplacementCutsceneRunning = false;
        _LastCutscene = _CutscenesQueue[0];
        _CutscenesQueue.erase(_CutscenesQueue.begin());
        if (!_DidReplacementCutsceneFailedToPlay && !_CutscenesQueue.empty())
        {
            printf("Playing next cutscene on queue: %s\n", _CutscenesQueue[0]->Name);
            bool isMobiCutsceneRunning = (_CutscenesQueue[0]->dsScreensState & 1) == 1;
            _PlayFrameLimitCount = isMobiCutsceneRunning ? 10 : 60;
            _IsMobiCutsceneRunning = isMobiCutsceneRunning;
            _IsInEngineCutsceneRunning = !isMobiCutsceneRunning;
            _IsReplacementCutsceneRunning = true;
            _IsIngameOrReplacementCutsceneRunning = true;
            _IsUnskippableCutscene = isUnskippableMobiCutscene(_CutscenesQueue[0]);
            std::string videoPath = replacementCutsceneFilePath(_CutscenesQueue[0]);
            std::string subtitlesPath = replacementCutsceneSubtitlesFilePath(_CutscenesQueue[0]);
            startReplacementCutscene(videoPath, subtitlesPath);
        }
        else
        {
            printf("Resuming game\n");
            resumeEmulatorAfterBothIngamePrerenderedCutsceneAndReplacementCutsceneEnded();
        }

        _ReplayFrameLimitCount = 60;

        if (!_DidReplacementCutsceneFailedToPlay && _CutscenesQueue.empty()) {
            u32 cutsceneAddress = detectTopScreenMobiCutsceneAddress();
            if (cutsceneAddress != 0) {
                nds->ARM7Write32(cutsceneAddress, 0x0);
            }

            u32 cutsceneAddress2 = detectBottomScreenMobiCutsceneAddress();
            if (cutsceneAddress2 != 0) {
                nds->ARM7Write32(cutsceneAddress2, 0x0);
            }
        }

        _DidReplacementCutsceneFailedToPlay = false;
    }

    if (cutscene != nullptr) {
        for (auto blacklistedCutscene : _CutscenesBlacklist) {
            if (cutscene->usAddress == blacklistedCutscene->usAddress) {
                cutscene = nullptr;
                break;
            }
        }
    }

    if (_ReplayFrameLimitCount > 0) {
        _ReplayFrameLimitCount--;
        if (cutscene != nullptr && cutscene->usAddress == _LastCutscene->usAddress) {
            cutscene = nullptr;
        }
    }

    if (_PlayFrameLimitCount > 0) {
        _PlayFrameLimitCount--;
    }

    if (_PlayFrameLimitCount == 0 && _IsMobiCutsceneRunning) {
        if (didMobiCutsceneEnded()) {
            printf("Ingame Mobi cutscene terminated\n");

            _IsMobiCutsceneRunning = false;
            pauseEmulatorAfterIngamePrerenderedCutsceneEndedBeforeReplacementCutscene();
        }
    }

    if (_PlayFrameLimitCount == 0 && _IsInEngineCutsceneRunning) {
        if (didInEngineCutsceneEnded()) {
            printf("Ingame in engine cutscene terminated\n");

            _IsInEngineCutsceneRunning = false;
            pauseEmulatorAfterIngamePrerenderedCutsceneEndedBeforeReplacementCutscene();
        }
    }

    if (cutscene != nullptr) {
        std::string path = replacementCutsceneFilePath(cutscene);

        int targetAddress = cutscene->usAddress;
        bool isInQueue = std::find_if(_CutscenesQueue.begin(), _CutscenesQueue.end(),
            [targetAddress](const CutsceneEntry* entry) {
                return entry != nullptr && entry->usAddress == targetAddress;
            }) != _CutscenesQueue.end();
        if (!isInQueue && !path.empty()) {
            if (!_CutscenesQueue.empty()) {
                printf("Adding cutscene to queue: %s\n", cutscene->Name);
                _CutscenesQueue.push_back(cutscene);
            }
            else {
                printf("Preparing to load cutscene: %s\n", cutscene->Name);
                bool isMobiCutsceneRunning = (cutscene->dsScreensState & 1) == 1;
                _PlayFrameLimitCount = isMobiCutsceneRunning ? 10 : 60;
                _IsMobiCutsceneRunning = isMobiCutsceneRunning;
                _IsInEngineCutsceneRunning = !isMobiCutsceneRunning;
                _IsReplacementCutsceneRunning = true;
                _IsIngameOrReplacementCutsceneRunning = true;
                _CutscenesQueue.push_back(cutscene);
                _IsUnskippableCutscene = isUnskippableMobiCutscene(_CutscenesQueue[0]);
                std::string videoPath = replacementCutsceneFilePath(_CutscenesQueue[0]);
                std::string subtitlesPath = replacementCutsceneSubtitlesFilePath(_CutscenesQueue[0]);
                startReplacementCutscene(videoPath, subtitlesPath);
            }
        }
    }
}

void Plugin::pauseReplacementCutsceneThroughPauseMenu()
{
    _ShowingCutsceneSkipMenu = true;
    _ReplacementCutsceneIsPaused = true;
    _CutsceneSkipMenuSelection = 0;

    showCutscenePauseMenuOverlay(0);
}

void Plugin::skipIngamePrerenderedCutsceneThroughPauseMenu()
{
    printf("Skip ingame cutscene through pause menu\n");

    _ShowingCutsceneSkipMenu = false;
    _ReplacementCutsceneIsPaused = false;
    _CutsceneSkipMenuSelection = 0;

    _SkipDsCutscene = true;
}

void Plugin::resumeReplacementCutsceneThroughPauseMenu()
{
    _ShowingCutsceneSkipMenu = false;
    _ReplacementCutsceneIsPaused = false;
    _CutsceneSkipMenuSelection = 0;

    hideCutscenePauseMenuOverlay();
}

void Plugin::stopReplacementCutsceneAndResumeGameAfterSkippingIngamePrerenderedCutscene() {
    printf("Stop replacement cutscene and resume game\n");

    _ShowingCutsceneSkipMenu = false;
    _CutsceneSkipMenuSelection = 0;
    _LastCutsceneMenuButtons = 0;

    _IsReplacementCutsceneRunning = false;
    stopReplacementCutsceneAndResumeEmulator();
}

void Plugin::skipIngamePrerenderedCutsceneAfterReplacementCutsceneFinishesNaturally()
{
    printf("Resume game after stopping replacement cutscene\n");

    if (_CutscenesQueue.size() <= 1)
    {
        if (_IsMobiCutsceneRunning || _IsInEngineCutsceneRunning)
        {
            _SkipDsCutscene = true;
        }
        else
        {
            _IsReplacementCutsceneRunning = false;
            resumeHiddenEmulatorAfterReplacementCutsceneStopped();
        }
    }
    else
    {
        _IsReplacementCutsceneRunning = false;
        resumeHiddenEmulatorAfterReplacementCutsceneStopped();
    }
}

void Plugin::resumeIngamePrerenderedCutsceneAfterReplacementCutsceneFailedToPlay(std::string error)
{
    printf("Resume game after replacement cutscene failed to play\n");

    if (postMessageToOsd != nullptr)
    {
        postMessageToOsd("Failed to play " + replacementCutsceneFilePath(_CutscenesQueue[0]));
        postMessageToOsd("Cause of failure: " + error);
    }

    _CutscenesBlacklist.push_back(_CutscenesQueue[0]);

    _DidReplacementCutsceneFailedToPlay = true;
    resumeHiddenEmulatorAfterReplacementCutsceneStopped();
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
    Platform::FileHandle* file = Platform::OpenLocalFile(iniFilePath.u8string().c_str(), Platform::FileMode::ReadText);
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

        bool firstLine = true;
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

            const char* line = linebuf;
            if (firstLine)
            {
                firstLine = false;
                line = skipUtf8Bom(line);
            }

            if (strlen(line) == 0
                || line[0] == '#'
                || line[0] == ';') {
                continue;
            }

            if (sscanf(line, "%[^=]=%[^\n]", entryname, entryval) == 2) {
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
    if (_IsIngameOrReplacementCutsceneRunning)
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

void Plugin::refreshGamePauseMenuOverlay()
{
    bool shouldShow = isPauseMenuGameScene();
    if (shouldShow == _ShowingGamePauseMenuOverlay)
    {
        return;
    }
    _ShowingGamePauseMenuOverlay = shouldShow;

    if (shouldShow)
    {
        if (showGamePauseMenuOverlay) showGamePauseMenuOverlay();
    }
    else
    {
        _GamePauseMenuSelection = 0;
        _LastGamePauseMenuButtons = 0;
        onGamePauseMenuOverlayHidden();
        if (hideGamePauseMenuOverlay) hideGamePauseMenuOverlay();
    }
}

bool Plugin::refreshGameScene()
{
    int newGameScene = detectGameScene();

    debugLogs(newGameScene);

    bool updated = setGameScene(newGameScene);

    refreshGamePauseMenuOverlay();

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
    AutomaticallyMapJoysticks = !getBoolConfig(root + ".DisableAutomaticJoystickMapping");
    JoystickDefaultsApplied = getBoolConfig(root + ".JoystickDefaultsApplied");
    DisableReplacementTextures = false;
    FastForwardLoadingScreens = getBoolConfig(root + ".FastForwardLoadingScreens");
    DaysDisableHisMemories = getBoolConfig(root + ".DaysDisableHisMemories");
    ExportTextures = getBoolConfig(root + ".ExportTextures");
    FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
    UIScale = getIntConfig(root + ".HUDScale");
    UIScale = (UIScale == 0) ? 4 : UIScale;
    SelectedAudioPack = getStringConfig(root + ".AudioPack");
    HDCutscenesEnabled = !getBoolConfig(root + ".DisableHDCutscenes");
    SubtitlesEnabled = !getBoolConfig(root + ".DisableSubtitles");
    JoystickConfirmIndex = getIntConfig("Instance0.JoystickConfirmIndex");
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
    current2DShapes = renderer_topScreen_2DShapes();
    current3DShapes = renderer_topScreen_3DShapes();

    std::vector<ShapeData2D> currentCompositionShapes = renderer_composition();
    current2DShapes.insert(current2DShapes.begin(), currentCompositionShapes.begin(), currentCompositionShapes.end());

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