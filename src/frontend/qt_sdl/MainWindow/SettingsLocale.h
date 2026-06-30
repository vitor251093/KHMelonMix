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

#ifndef SETTINGSLOCALE_H
#define SETTINGSLOCALE_H

struct SettingsLocale
{
    // ── General ───────────────────────────────────────────────────────────────
    const char* on;
    const char* off;
    const char* none;               // audio pack "None" entry and member-level uses
    const char* titleGameSettings;

    // ── Sidebar (9 items, indexed by kIdx*) ───────────────────────────────────
    const char* sidebarLabels[9];
    const char* sectionOverview[9];

    // ── Action bar hints ──────────────────────────────────────────────────────
    const char* hintToggle;
    const char* hintSelect;
    const char* hintAdjust;
    const char* hintReset;
    const char* hintBack;
    const char* hintEnter;
    const char* hintClose;
    const char* hintConfirm;
    const char* hintRebind;
    const char* hintResetAll;
    const char* hintClear;          // remap action bar: clear selected binding (Y / Delete)
    const char* remapSelectPrompt;  // "Select an action to assign."
    const char* remapResetBackHint; // "X Reset All  |  B Back to Sidebar"
    const char* hintOpenWebsite;
    const char* hintQuitGame;
    const char* hintYes;
    const char* hintNo;
    const char* hintCancel;

    // ── Capture overlay ───────────────────────────────────────────────────────
    const char* captureController;     // "Press a controller button\xE2\x80\xA6"
    const char* captureKey;            // "Press a key\xE2\x80\xA6"
    const char* captureCancelKeyboard; // "Press Esc to cancel"
    const char* captureReleaseHint;    // "Release controller\xE2\x80\xA6" (shown before the pad is at rest)
    const char* captureCancelCountdown;// "Cancelling in %1s" — use with QString::arg()

    // ── Confirmation dialogs ──────────────────────────────────────────────────
    const char* resetSectionPrompt; // "Reset %1 to defaults?" — use with QString::arg()
    const char* resetAllBindings;   // "Reset all bindings?"
    const char* pillYes;            // "Yes" — caller prepends "A "
    const char* pillNo;             // "No"  — caller prepends "B "

    // ── No KH game note (shown in Game section when no KH ROM is loaded) ──────
    const char* noKHGameNote;

    // ── KH-specific row labels and descriptions ───────────────────────────────
    // Game section
    const char* gameLanguageLabel;
    const char* gameLanguageDesc;
    const char* gameFastForwardLabel;
    const char* gameFastForwardDesc;
    const char* gameSkipCutscenesLabel;
    const char* gameSkipCutscenesDesc;
    const char* gameHisMemoriesLabel;
    const char* gameHisMemoriesDesc;

    // Display section (KH-guarded rows)
    const char* displayEnhancedLabel;
    const char* displayEnhancedDesc;
    const char* displaySingleScreenLabel;
    const char* displaySingleScreenDesc;
    const char* displaySubtitlesLabel;
    const char* displaySubtitlesDesc;
    const char* displayHUDScaleLabel;
    const char* displayHUDScaleDesc;

    // Sound section (KH-guarded row)
    const char* soundAudioPackLabel;
    const char* soundAudioPackDesc;

    // Keyboard section (KH-guarded rows)
    const char* keyboardCamSensLabel;
    const char* keyboardCamSensDesc;
    const char* keyboardBindingsLabel;
    const char* keyboardBindingsDesc;

    // ── Generic (non-KH) setting rows ─────────────────────────────────────────
    // Emulation
    const char* emuConsoleTypeLabel;     const char* emuConsoleTypeDesc;
    const char* emuDirectBootLabel;      const char* emuDirectBootDesc;
    const char* emuFpsLimitLabel;        const char* emuFpsLimitDesc;
    const char* emuTargetFpsLabel;       const char* emuTargetFpsDesc;
    const char* emuFastForwardFpsLabel;  const char* emuFastForwardFpsDesc;
    const char* emuSlowmoFpsLabel;       const char* emuSlowmoFpsDesc;
    const char* emuMuteFastForwardLabel; const char* emuMuteFastForwardDesc;
    const char* emuPauseLostFocusLabel;  const char* emuPauseLostFocusDesc;
    const char* emuHideMouseLabel;       const char* emuHideMouseDesc;
    const char* emuHideMouseAfterLabel;  const char* emuHideMouseAfterDesc;
    // Display
    const char* displayRendererLabel;    const char* displayRendererDesc;
    const char* displayResolutionLabel;  const char* displayResolutionDesc;
    const char* displayVSyncLabel;       const char* displayVSyncDesc;
    const char* displayThemeColorLabel;  const char* displayThemeColorDesc;
    // Sound
    const char* soundVolumeLabel;        const char* soundVolumeDesc;
    const char* soundInterpolationLabel; const char* soundInterpolationDesc;
    const char* soundBitDepthLabel;      const char* soundBitDepthDesc;
    const char* soundDSiVolumeSyncLabel; const char* soundDSiVolumeSyncDesc;
    const char* soundMicInputLabel;      const char* soundMicInputDesc;
    // System
    const char* systemWifiModeLabel;     const char* systemWifiModeDesc;
    const char* systemWifiAdapterLabel;  const char* systemWifiAdapterDesc;
    const char* systemDSBatteryLabel;    const char* systemDSBatteryDesc;
    const char* systemDSiBatteryLabel;   const char* systemDSiBatteryDesc;
    const char* systemDSiChargingLabel;  const char* systemDSiChargingDesc;

    // Stream page body + Quit page body
    const char* streamPara1;
    const char* streamPara2;
    const char* streamPara3;
    const char* streamOpenHint;
    const char* quitBody;
};

// Indexed by display language: 0=English, 1=Japanese, 2=French, 3=German, 4=Italian, 5=Spanish
// (same index mapping as the Language combobox display index in rowsFor)
extern const SettingsLocale kLocales[6];

#endif // SETTINGSLOCALE_H
