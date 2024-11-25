#ifndef MELONDS_MELONDS_H
#define MELONDS_MELONDS_H

#include <list>
#include "AndroidFileHandler.h"
#include "AndroidCameraHandler.h"
#include "RewindManager.h"
#include "RomGbaSlotConfig.h"
#include "retroachievements/RAAchievement.h"
#include "retroachievements/RACallback.h"
#include "../types.h"
#include "../GPU.h"
#include <android/asset_manager.h>

namespace MelonDSAndroid {
    typedef struct {
        char username[11];
        int language;
        int birthdayMonth;
        int birthdayDay;
        int favouriteColour;
        char message[27];
        bool randomizeMacAddress;
        char macAddress[18];
    } FirmwareConfiguration;

    typedef struct {
        bool userInternalFirmwareAndBios;
        char* dsBios7Path;
        char* dsBios9Path;
        char* dsFirmwarePath;
        char* dsiBios7Path;
        char* dsiBios9Path;
        char* dsiFirmwarePath;
        char* dsiNandPath;
        char* internalFilesDir;
        float fastForwardSpeedMultiplier;
        bool showBootScreen;
        bool useJit;
        int consoleType;
        bool soundEnabled;
        int volume;
        int audioInterpolation;
        int audioBitrate;
        int audioLatency;
        int micSource;
        int rewindEnabled;
        int rewindCaptureSpacingSeconds;
        int rewindLengthSeconds;
        FirmwareConfiguration firmwareConfiguration;
        GPU::RenderSettings renderSettings;
        int renderer;
    } EmulatorConfiguration;

    typedef struct {
        u32 codeLength;
        u32 code[2*64];
    } Cheat;

    typedef struct {
        int currentFrame;
        std::list<RewindManager::RewindSaveState> rewindStates;
    } RewindWindow;

    typedef enum {
        ROM,
        FIRMWARE
    } RunMode;

    extern AAssetManager* assetManager;
    extern AndroidFileHandler* fileHandler;
    extern AndroidCameraHandler* cameraHandler;
    extern std::string internalFilesDir;

    extern void setConfiguration(EmulatorConfiguration emulatorConfiguration);
    extern void setup(AAssetManager* androidAssetManager, AndroidCameraHandler* androidCameraHandler, RetroAchievements::RACallback* raCallback, u32* textureBufferPointer, bool isMasterInstance);
    extern void setCodeList(std::list<Cheat> cheats);
    extern void setupAchievements(std::list<RetroAchievements::RAAchievement> achievements, std::string* richPresenceScript);
    extern void unloadAchievements(std::list<RetroAchievements::RAAchievement> achievements);
    extern std::string getRichPresenceStatus();
    extern void updateEmulatorConfiguration(EmulatorConfiguration emulatorConfiguration, u32* frameBuffer);

    /**
     * Loads the NDS ROM and, optionally, the GBA ROM.
     *
     * @param romPath The path to the NDS rom
     * @param sramPath The path to the rom's SRAM file
     * @param gbaSlotConfig The config to be used for the GBA slot
     * @return The load result. 0 if everything was loaded successfully, 1 if the NDS ROM was loaded but the GBA ROM
     * failed to load, 2 if the NDS ROM failed to load
     */
    extern int loadRom(char* romPath, char* sramPath, RomGbaSlotConfig* gbaSlotConfig);
    extern int bootFirmware();
    extern void start();
    extern u32 loop();
    extern void pause();
    extern void resume();
    extern bool reset();
    extern void updateMic();
    extern bool saveState(const char* path);
    extern bool loadState(const char* path);
    extern bool saveRewindState(RewindManager::RewindSaveState rewindSaveState);
    extern bool loadRewindState(RewindManager::RewindSaveState rewindSaveState);
    extern RewindWindow getRewindWindow();
    extern void cleanup();
}

#endif //MELONDS_MELONDS_H
