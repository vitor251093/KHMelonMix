/*
    Copyright 2016-2025 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include "SettingsLocale.h"

static const SettingsLocale kEnglish = {
    /* on */  "On",
    /* off */ "Off",
    /* none */ "None",
    /* titleGameSettings */ "GAME SETTINGS",

    /* sidebarLabels */ {
        "Game",
        "Emulation",
        "Display",
        "Sound",
        "Gamepad",
        "Keyboard",
        "System",
        "Before You Stream\xE2\x80\xA6",
        "Quit Game",
    },

    /* sectionOverview */ {
        "Settings specific to the loaded Kingdom Hearts game.",
        "Configure console type, frame rate, and runtime behavior.",
        "Renderer, resolution, synchronization, and visual theme.",
        "Volume, audio quality, and microphone input.",
        "Remap controller buttons and analog stick assignments.",
        "Remap keyboard bindings for DS buttons and hotkeys.",
        "Network mode, battery simulation, and firmware options.",
        "Streaming guidelines and copyright information.",
        "Exit the current game session.",
    },

    /* hintToggle */      "Toggle",
    /* hintSelect */      "Select",
    /* hintAdjust */      "Adjust",
    /* hintReset */       "Reset",
    /* hintBack */        "Back",
    /* hintEnter */       "Enter",
    /* hintClose */       "Close",
    /* hintConfirm */     "Confirm",
    /* hintRebind */      "Rebind",
    /* hintResetAll */    "Reset All",
    /* hintClear */       "Clear",
    /* remapSelectPrompt */  "Select an action to assign.",
    /* remapResetBackHint */ "X Reset All  |  B Back to Sidebar",
    /* hintOpenWebsite */ "Open Website",
    /* hintQuitGame */    "Quit Game",
    /* hintYes */         "Yes",
    /* hintNo */          "No",
    /* hintCancel */      "Cancel",

    /* captureController */     "Press a controller button\xE2\x80\xA6",
    /* captureKey */            "Press a key\xE2\x80\xA6",
    /* captureCancelKeyboard */ "Press Esc to cancel",
    /* captureReleaseHint */    "Release controller\xE2\x80\xA6",
    /* captureCancelCountdown */"Cancelling in %1s",

    /* resetSectionPrompt */ "Reset %1 to defaults?",
    /* resetAllBindings */   "Reset all bindings?",
    /* pillYes */ "Yes",
    /* pillNo */  "No",

    /* noKHGameNote */
        "No Kingdom Hearts game is running \xe2\x80\x94 "
        "KH-specific settings will appear here once a game is loaded.",

    /* gameLanguageLabel */       "Language",
    /* gameLanguageDesc */        "Firmware language used for cutscene subtitles and pause menus.",
    /* gameFastForwardLabel */    "Fast-forward Loading",
    /* gameFastForwardDesc */     "Speed through loading screens automatically.",
    /* gameSkipCutscenesLabel */  "Skip Cutscenes Instantly",
    /* gameSkipCutscenesDesc */   "Press a button during a cutscene to skip it immediately.",
    /* gameHisMemoriesLabel */    "Disable \"His\" Memories",
    /* gameHisMemoriesDesc */     "Remove the additional memory episodes added by this port.",

    /* displayEnhancedLabel */     "Enhanced Graphics",
    /* displayEnhancedDesc */      "Enable high-resolution backgrounds and models.",
    /* displaySingleScreenLabel */ "Single Screen Mode",
    /* displaySingleScreenDesc */  "Display only the top screen, scaled to fill the window.",
    /* displaySubtitlesLabel */    "Show Subtitles",
    /* displaySubtitlesDesc */     "Display subtitles during HD cutscene playback.",
    /* displayHUDScaleLabel */     "HUD Scale",
    /* displayHUDScaleDesc */      "Scale the heads-up display elements.",

    /* soundAudioPackLabel */ "Audio Pack",
    /* soundAudioPackDesc */  "Replacement music pack for in-game audio.",

    /* keyboardCamSensLabel */  "Camera Sensitivity",
    /* keyboardCamSensDesc */   "Speed of the touch-screen camera controls.",
    /* keyboardBindingsLabel */ "Key Bindings",
    /* keyboardBindingsDesc */  "Assign keyboard keys to DS buttons and hotkeys.",

    // Emulation
    /* emuConsoleType */     "Console Type",          "Emulated console. Changes take effect after restarting the game.",
    /* emuDirectBoot */      "Direct Boot",           "Skip the DS boot sequence and launch the game directly.",
    /* emuFpsLimit */        "FPS Limit",             "Cap emulation speed to the target frame rate.",
    /* emuTargetFps */       "Target FPS",            "Frame rate cap for normal play.",
    /* emuFastForwardFps */  "Fast-forward FPS",      "Speed cap while fast-forward is held.",
    /* emuSlowmoFps */       "Slowmo FPS",            "Frame rate cap while slow-motion is active.",
    /* emuMuteFastForward */ "Mute Fast-forward",     "Silence audio while fast-forward is held.",
    /* emuPauseLostFocus */  "Pause on Lost Focus",   "Pause emulation when the window loses focus.",
    /* emuHideMouse */       "Hide Mouse",            "Hide the mouse cursor while the emulator window is active.",
    /* emuHideMouseAfter */  "Hide Mouse After",      "Automatically hide the cursor after this period of inactivity.",
    /* emuJit */             "JIT Recompiler",        "Improves performance but may reduce stability. Takes effect on the next ROM load.",
    // Display
    /* displayRenderer */    "3D Renderer",           "Graphics renderer for 3D scenes. OpenGL modes require GPU support.",
    /* displayResolution */  "3D Resolution",         "Internal rendering scale for 3D graphics.",
    /* displayVSync */       "VSync",                 "Synchronize rendering to your display refresh rate.",
    /* displayThemeColor */  "Theme Color",           "Settings menu accent color inspired by each Kingdom Hearts title.",
    // Sound
    /* soundVolume */        "Volume",                "Game audio output level.",
    /* soundBgmVolume */     "BGM Volume",            "Background music audio output level.",
    /* soundInterpolation */ "Interpolation",         "Audio resampling quality. Higher settings improve fidelity but use more CPU.",
    /* soundBitDepth */      "Bit Depth",             "Audio sample bit depth. 10-bit mimics original DS hardware.",
    /* soundDSiVolumeSync */ "DSi Volume Sync",       "Sync DS audio volume with the DSi firmware volume level.",
    /* soundMicInput */      "Mic Input",             "Microphone input source for games that use the DS microphone.",
    // System
    /* systemWifiMode */     "WiFi Mode",             "Connection mode for wireless features.",
    /* systemWifiAdapter */  "WiFi Adapter",          "Network adapter used for Direct Mode. Only active when Direct is selected.",
    /* systemDSBattery */    "DS Battery",            "Simulated DS battery level for games that check it.",
    /* systemDSiBattery */   "DSi Battery Level",     "Simulated DSi battery charge level.",
    /* systemDSiCharging */  "DSi Charging",          "Whether the simulated DSi battery is charging.",

    /* streamPara1 */
        "In order to stream Melon Mix with OBS, capture your screen or it won't record "
        "the cutscenes, and capture your whole audio or it won't record the replacement BGM.",
    /* streamPara2 */
        "This game is a copyrighted work. The copyright is held by The Walt Disney "
        "Company and a collaboration of authors representing The Walt Disney Company. "
        "Additionally, the copyright of certain characters is held by Square Enix Co., Ltd.",
    /* streamPara3 */
        "You are free to stream this game in non-commercial contexts. However, using "
        "streams of the game to primarily provide or listen to the music is prohibited "
        "even in such non-commercial contexts.",
    /* streamPara4 */
        "For information on the terms of use relating to streaming the game, please see "
        "the official KINGDOM HEARTS site.",
    /* streamOpenHint */ "Open a browser to view the official website.",
    /* quitBody */
        "You are about to quit the current game. "
        "Any progress since your last save will be lost.",
};

const SettingsLocale kLocales[6] = {
    kEnglish,  // 0  English
    kEnglish,  // 1  Japanese  (stub — fill in fields to localize)
    kEnglish,  // 2  French    (stub)
    kEnglish,  // 3  German    (stub)
    kEnglish,  // 4  Italian   (stub)
    kEnglish,  // 5  Spanish   (stub)
};
