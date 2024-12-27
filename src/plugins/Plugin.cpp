#include "Plugin.h"

#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>

namespace Plugins
{

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

    if ((*HotkeyPress) & (1 << 18)) { // HUD Toggle (HK_HUDToggle)
        hudToggle();
    }

    return true;
}

void Plugin::_superApplyTouchKeyMask(u32 TouchKeyMask, u16 sensitivity, bool resetOnEdge, u16* touchX, u16* touchY, bool* isTouching)
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
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path texturesFolder = assetsFolderPath / "textures";
    std::filesystem::path fullPath = texturesFolder / filename;

    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    return fullPath.string();
}
std::map<std::string, TextureEntry> Plugin::getTexturesIndex() {
    if (!texturesIndex.empty()) {
        return texturesIndex;
    }

    std::map<std::string, TextureEntry> _texturesIndex;
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
                std::string s = entryvalStr;
                std::string delimiter = ";";
                std::string delimiter2 = ",";

                TextureEntry textureObj = {s, 0, 0, 0, 0};
                if (s.find(delimiter) != std::string::npos) {
                    std::vector<std::string> tokens;
                    size_t last = 0;
                    size_t next = 0;
                    while ((next = s.find(delimiter, last)) != std::string::npos) {
                        tokens.push_back(s.substr(last, next-last));
                        last = next + 1;
                    }
                    tokens.push_back(s.substr(last));

                    if (tokens.size() == 3) {
                        textureObj.filename = tokens[0];
                        textureObj.posY = (u16)std::stoi(tokens[1].substr(0, tokens[1].find(delimiter2)));
                        textureObj.posX = (u16)std::stoi(tokens[1].substr(tokens[1].find(delimiter2) + 1));
                        textureObj.sizeX = (u16)std::stoi(tokens[2].substr(0, tokens[2].find(delimiter2)));
                        textureObj.sizeY = (u16)std::stoi(tokens[2].substr(tokens[2].find(delimiter2) + 1));
                    }
                }

                _texturesIndex[entrynameStr] = textureObj;
            }
        }
    }
    texturesIndex = _texturesIndex;
    return texturesIndex;
}

TextureEntry* Plugin::textureFileConfig(std::string texture) {
    textureFilePath(texture);
    if (texturesIndex.count(texture)) {
        return &texturesIndex[texture];
    }
    return nullptr;
}
std::string Plugin::textureFilePath(std::string texture) {
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path texturesFolder = assetsFolderPath / "textures";
    if (!std::filesystem::exists(assetsFolderPath)) {
        std::filesystem::create_directory(assetsFolderPath);
    }

    std::map<std::string, TextureEntry> texturesIndex = getTexturesIndex();
    if (texturesIndex.count(texture)) {
        TextureEntry textureObj = texturesIndex[texture];
        std::filesystem::path fullPath = texturesFolder / textureObj.filename;
        if (!std::filesystem::exists(fullPath)) {
            errorLog("Texture %s was supposed to be replaced by %s, but it doesn't exist", texture.c_str(), fullPath.string().c_str());
            return "";
        }

        return fullPath.string();
    }

    std::string filename = texture + ".png";
    std::filesystem::path fullPath = texturesFolder / filename;
    if (!std::filesystem::exists(fullPath)) {
        return "";
    }

    texturesIndex[texture] = {filename, 0, 0, 0, 0};
    return fullPath.string();
}
std::string Plugin::tmpTextureFilePath(std::string texture, bool partial) {
    std::string assetsFolderName = assetsFolder();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolderName;
    std::filesystem::path tmpFolderPath = assetsFolderPath / "textures_tmp";

    if (shouldExportTextures() && !std::filesystem::exists(tmpFolderPath)) {
        std::filesystem::create_directory(tmpFolderPath);
    }

    std::filesystem::path fullPathTmp = tmpFolderPath / (texture + (partial ? ".png.part" : ".png"));
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

void Plugin::_superLoadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
{
    std::string root = tomlUniqueIdentifier();
    DisableEnhancedGraphics = getBoolConfig(root + ".DisableEnhancedGraphics");
    ExportTextures = getBoolConfig(root + ".ExportTextures");
    FullscreenOnStartup = getBoolConfig(root + ".FullscreenOnStartup");
}
void Plugin::loadConfigs(std::function<bool(std::string)> getBoolConfig, std::function<std::string(std::string)> getStringConfig)
{
    _superLoadConfigs(getBoolConfig, getStringConfig);
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