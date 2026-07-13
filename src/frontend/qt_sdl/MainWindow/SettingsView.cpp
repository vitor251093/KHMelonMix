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

#include <cmath>
#include <SDL2/SDL.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QKeySequence>
#include <QDesktopServices>
#include <QNetworkInterface>
#include <QUrl>

#include "SettingsView.h"
#include "SettingsLocale.h"
#include "MainWindowSettings.h"
#include "EmuInstance.h"
#include "plugins/Plugin.h"
#include "InputConfig/BindingName.h"

using namespace Plugins;

// ─── pill path ────────────────────────────────────────────────────────────────

static void addKHPillPath(QPainterPath& path, QRectF r, qreal radius)
{
    path.moveTo(r.left() + radius, r.top());
    path.lineTo(r.right(), r.top());
    path.lineTo(r.right(), r.bottom() - radius);
    path.arcTo(r.right() - radius * 2, r.bottom() - radius * 2,
               radius * 2, radius * 2, 0, -90);
    path.lineTo(r.left(), r.bottom());
    path.lineTo(r.left(), r.top() + radius);
    path.arcTo(r.left(), r.top(), radius * 2, radius * 2, 180, -90);
    path.closeSubpath();
}

static void addRoundedPillPath(QPainterPath& path, QRectF r)
{
    qreal rad = r.height() / 2.0;
    path.addRoundedRect(r, rad, rad);
}

static void addArrowPillPath(QPainterPath& path, QRectF r)
{
    qreal rad  = r.height() / 2.0;
    qreal tipX = r.right() - rad + rad * std::sqrt(3.0);
    path.moveTo(r.left() + rad, r.top());
    path.lineTo(r.right() - rad, r.top());
    path.lineTo(tipX, r.center().y());
    path.lineTo(r.right() - rad, r.bottom());
    path.lineTo(r.left() + rad, r.bottom());
    path.arcTo(r.left(), r.top(), rad * 2, rad * 2, 270, -180);
    path.closeSubpath();
}

// ─── data ─────────────────────────────────────────────────────────────────────

static constexpr int kSidebarCount = 9;

// Sidebar indices for special-cased categories
static constexpr int kIdxGame       = 0;
static constexpr int kIdxEmulation  = 1;
static constexpr int kIdxDisplay    = 2;
static constexpr int kIdxSound      = 3;
static constexpr int kIdxGamepad    = 4;
static constexpr int kIdxKeyboard   = 5;
static constexpr int kIdxSystem     = 6;
static constexpr int kIdxStream     = 7;
static constexpr int kIdxQuit       = 8;

struct ThemePreset { const char* label; QRgb rgb; };
static const ThemePreset kThemePresets[] = {
    { "Auto",                0xFF3C3C3C },
    { "358/2 Days",          0xFFC0380E },
    { "Re:coded",            0xFFB8920E },
    { "Kingdom Hearts",      0xFF2A58A8 },
    { "Birth by Sleep",      0xFF4A9EC0 },
    { "KH III",              0xFF1A2A72 },
    { "Dream Drop Distance", 0xFFC01878 },
    { "Melody of Memory",    0xFF8B6E14 },
    { "KH IV",               0xFF3C3C3C },
};
static constexpr int kThemePresetCount = 9;

// FPS preset values
static const double kTargetFPSPresets[] = { 30.0, 60.0, 120.0 };
static constexpr int kTargetFPSCount = 3;
static const double kFFPSPresets[]    = { 120.0, 240.0, 480.0, 1000.0 };
static constexpr int kFFPSCount = 4;
static const double kSlowmoFPSPresets[] = { 15.0, 30.0, 45.0 };
static constexpr int kSlowmoFPSCount = 3;

// Mouse hide second presets
static const int kMouseHidePresets[] = { 3, 5, 10, 30 };
static constexpr int kMouseHideCount = 4;

// DSi battery level presets (stored as 0-15 integer)
static const int kDsiBattLevels[] = { 0, 4, 8, 12, 15 };
static constexpr int kDsiBattCount = 5;

// Touch-screen remap actions
static const struct { const char* label; const char* key; } kTouchActions[] = {
    { "Touch Left",  "CameraLeft"  },
    { "Touch Right", "CameraRight" },
    { "Touch Up",    "CameraUp"    },
    { "Touch Down",  "CameraDown"  },
};

// ─── binding display helpers ──────────────────────────────────────────────────

static QString keyBindingText(int key)
{
    if (key == -1 || key == 0) return "None";
    QString isright = (key & (1 << 31)) ? "Right " : "";
    key &= ~(1 << 31);
    switch (key)
    {
    case Qt::Key_Control: return isright + "Ctrl";
    case Qt::Key_Alt:     return "Alt";
    case Qt::Key_AltGr:   return "AltGr";
    case Qt::Key_Shift:   return isright + "Shift";
    case Qt::Key_Meta:    return "Meta";
    }
    return QKeySequence(key).toString(QKeySequence::NativeText);
}

// ─── row definitions ──────────────────────────────────────────────────────────

QVector<SettingRow> SettingsView::rowsFor(int idx) const
{
    QVector<SettingRow> rows;
    const SettingsLocale& loc = locale();
    EmuInstance* emu = m_mainWindow ? m_mainWindow->getEmuInstance() : nullptr;

    switch (idx)
    {
    case kIdxGame:
    {
        if (!emu) break;
        auto* gcfg = &emu->getGlobalConfig();

        // Language is always available regardless of which ROM is loaded
        rows.append({ SettingRow::Type::Combobox, loc.gameLanguageLabel,
            loc.gameLanguageDesc,
            {"English", "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e", "Fran\xc3\xa7" "ais", "Deutsch", "Italiano", "Espa\xc3\xb1ol"} });
        rows.last().read  = [gcfg]() {
            int v = gcfg->GetInt("Instance0.Firmware.Language");
            return (v == 1) ? 0 : (v == 0) ? 1 : v; // stored: Eng=1,Jap=0; displayed: Eng=0,Jap=1
        };
        rows.last().write = [gcfg, emu](int v) {
            gcfg->SetInt("Instance0.Firmware.Language", (v == 0) ? 1 : (v == 1) ? 0 : v);
            Config::Save();
            if (emu->plugin) emu->plugin->shouldInvalidateConfigs = true;
        };
        rows.last().reset = [gcfg, emu]() {
            gcfg->SetInt("Instance0.Firmware.Language", 1); // English
            Config::Save();
            if (emu->plugin) emu->plugin->shouldInvalidateConfigs = true;
        };

        // KH-specific rows — only when Days or Re:coded is actively running
        if (!emu->plugin || !emu->emuIsActive()) break;
        if (!emu->plugin->supportsKHExtendedSettings()) break;
        bool canDisableHisMemories = emu->plugin->supportsDisableHisMemories();

        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        auto pluginSave = [emu]() { Config::Save(); emu->plugin->shouldInvalidateConfigs = true; };

        rows.append({ SettingRow::Type::Toggle, loc.gameFastForwardLabel,
            loc.gameFastForwardDesc });
        rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "FastForwardLoadingScreens") ? 1 : 0; };
        rows.last().write = [gcfg, prefix, pluginSave](int v) { gcfg->SetBool(prefix + "FastForwardLoadingScreens", v); pluginSave(); };
        rows.last().reset = [gcfg, prefix, pluginSave]() { gcfg->SetBool(prefix + "FastForwardLoadingScreens", false); pluginSave(); };

        rows.append({ SettingRow::Type::Toggle, loc.gameSkipCutscenesLabel,
            loc.gameSkipCutscenesDesc });
        rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "InstantSkipCutsceneOnStart") ? 1 : 0; };
        rows.last().write = [gcfg, prefix, pluginSave](int v) { gcfg->SetBool(prefix + "InstantSkipCutsceneOnStart", v); pluginSave(); };
        rows.last().reset = [gcfg, prefix, pluginSave]() { gcfg->SetBool(prefix + "InstantSkipCutsceneOnStart", false); pluginSave(); };

        if (canDisableHisMemories)
        {
            rows.append({ SettingRow::Type::Toggle, loc.gameHisMemoriesLabel,
                loc.gameHisMemoriesDesc });
            rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "DaysDisableHisMemories") ? 1 : 0; };
            rows.last().write = [gcfg, prefix, pluginSave](int v) { gcfg->SetBool(prefix + "DaysDisableHisMemories", v); pluginSave(); };
            rows.last().reset = [gcfg, prefix, pluginSave]() { gcfg->SetBool(prefix + "DaysDisableHisMemories", false); pluginSave(); };
        }
        break;
    }
    case kIdxEmulation:
    {
        if (!emu) break;
        auto* gcfg = &emu->getGlobalConfig();
        auto saveOnly = []() { Config::Save(); };

        rows.append({ SettingRow::Type::Combobox, loc.emuConsoleTypeLabel,
            loc.emuConsoleTypeDesc,
            {"DS", "DSi"} });
        rows.last().read  = [gcfg]() { return gcfg->GetInt("Emu.ConsoleType"); };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetInt("Emu.ConsoleType", v); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetInt("Emu.ConsoleType", 0); saveOnly(); };

        rows.append({ SettingRow::Type::Toggle, loc.emuDirectBootLabel,
            loc.emuDirectBootDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("Emu.DirectBoot") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("Emu.DirectBoot", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("Emu.DirectBoot", true); saveOnly(); };

#ifdef JIT_ENABLED
        rows.append({ SettingRow::Type::Toggle, loc.emuJitLabel,
            loc.emuJitDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("JIT.Enable") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("JIT.Enable", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("JIT.Enable", false); saveOnly(); };
#endif

        rows.append({ SettingRow::Type::Toggle, loc.emuFpsLimitLabel,
            loc.emuFpsLimitDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("LimitFPS") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("LimitFPS", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("LimitFPS", true); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.emuTargetFpsLabel,
            loc.emuTargetFpsDesc,
            {"30", "60", "120"} });
        rows.last().read  = [gcfg]() {
            double v = gcfg->GetDouble("TargetFPS");
            for (int i = 0; i < kTargetFPSCount; i++) if (qAbs(v - kTargetFPSPresets[i]) < 0.5) return i;
            return 1;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v < kTargetFPSCount) gcfg->SetDouble("TargetFPS", kTargetFPSPresets[v]);
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetDouble("TargetFPS", 60.0); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.emuFastForwardFpsLabel,
            loc.emuFastForwardFpsDesc,
            {"2\xC3\x97", "4\xC3\x97", "8\xC3\x97", "Unlimited"} });
        rows.last().read  = [gcfg]() {
            double v = gcfg->GetDouble("FastForwardFPS");
            for (int i = 0; i < kFFPSCount; i++) if (qAbs(v - kFFPSPresets[i]) < 0.5) return i;
            return 3;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v < kFFPSCount) gcfg->SetDouble("FastForwardFPS", kFFPSPresets[v]);
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetDouble("FastForwardFPS", 1000.0); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.emuSlowmoFpsLabel,
            loc.emuSlowmoFpsDesc,
            {"25%", "50%", "75%"} });
        rows.last().read  = [gcfg]() {
            double v = gcfg->GetDouble("SlowmoFPS");
            for (int i = 0; i < kSlowmoFPSCount; i++) if (qAbs(v - kSlowmoFPSPresets[i]) < 0.5) return i;
            return 1;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v < kSlowmoFPSCount) gcfg->SetDouble("SlowmoFPS", kSlowmoFPSPresets[v]);
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetDouble("SlowmoFPS", 30.0); saveOnly(); };

        rows.append({ SettingRow::Type::Toggle, loc.emuMuteFastForwardLabel,
            loc.emuMuteFastForwardDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("MuteFastForward") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("MuteFastForward", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("MuteFastForward", false); saveOnly(); };

        rows.append({ SettingRow::Type::Toggle, loc.emuPauseLostFocusLabel,
            loc.emuPauseLostFocusDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("PauseLostFocus") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("PauseLostFocus", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("PauseLostFocus", false); saveOnly(); };

        rows.append({ SettingRow::Type::Toggle, loc.emuHideMouseLabel,
            loc.emuHideMouseDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("Mouse.Hide") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("Mouse.Hide", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("Mouse.Hide", true); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.emuHideMouseAfterLabel,
            loc.emuHideMouseAfterDesc,
            {"3 seconds", "5 seconds", "10 seconds", "30 seconds"} });
        rows.last().read  = [gcfg]() {
            int v = gcfg->GetInt("Mouse.HideSeconds");
            for (int i = 0; i < kMouseHideCount; i++) if (v == kMouseHidePresets[i]) return i;
            return 0;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v < kMouseHideCount) gcfg->SetInt("Mouse.HideSeconds", kMouseHidePresets[v]);
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetInt("Mouse.HideSeconds", 3); saveOnly(); };
        break;
    }
    case kIdxDisplay:
    {
        if (!emu) break;
        auto* gcfg    = &emu->getGlobalConfig();
        auto saveVideo = [emu]() { Config::Save(); emu->getEmuThread()->updateVideoSettings(); };
        auto saveOnly  = []()    { Config::Save(); };

        rows.append({ SettingRow::Type::Combobox, loc.displayRendererLabel,
            loc.displayRendererDesc,
            {"Software", "OpenGL", "OpenGL Compute"} });
        rows.last().read  = [gcfg]() { return gcfg->GetInt("3D.Renderer"); };
        rows.last().write = [gcfg, saveVideo](int v) { gcfg->SetInt("3D.Renderer", v); saveVideo(); };
        rows.last().reset = [gcfg, saveVideo]() { gcfg->SetInt("3D.Renderer", 1); saveVideo(); };

        rows.append({ SettingRow::Type::Combobox, loc.displayResolutionLabel,
            loc.displayResolutionDesc,
            {"1x native (256x192)",   "2x native (512x384)",   "3x native (768x576)",   "4x native (1024x768)",
             "5x native (1280x960)",  "6x native (1536x1152)", "7x native (1792x1344)", "8x native (2048x1536)",
             "9x native (2304x1728)", "10x native (2560x1920)","11x native (2816x2112)","12x native (3072x2304)",
             "13x native (3328x2496)","14x native (3584x2688)","15x native (3840x2880)","16x native (4096x3072)"} });
        rows.last().read  = [gcfg]() { return qBound(0, gcfg->GetInt("3D.GL.ScaleFactor") - 1, 15); };
        rows.last().write = [gcfg, saveVideo](int v) { gcfg->SetInt("3D.GL.ScaleFactor", v + 1); saveVideo(); };
        rows.last().reset = [gcfg, saveVideo]() { gcfg->SetInt("3D.GL.ScaleFactor", 3); saveVideo(); };

        rows.append({ SettingRow::Type::Toggle, loc.displayVSyncLabel,
            loc.displayVSyncDesc });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("Screen.VSync") ? 1 : 0; };
        rows.last().write = [gcfg, saveVideo](int v) { gcfg->SetBool("Screen.VSync", v != 0); saveVideo(); };
        rows.last().reset = [gcfg, saveVideo]() { gcfg->SetBool("Screen.VSync", false); saveVideo(); };

        rows.append({ SettingRow::Type::Combobox, loc.displayThemeColorLabel,
            loc.displayThemeColorDesc,
            {"Auto", "358/2 Days", "Re:coded", "Kingdom Hearts",
             "Birth by Sleep", "KH III", "Dream Drop Distance", "Melody of Memory", "KH IV"},
            0, 0, SettingRow::Tag::ThemeColorPicker });
        rows.last().read  = [gcfg]() {
            QString saved = gcfg->GetQString("KHSettings.ThemeColor");
            if (!saved.isEmpty())
            {
                QColor c(saved);
                for (int i = 1; i < kThemePresetCount; i++)
                    if (QColor(kThemePresets[i].rgb).rgb() == c.rgb()) return i;
            }
            return 0;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v >= 0 && v < kThemePresetCount)
            {
                if (v == 0) gcfg->SetQString("KHSettings.ThemeColor", QString());
                else        gcfg->SetQString("KHSettings.ThemeColor", QColor(kThemePresets[v].rgb).name());
            }
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetQString("KHSettings.ThemeColor", QString()); saveOnly(); };

        if (emu->plugin && emu->emuIsActive())
        {
            if (emu->plugin->supportsKHExtendedSettings())
            {
                std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                auto savePlugin = [emu]() { Config::Save(); emu->plugin->shouldInvalidateConfigs = true; };

                rows.append({ SettingRow::Type::Toggle, loc.displayEnhancedLabel,
                    loc.displayEnhancedDesc });
                rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "DisableEnhancedGraphics") ? 0 : 1; };
                rows.last().write = [gcfg, prefix, savePlugin](int v) { gcfg->SetBool(prefix + "DisableEnhancedGraphics", !v); savePlugin(); };
                rows.last().reset = [gcfg, prefix, savePlugin]() { gcfg->SetBool(prefix + "DisableEnhancedGraphics", false); savePlugin(); };

                rows.append({ SettingRow::Type::Toggle, loc.displaySingleScreenLabel,
                    loc.displaySingleScreenDesc });
                rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "DisableSingleScreenMode") ? 0 : 1; };
                rows.last().write = [gcfg, prefix, savePlugin](int v) { gcfg->SetBool(prefix + "DisableSingleScreenMode", !v); savePlugin(); };
                rows.last().reset = [gcfg, prefix, savePlugin]() { gcfg->SetBool(prefix + "DisableSingleScreenMode", false); savePlugin(); };

                rows.append({ SettingRow::Type::Toggle, loc.displaySubtitlesLabel,
                    loc.displaySubtitlesDesc });
                rows.last().read  = [gcfg, prefix]() { return gcfg->GetBool(prefix + "SubtitlesEnabled") ? 1 : 0; };
                rows.last().write = [gcfg, prefix, savePlugin](int v) { gcfg->SetBool(prefix + "SubtitlesEnabled", v); savePlugin(); };
                rows.last().reset = [gcfg, prefix, savePlugin]() { gcfg->SetBool(prefix + "SubtitlesEnabled", true); savePlugin(); };

                rows.append({ SettingRow::Type::Slider, loc.displayHUDScaleLabel,
                    loc.displayHUDScaleDesc, {}, 1, 10 });
                rows.last().read  = [gcfg, prefix]() { int v = gcfg->GetInt(prefix + "HUDScale"); return v == 0 ? 4 : v; };
                rows.last().write = [gcfg, prefix, savePlugin](int v) { gcfg->SetInt(prefix + "HUDScale", v); savePlugin(); };
                rows.last().reset = [gcfg, prefix, savePlugin]() { gcfg->SetInt(prefix + "HUDScale", 4); savePlugin(); };
            }
        }
        break;
    }
    case kIdxSound:
    {
        if (!emu) break;
        auto* gcfg = &emu->getGlobalConfig();
        auto* lcfg = &emu->getLocalConfig();
        auto saveAudio = [emu]() { Config::Save(); emu->applyAudioSettings(); };

        rows.append({ SettingRow::Type::Slider, loc.soundVolumeLabel,
            loc.soundVolumeDesc, {}, 0, 10 });
        rows.last().read  = [lcfg]()      { return lcfg->GetInt("Audio.Volume") / 25; };
        rows.last().write = [emu, lcfg, saveAudio](int v) {
            int vol = qBound(0, v * 25, 256);
            lcfg->SetInt("Audio.Volume", vol);
            emu->setAudioVolume(vol);
            saveAudio();
        };
        rows.last().reset = [emu, lcfg, saveAudio]() {
            lcfg->SetInt("Audio.Volume", 256);
            emu->setAudioVolume(256);
            saveAudio();
        };

        rows.append({ SettingRow::Type::Slider, loc.soundBgmVolumeLabel,
            loc.soundBgmVolumeDesc, {}, 0, 10 });
        rows.last().read  = [lcfg]()      { return lcfg->GetInt("Audio.BGMVolume") / 10; };
        rows.last().write = [emu, lcfg, saveAudio](int v) {
            int vol = qBound(0, v * 10, 100);
            lcfg->SetInt("Audio.BGMVolume", vol);
            emu->setAudioVolume(vol);
            saveAudio();
        };
        rows.last().reset = [emu, lcfg, saveAudio]() {
            lcfg->SetInt("Audio.BGMVolume", 100);
            emu->setAudioVolume(100);
            saveAudio();
        };

        rows.append({ SettingRow::Type::Combobox, loc.soundInterpolationLabel,
            loc.soundInterpolationDesc,
            {"None", "Linear", "Cosine", "Cubic", "Gaussian"} });
        rows.last().read  = [gcfg]()      { return gcfg->GetInt("Audio.Interpolation"); };
        rows.last().write = [gcfg, saveAudio](int v) { gcfg->SetInt("Audio.Interpolation", v); saveAudio(); };
        rows.last().reset = [gcfg, saveAudio]() { gcfg->SetInt("Audio.Interpolation", 0); saveAudio(); };

        rows.append({ SettingRow::Type::Combobox, loc.soundBitDepthLabel,
            loc.soundBitDepthDesc,
            {"Auto", "10-bit", "16-bit"} });
        rows.last().read  = [gcfg]()      { return gcfg->GetInt("Audio.BitDepth"); };
        rows.last().write = [gcfg, saveAudio](int v) { gcfg->SetInt("Audio.BitDepth", v); saveAudio(); };
        rows.last().reset = [gcfg, saveAudio]() { gcfg->SetInt("Audio.BitDepth", 0); saveAudio(); };

        rows.append({ SettingRow::Type::Toggle, loc.soundDSiVolumeSyncLabel,
            loc.soundDSiVolumeSyncDesc });
        rows.last().enabled = [gcfg]() { return gcfg->GetInt("Emu.ConsoleType") == 1; };
        rows.last().read  = [lcfg]()      { return lcfg->GetBool("Audio.DSiVolumeSync") ? 1 : 0; };
        rows.last().write = [lcfg, saveAudio](int v) { lcfg->SetBool("Audio.DSiVolumeSync", v != 0); saveAudio(); };
        rows.last().reset = [lcfg, saveAudio]() { lcfg->SetBool("Audio.DSiVolumeSync", false); saveAudio(); };

        rows.append({ SettingRow::Type::Combobox, loc.soundMicInputLabel,
            loc.soundMicInputDesc,
            {"None", "External", "Noise", "WAV"} });
        rows.last().read  = [gcfg]()      { return gcfg->GetInt("Mic.InputType"); };
        rows.last().write = [gcfg, saveAudio](int v) { gcfg->SetInt("Mic.InputType", v); saveAudio(); };
        rows.last().reset = [gcfg, saveAudio]() { gcfg->SetInt("Mic.InputType", 1); saveAudio(); };

        if (emu->plugin && emu->emuIsActive())
        {
            if (emu->plugin->supportsKHExtendedSettings())
            {
                rows.append({ SettingRow::Type::Combobox, loc.soundAudioPackLabel,
                    loc.soundAudioPackDesc });
                rows.last().isDynamic = true;
                rows.last().read  = [this, gcfg, emu]() -> int {
                    if (!emu->plugin || m_audioPackNames.isEmpty()) return 0;
                    std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                    QString saved = gcfg->GetQString(prefix + "AudioPack");
                    if (saved.isEmpty()) return 0;
                    for (int i = 1; i < m_audioPackNames.size(); i++)
                        if (m_audioPackNames[i] == saved) return i;
                    return 0;
                };
                rows.last().write = [this, gcfg, emu, saveAudio](int v) {
                    if (!emu->plugin) return;
                    std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                    if (v == 0)
                        gcfg->SetQString(prefix + "AudioPack", QString());
                    else if (v > 0 && v < m_audioPackNames.size())
                        gcfg->SetQString(prefix + "AudioPack", m_audioPackNames[v]);
                    emu->plugin->shouldInvalidateConfigs = true;
                    saveAudio();
                };
                rows.last().dynamicOptions   = [this]() { return m_audioPackNames; };
                rows.last().dynamicValueText = [this, gcfg, emu]() -> QString {
                    if (!emu->plugin) return QString::fromUtf8(locale().none);
                    std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                    QString saved = gcfg->GetQString(prefix + "AudioPack");
                    return saved.isEmpty() ? QString::fromUtf8(locale().none) : saved;
                };
                rows.last().reset = [gcfg, emu, saveAudio]() {
                    if (!emu->plugin) return;
                    std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                    gcfg->SetQString(prefix + "AudioPack", QString());
                    emu->plugin->shouldInvalidateConfigs = true;
                    saveAudio();
                };
            }
        }
        break;
    }
    case kIdxGamepad:
    {
        if (!emu || !emu->plugin || !emu->emuIsActive()) break;
        if (!emu->plugin->supportsKHExtendedSettings()) break;
        auto* gcfg = &emu->getGlobalConfig();
        auto* lcfg = &emu->getLocalConfig();
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        auto savePlugin = [emu]() { Config::Save(); emu->plugin->shouldInvalidateConfigs = true; };

        rows.append({ SettingRow::Type::Combobox, loc.gamepadConfirmBtnLabel,
            loc.gamepadConfirmBtnDesc,
        {"A", "B"} });
        rows.last().read  = [lcfg]()      { return lcfg->GetInt("JoystickConfirmIndex"); };
        rows.last().write = [lcfg, savePlugin](int v) { lcfg->SetInt("JoystickConfirmIndex", v); savePlugin(); };
        rows.last().reset = [lcfg, savePlugin]() { lcfg->SetInt("JoystickConfirmIndex", 0); savePlugin(); };

        rows.append({ SettingRow::Type::Slider, loc.gamepadCamSensLabel,
            loc.gamepadCamSensDesc, {}, 1, 10 });
        rows.last().read  = [gcfg, prefix]() { int v = gcfg->GetInt(prefix + "CameraSensitivity"); return v == 0 ? 3 : v; };
        rows.last().write = [gcfg, prefix, savePlugin](int v) { gcfg->SetInt(prefix + "CameraSensitivity", v); savePlugin(); };

        rows.append({ SettingRow::Type::Navigate, loc.gamepadBindingsLabel,
            loc.gamepadBindingsDesc });
        break;
    }
    case kIdxKeyboard:
    {
        if (!emu || !emu->plugin || !emu->emuIsActive()) break;
        if (!emu->plugin->supportsKHExtendedSettings()) break;
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";

        rows.append({ SettingRow::Type::Navigate, loc.keyboardBindingsLabel,
            loc.keyboardBindingsDesc });
        break;
    }
    case kIdxSystem:
    {
        if (!emu) break;
        auto* gcfg = &emu->getGlobalConfig();
        auto saveOnly = []() { Config::Save(); };

        rows.append({ SettingRow::Type::Combobox, loc.systemWifiModeLabel,
            loc.systemWifiModeDesc,
            {"Indirect", "Direct"} });
        rows.last().read  = [gcfg]() { return gcfg->GetBool("LAN.DirectMode") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("LAN.DirectMode", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("LAN.DirectMode", false); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.systemWifiAdapterLabel,
            loc.systemWifiAdapterDesc });
        rows.last().isDynamic = true;
        rows.last().enabled = [gcfg]() { return gcfg->GetBool("LAN.DirectMode"); };
        rows.last().read  = [this, gcfg]() {
            QString saved = gcfg->GetQString("LAN.Device");
            for (int i = 0; i < m_wifiAdapters.size(); i++)
                if (m_wifiAdapters[i] == saved) return i;
            return 0;
        };
        rows.last().write = [this, gcfg, saveOnly](int v) {
            if (v >= 0 && v < m_wifiAdapters.size())
                gcfg->SetQString("LAN.Device", m_wifiAdapters[v]);
            saveOnly();
        };
        rows.last().dynamicOptions   = [this]() { return m_wifiAdapters; };
        rows.last().dynamicValueText = [gcfg]() { return gcfg->GetQString("LAN.Device"); };

        rows.append({ SettingRow::Type::Combobox, loc.systemDSBatteryLabel,
            loc.systemDSBatteryDesc,
            {"Low", "Okay"} });
        rows.last().enabled = [gcfg]() { return gcfg->GetInt("Emu.ConsoleType") == 0; };
        rows.last().read  = [gcfg]() { return gcfg->GetBool("DS.Battery.LevelOkay") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("DS.Battery.LevelOkay", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("DS.Battery.LevelOkay", true); saveOnly(); };

        rows.append({ SettingRow::Type::Combobox, loc.systemDSiBatteryLabel,
            loc.systemDSiBatteryDesc,
            {"Almost Empty", "Low", "Half", "High", "Full"} });
        rows.last().enabled = [gcfg]() { return gcfg->GetInt("Emu.ConsoleType") == 1; };
        rows.last().read  = [gcfg]() {
            int v = gcfg->GetInt("DSi.Battery.Level");
            for (int i = 0; i < kDsiBattCount; i++) if (v == kDsiBattLevels[i]) return i;
            return 2;
        };
        rows.last().write = [gcfg, saveOnly](int v) {
            if (v < kDsiBattCount) gcfg->SetInt("DSi.Battery.Level", kDsiBattLevels[v]);
            saveOnly();
        };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetInt("DSi.Battery.Level", 15); saveOnly(); };

        rows.append({ SettingRow::Type::Toggle, loc.systemDSiChargingLabel,
            loc.systemDSiChargingDesc });
        rows.last().enabled = [gcfg]() { return gcfg->GetInt("Emu.ConsoleType") == 1; };
        rows.last().read  = [gcfg]() { return gcfg->GetBool("DSi.Battery.Charging") ? 1 : 0; };
        rows.last().write = [gcfg, saveOnly](int v) { gcfg->SetBool("DSi.Battery.Charging", v != 0); saveOnly(); };
        rows.last().reset = [gcfg, saveOnly]() { gcfg->SetBool("DSi.Battery.Charging", true); saveOnly(); };
        break;
    }
    default:
        break;
    }
    return rows;
}

// ─── SettingsView ─────────────────────────────────────────────────────────────

SettingsView::SettingsView(MainWindowSettings* mainWindow)
    : QWidget(nullptr),
      m_mainWindow(mainWindow)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);

    m_background = QPixmap(":/ds/settings_bg.png");
    m_gearPixmap = QPixmap(":/ds/gear.svg");

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() { update(); });

    m_joyTimer = new QTimer(this);
    m_joyTimer->setInterval(30);
    connect(m_joyTimer, &QTimer::timeout, this, &SettingsView::pollJoystick);
}

void SettingsView::resetToFirstScreen()
{
    currentScreen     = Screen::Sidebar;
    sidebarIndex      = 0;
    detailIndex       = 0;
    optionIndex       = 0;

    m_pendingClose    = false;
    m_waitingForBind  = false;
    m_bindArmed       = false;
    m_resettingBindings = false;
    m_resettingSection  = false;
    m_detailScrollOffset = 0;
    m_remapScrollOffset  = 0;
    m_optionScrollOffset = 0;
    m_draggingSlider  = false;
    update();
}

void SettingsView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_animClock.restart();
    m_lastJoyInput = 0;
    m_animTimer->start();
    m_joyTimer->start();
    setFocus();
}

void SettingsView::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    m_animTimer->stop();
    m_joyTimer->stop();
    m_pendingClose    = false;
    m_waitingForBind  = false;
    m_bindArmed       = false;
    m_resettingBindings = false;
    m_resettingSection  = false;
    m_optionScrollOffset = 0;
    m_draggingSlider  = false;
}

// ─── theme color ──────────────────────────────────────────────────────────────

QColor SettingsView::currentThemeColor() const
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return QColor(60, 60, 60);

    // Live preview while browsing Theme Color option list
    if (currentScreen == Screen::OptionList)
    {
        QVector<SettingRow> rows = rowsFor(sidebarIndex);
        if (detailIndex >= 0 && detailIndex < rows.size() &&
            rows[detailIndex].tag == SettingRow::Tag::ThemeColorPicker &&
            optionIndex > 0 && optionIndex < kThemePresetCount)
        {
            return QColor(kThemePresets[optionIndex].rgb);
        }
    }

    QString saved = emu->getGlobalConfig().GetQString("KHSettings.ThemeColor");
    if (!saved.isEmpty())
    {
        QColor c(saved);
        if (c.isValid()) return c;
    }

    if (emu->plugin && emu->getEmuThread()->emuIsActive())
    {
        ThemeColor tc = emu->plugin->defaultThemeColor();
        return QColor::fromRgb(tc.r, tc.g, tc.b);
    }

    return QColor(60, 60, 60);
}

const SettingsLocale& SettingsView::locale() const
{
    EmuInstance* emu = m_mainWindow ? m_mainWindow->getEmuInstance() : nullptr;
    int fw  = emu ? emu->getGlobalConfig().GetInt("Instance0.Firmware.Language") : 1;
    int idx = (fw == 1) ? 0 : (fw == 0) ? 1 : fw;
    return kLocales[qBound(0, idx, 5)];
}

void SettingsView::scrollOptionToIdx(int idx)
{
    int dX_, dW_, sY_, bH_, sp_;
    computeDetailGeometry(dX_, dW_, sY_, bH_, sp_);
    const int h_ = height();
    const qreal bottomLineY_ = contentBottomLineY();
    int labelH_ = qMax(12, (int)(bH_ * 0.68));
    int intraGap_ = qMax(2, (int)(bH_ * 0.10));
    int optStartY_ = sY_ + labelH_ + intraGap_;
    int visH = (int)(bottomLineY_) - optStartY_ - (int)(h_ * 0.04);
    int optY = idx * (bH_ + sp_);
    if (optY < m_optionScrollOffset)
        m_optionScrollOffset = optY;
    else if (optY + bH_ + sp_ > m_optionScrollOffset + visH)
        m_optionScrollOffset = optY + bH_ + sp_ - visH;
    m_optionScrollOffset = qMax(0, m_optionScrollOffset);
    QStringList opts_ = currentOptionList();
    int n_ = opts_.size();
    int maxOff_ = qMax(0, n_ * (bH_ + sp_) - visH);
    m_optionScrollOffset = qMin(m_optionScrollOffset, maxOff_);
    if (idx == 0)      m_optionScrollOffset = 0;
    if (idx == n_ - 1) m_optionScrollOffset = maxOff_;
}

// ─── geometry ─────────────────────────────────────────────────────────────────

void SettingsView::computeTitleBarGeometry(int& barY, int& barH) const
{
    barY = (int)(height() * 0.11);
    barH = (int)(height() * 0.045);
}

qreal SettingsView::contentBottomLineY() const
{
    int barY, barH;
    computeTitleBarGeometry(barY, barH);
    return height() - (barY + barH / 2.0);
}

void SettingsView::computeSidebarGeometry(int& sX, int& sW, int& sY,
                                           int& bH, int& sp, int& dX) const
{
    const int w = width();
    const int h = height();
    int barY_d, barH_d;
    computeTitleBarGeometry(barY_d, barH_d);
    const int circleR  = 3;
    const int divGap   = (int)(h * 0.042);
    const int divStart = barY_d + barH_d + divGap;
    sX = (int)(w * 0.03);
    sW = (int)(w * 0.19);
    bH = (int)qMax(12.0, h * 0.036);
    sp = (int)(bH * 0.26);
    sY = divStart + circleR + (int)(h * 0.012);
    dX = sX + sW + (int)(w * 0.040);
}

void SettingsView::computeDetailGeometry(int& dX, int& dW, int& sY,
                                          int& bH, int& sp) const
{
    int sidebarX, sidebarW, startY, btnH, spacing, divX;
    computeSidebarGeometry(sidebarX, sidebarW, startY, btnH, spacing, divX);
    dX = divX + (int)(width() * 0.030);
    dW = width() - dX - (int)(width() * 0.025);
    sY = startY;
    bH = btnH;
    sp = spacing;
}

void SettingsView::sliderTrackGeometry(int detailX, int rowPillW, int btnH,
                                       int& trackX, int& trackW) const
{
    int numPad    = (int)(btnH * 1.1);
    trackX        = detailX + numPad + (int)(btnH * 1.5) + (int)(btnH * 0.05);
    int trackEndX = detailX + rowPillW - (int)(btnH * 1.5);
    trackW        = qMax(1, trackEndX - trackX);
}

// ─── config read / write ──────────────────────────────────────────────────────

int SettingsView::readRowValue(int sidebar, int row) const
{
    auto rows = rowsFor(sidebar);
    if (row < 0 || row >= rows.size() || !rows[row].read) return 0;
    return rows[row].read();
}

void SettingsView::writeRowValue(int sidebar, int row, int newValue)
{
    auto rows = rowsFor(sidebar);
    if (row < 0 || row >= rows.size() || !rows[row].write) return;
    rows[row].write(newValue);
}

QString SettingsView::rowDynamicValueText(int sidebar, int row) const
{
    QVector<SettingRow> rows = rowsFor(sidebar);
    if (row < 0 || row >= rows.size()) return {};
    const SettingRow& r = rows[row];
    return r.dynamicValueText ? r.dynamicValueText() : QString();
}

QStringList SettingsView::currentOptionList() const
{
    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    if (detailIndex < 0 || detailIndex >= rows.size()) return {};
    const SettingRow& row = rows[detailIndex];
    if (!row.options.isEmpty()) return row.options;
    if (row.isDynamic && row.dynamicOptions) return row.dynamicOptions();
    return {};
}

void SettingsView::populateWifiAdapters()
{
    m_wifiAdapters.clear();
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces())
        m_wifiAdapters.append(iface.humanReadableName());
}

void SettingsView::populateAudioPacks()
{
    m_audioPackNames.clear();
    m_audioPackNames.append(locale().none);
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu || !emu->plugin) return;
    auto names = emu->plugin->audioPackNames();
    for (const auto& n : names)
        m_audioPackNames.append(QString::fromStdString(n));
}

// ─── remap list ───────────────────────────────────────────────────────────────

void SettingsView::buildRemapList()
{
    m_remapItems.clear();
    m_remapActionToItemIdx.clear();

    EmuInstance* emu = m_mainWindow->getEmuInstance();

    auto addHeader = [&](const char* lbl) {
        RemapItem h;
        h.isHeader = true;
        h.label    = QString::fromUtf8(lbl);
        m_remapItems.append(h);
    };
    auto addAction = [&](const QString& lbl, const std::string& key) {
        RemapItem a;
        a.isHeader  = false;
        a.label     = lbl;
        a.configKey = key;
        m_remapActionToItemIdx.append(m_remapItems.size());
        m_remapItems.append(a);
    };

    addHeader("DS BUTTONS");
    for (int i = 0; i < 12; i++)
        addAction(QString::fromUtf8(EmuInstance::buttonNames[i]),
                  std::string(EmuInstance::buttonNames[i]));

    addHeader("TOUCH SCREEN");
    for (int i = 0; i < 4; i++)
        addAction(QString::fromUtf8(kTouchActions[i].label),
                  std::string(kTouchActions[i].key));

    if (emu && emu->plugin)
    {
        const auto& labels = emu->plugin->customKeyMappingLabels;
        const auto& keys   = emu->plugin->customKeyMappingNames;
        if (!labels.empty())
        {
            addHeader("ADD-ONS");
            for (int i = 0; i < (int)std::min(labels.size(), keys.size()); i++)
                addAction(QString::fromUtf8(labels[i]),
                          std::string(keys[i]));
        }
    }

    addHeader("GENERAL HOTKEYS");
    auto fmtHKLabel = [](const char* raw) -> QString {
        QString s = QString::fromUtf8(raw);
        if (s.startsWith("HK_")) s = s.mid(3);
        QString out;
        for (int i = 0; i < s.length(); i++) {
            if (i > 0 && s[i].isUpper() && s[i - 1].isLower())
                out += ' ';
            out += s[i];
        }
        return out;
    };
    for (int i = 0; i < HK_MAX; i++)
    {
        const char* name = EmuInstance::hotkeyNames[i];
        if (name && name[0] != '\0')
            addAction(fmtHKLabel(name), std::string(name));
    }
}

int SettingsView::readRemapBinding(int actionIdx) const
{
    if (actionIdx < 0 || actionIdx >= m_remapActionToItemIdx.size()) return -1;
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return -1;
    auto& lcfg = emu->getLocalConfig();
    const std::string& key = m_remapItems[m_remapActionToItemIdx[actionIdx]].configKey;
    if (key.empty()) return -1;

    if (m_remapIsJoystick)
    {
        int uid = lcfg.GetInt("JoystickUniqueID");
        if (uid <= 0) return -1;
        Config::Table joycfg = lcfg.GetTable("Joystick." + std::to_string(uid));
        return joycfg.GetInt(key);
    }
    else
    {
        Config::Table keycfg = lcfg.GetTable("Keyboard");
        return keycfg.GetInt(key);
    }
}

void SettingsView::writeRemapBinding(int actionIdx, int value)
{
    if (actionIdx < 0 || actionIdx >= m_remapActionToItemIdx.size()) return;
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;
    auto& lcfg = emu->getLocalConfig();
    const std::string& key = m_remapItems[m_remapActionToItemIdx[actionIdx]].configKey;
    if (key.empty()) return;

    if (m_remapIsJoystick)
    {
        int uid = lcfg.GetInt("JoystickUniqueID");
        if (uid <= 0) return;
        Config::Table joycfg = lcfg.GetTable("Joystick." + std::to_string(uid));
        joycfg.SetInt(key, value);
    }
    else
    {
        Config::Table keycfg = lcfg.GetTable("Keyboard");
        keycfg.SetInt(key, value);
    }
    Config::Save();
    emu->inputLoadConfig();
}

void SettingsView::resetAllRemapBindings()
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;
    auto& lcfg = emu->getLocalConfig();

    if (m_remapIsJoystick)
    {
        int uid = lcfg.GetInt("JoystickUniqueID");
        if (uid <= 0) return;
        Config::Table joycfg = lcfg.GetTable("Joystick." + std::to_string(uid));
        for (const RemapItem& item : m_remapItems)
            if (!item.isHeader && !item.configKey.empty())
                joycfg.SetInt(item.configKey, -1);
    }
    else
    {
        Config::Table keycfg = lcfg.GetTable("Keyboard");
        for (const RemapItem& item : m_remapItems)
            if (!item.isHeader && !item.configKey.empty())
                keycfg.SetInt(item.configKey, 0);
    }
    Config::Save();
    emu->inputLoadConfig();
}

void SettingsView::resetSectionToDefaults(int idx)
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;

    // Gamepad is not a row list — it reapplies the plugin's recommended controller bindings.
    if (idx == kIdxGamepad)
    {
        if (!emu->plugin) return;
        auto& lcfg = emu->getLocalConfig();
        int uid = lcfg.GetInt("JoystickUniqueID");
        if (uid <= 0) return;
        Config::Table joycfg = lcfg.GetTable("Joystick." + std::to_string(uid));
        auto setIntCfg = [&joycfg](std::string key, int value) { joycfg.SetInt(key, value); };
        emu->plugin->applyRecommendedJoystickMappings(setIntCfg);
        Config::Save();
        emu->inputLoadConfig();
        return;
    }

    // Every resettable row carries its own default via reset(), so this stays in lockstep with
    // rowsFor() — adding or removing a row can't leave a parallel reset switch out of sync.
    for (const SettingRow& r : rowsFor(idx))
        if (r.reset) r.reset();
}

// ─── hit testing ──────────────────────────────────────────────────────────────

int SettingsView::hitTestSidebar(QPoint pos) const
{
    int sX, sW, sY, bH, sp, dX;
    computeSidebarGeometry(sX, sW, sY, bH, sp, dX);
    if (pos.x() > dX) return -1;
    for (int i = 0; i < kSidebarCount; i++)
    {
        int y = sY + i * (bH + sp);
        if (pos.y() >= y && pos.y() < y + bH) return i;
    }
    return -1;
}

int SettingsView::hitTestDetail(QPoint pos) const
{
    int dX, dW, sY, bH, sp;
    computeDetailGeometry(dX, dW, sY, bH, sp);
    if (pos.x() < dX) return -1;
    int rowPillW = (int)(dW * 0.56) - (int)(bH * 1.7);
    if (pos.x() > dX + rowPillW) return -1;

    int labelH   = qMax(12, (int)(bH * 0.68));
    int intraGap = qMax(2,  (int)(bH * 0.10));
    int stride   = labelH + intraGap + bH + sp;

    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    for (int i = 0; i < rows.size(); i++)
    {
        int pillY = sY + i * stride - m_detailScrollOffset + labelH + intraGap;
        if (pos.y() >= pillY && pos.y() < pillY + bH) return i;
    }
    return -1;
}

int SettingsView::hitTestOptionList(QPoint pos) const
{
    int dX, dW, sY, bH, sp;
    computeDetailGeometry(dX, dW, sY, bH, sp);

    int rowPillW  = (int)(dW * 0.56) - (int)(bH * 1.7);
    int optGapW   = (int)(dW * 0.06);
    int optPanelX = dX + rowPillW + optGapW;
    if (pos.x() < optPanelX) return -1;

    int labelH    = qMax(12, (int)(bH * 0.68));
    int intraGap  = qMax(2,  (int)(bH * 0.10));
    int optStartY = sY + labelH + intraGap;

    QStringList opts = currentOptionList();
    for (int i = 0; i < opts.size(); i++)
    {
        int y = optStartY + i * (bH + sp) - m_optionScrollOffset;
        if (pos.y() >= y && pos.y() < y + bH) return i;
    }
    return -1;
}

int SettingsView::hitTestRemap(QPoint pos) const
{
    if (m_remapItems.isEmpty()) return -1;
    int dX, dW, sY, bH, sp;
    computeDetailGeometry(dX, dW, sY, bH, sp);
    if (pos.x() < dX) return -1;

    int stride = bH + sp;
    int actionIdx = 0;
    for (int itemIdx = 0; itemIdx < m_remapItems.size(); itemIdx++)
    {
        int itemY = sY + itemIdx * stride - m_remapScrollOffset;
        if (!m_remapItems[itemIdx].isHeader)
        {
            if (pos.y() >= itemY && pos.y() < itemY + bH)
                return actionIdx;
            actionIdx++;
        }
    }
    return -1;
}

// ─── joystick polling ─────────────────────────────────────────────────────────

void SettingsView::pollBindCapture(SDL_Joystick* joy)
{
    if (!m_remapIsJoystick) return; // keyboard binds are handled in keyPressEvent
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;

    const qint64 now = m_animClock.elapsed();
    const int numButtons = SDL_JoystickNumButtons(joy);
    const int numHats     = SDL_JoystickNumHats(joy);
    const int numAxes     = qMin(SDL_JoystickNumAxes(joy), 16);

    // Is anything currently held/deflected past rest?
    bool anyDown = false;
    for (int i = 0; i < numButtons && !anyDown; i++)
        if (SDL_JoystickGetButton(joy, i)) anyDown = true;
    for (int hat = 0; hat < numHats && !anyDown; hat++)
        if (SDL_JoystickGetHat(joy, hat) != SDL_HAT_CENTERED) anyDown = true;
    for (int i = 0; i < numAxes && !anyDown; i++)
        if (abs(SDL_JoystickGetAxis(joy, i) - m_axesRest[i]) >= 16384) anyDown = true;

    // Arming gate: the input that started the listen must be released before we capture a
    // new one (otherwise the confirm press that opened the listen is grabbed instantly).
    if (!m_bindArmed)
    {
        if (!anyDown)
        {
            m_bindArmed   = true;
            m_bindArmTime = now;
        }
        else if (now - m_bindListenStart >= kBindPreInputTimeoutMs)
        {
            // pad never settled (stuck/drift) — cancel, keep the existing binding
            m_waitingForBind = false;
            playSound(3);
            update();
        }
        return;
    }

    // Assignment timeout: armed and waiting, but no input arrived — cancel (keep old binding).
    if (now - m_bindArmTime >= kBindAssignTimeoutMs)
    {
        m_waitingForBind = false;
        playSound(3);
        update();
        return;
    }

    // Armed: capture the first new input.
    for (int i = 0; i < numButtons; i++)
    {
        if (SDL_JoystickGetButton(joy, i))
        {
            writeRemapBinding(m_bindTarget, i);
            m_waitingForBind = false;
            m_lastJoyInput = now;
            update();
            return;
        }
    }
    for (int hat = 0; hat < numHats; hat++)
    {
        Uint8 hv = SDL_JoystickGetHat(joy, hat);
        if (hv != SDL_HAT_CENTERED)
        {
            if      (hv & SDL_HAT_UP)    hv = SDL_HAT_UP;
            else if (hv & SDL_HAT_DOWN)  hv = SDL_HAT_DOWN;
            else if (hv & SDL_HAT_LEFT)  hv = SDL_HAT_LEFT;
            else if (hv & SDL_HAT_RIGHT) hv = SDL_HAT_RIGHT;
            int encoded = 0x100 | hv | (hat << 4);
            writeRemapBinding(m_bindTarget, encoded);
            m_waitingForBind = false;
            m_lastJoyInput = now;
            update();
            return;
        }
    }
    for (int i = 0; i < numAxes; i++)
    {
        Sint16 val  = SDL_JoystickGetAxis(joy, i);
        Sint16 rest = m_axesRest[i];
        if (abs(val - rest) >= 16384)
        {
            int axisType;
            if (rest < -16384) axisType = 2;
            else               axisType = (val > rest) ? 0 : 1;
            // 0xFFFF in the low 16 bits = "no button"; without it joystickButtonDown
            // also treats the binding as button 0.
            int encoded = 0xFFFF | 0x10000 | (axisType << 20) | (i << 24);
            writeRemapBinding(m_bindTarget, encoded);
            m_waitingForBind = false;
            m_lastJoyInput = now;
            update();
            return;
        }
    }

    // Triggers that don't surface on raw axes (e.g. NSO SNES ZL/ZR). Mirrors MapButton.h:
    // try SDL's GameController trigger axes, then a raw-HID fallback for the NSO SNES.
    SDL_GameController* gc = emu->getController();
    if (gc)
    {
        for (SDL_GameControllerAxis axis : {SDL_CONTROLLER_AXIS_TRIGGERLEFT,
                                            SDL_CONTROLLER_AXIS_TRIGGERRIGHT})
        {
            if (SDL_GameControllerGetAxis(gc, axis) > 16384)
            {
                writeRemapBinding(m_bindTarget, 0xFFFF | 0x10000 | (2 << 20) | ((int)axis << 24));
                m_waitingForBind = false;
                m_lastJoyInput = now;
                update();
                return;
            }
        }
    }
    emu->pollHidReport();
    const auto* hr = emu->getHidReport();
    if (hr[0] == 0x3F) // NSO SNES: ZL = report[1] bit 6, ZR = report[2] bit 7
    {
        int trigAxis = -1;
        if      (hr[1] & 0x40) trigAxis = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
        else if (hr[2] & 0x80) trigAxis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
        if (trigAxis >= 0)
        {
            writeRemapBinding(m_bindTarget, 0xFFFF | 0x10000 | (2 << 20) | (trigAxis << 24));
            m_waitingForBind = false;
            m_lastJoyInput = now;
            update();
        }
    }
}

void SettingsView::pollJoystick()
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;

    SDL_JoystickUpdate();

    // Self-heal: inputLoadConfig() closes the joystick on every remap/reset, and the EmuThread
    // reopen path (inputProcess) is skipped while the overlay pauses emulation. Reopen here so
    // controller input survives binds, resets, and unplug/replug while settings is open.
    emu->ensureJoystickOpen();
    SDL_Joystick* joy = emu->getJoystick();
    if (!joy) return;

    if (m_waitingForBind)
    {
        pollBindCapture(joy);
        return;
    }

    bool backNow = emu->joystickButtonDown(emu->getJoyMapping(1)) != 0;

    if (m_pendingClose)
    {
        if (!backNow)
        {
            m_pendingClose = false;
            emit settingsClosed();
        }
        return;
    }

    qint64 now = m_animClock.elapsed();
    if (now - m_lastJoyInput < 150) return;

    bool up      = emu->joystickButtonDown(emu->getJoyMapping(6)) != 0;
    bool down    = emu->joystickButtonDown(emu->getJoyMapping(7)) != 0;
    bool left    = emu->joystickButtonDown(emu->getJoyMapping(5)) != 0;
    bool right   = emu->joystickButtonDown(emu->getJoyMapping(4)) != 0;
    bool confirm = emu->joystickButtonDown(emu->getJoyMapping(0)) != 0;
    // Reset = DS X (index 10), Clear = DS Y (index 11), so each matches its "X"/"Y" action-bar hint.
    bool resetBtn = emu->joystickButtonDown(emu->getJoyMapping(10)) != 0;
    bool clearBtn = emu->joystickButtonDown(emu->getJoyMapping(11)) != 0;

    int dir = -1;
    if      (up)      dir = 0;
    else if (down)    dir = 1;
    else if (left)    dir = 2;
    else if (right)   dir = 3;
    else if (confirm) dir = 4;
    else if (backNow) dir = 5;
    else if (resetBtn && (currentScreen == Screen::Remap || currentScreen == Screen::Detail)) dir = 6;
    else if (clearBtn && currentScreen == Screen::Remap) dir = 7;

    if (dir < 0) return;

    if (dir == 5 && !m_resettingBindings && currentScreen == Screen::Sidebar)
    {
        m_lastJoyInput = now;
        m_pendingClose = true;
        playSound(3);
        return;
    }

    m_lastJoyInput = now;
    handleNavigation(dir);
}

void SettingsView::playSound(int kind)
{
    m_mainWindow->playCutsceneMenuSound(kind);
}

// ─── navigation helpers ───────────────────────────────────────────────────────

void SettingsView::scrollDetailToRow(int idx, const QVector<SettingRow>& rows)
{
    int dX, dW, sY, bH, sp;
    computeDetailGeometry(dX, dW, sY, bH, sp);
    const int h = height();
    const qreal bottomLineY = contentBottomLineY();
    int labelH   = qMax(12, (int)(bH * 0.68));
    int intraGap = qMax(2,  (int)(bH * 0.10));
    int stride   = labelH + intraGap + bH + sp;
    int footerH  = (int)(h * 0.08) + (int)(h * 0.01);
    int visibleH = (int)(bottomLineY) - sY - footerH;
    int rowTop   = idx * stride;
    int rowBot   = rowTop + stride;
    if (rowTop < m_detailScrollOffset)
        m_detailScrollOffset = rowTop;
    else if (rowBot > m_detailScrollOffset + visibleH)
        m_detailScrollOffset = rowBot - visibleH;
    m_detailScrollOffset = qMax(0, m_detailScrollOffset);
    int maxOff = qMax(0, (int)rows.size() * stride - visibleH);
    m_detailScrollOffset = qMin(m_detailScrollOffset, maxOff);
    if (idx == 0)                    m_detailScrollOffset = 0;
    if (idx == (int)rows.size() - 1) m_detailScrollOffset = maxOff;
}

void SettingsView::scrollRemapToAction(int actionIdx)
{
    if (actionIdx < 0 || actionIdx >= m_remapActionToItemIdx.size()) return;
    int itemIdx = m_remapActionToItemIdx[actionIdx];
    int dX, dW, sY, bH, sp;
    computeDetailGeometry(dX, dW, sY, bH, sp);
    int stride   = bH + sp;
    const qreal bottomLineY = contentBottomLineY();
    int visibleH = (int)(bottomLineY) - sY - bH * 3 - sp;
    int itemY = itemIdx * stride;
    if (itemY < m_remapScrollOffset)
        m_remapScrollOffset = itemY;
    else if (itemY + stride > m_remapScrollOffset + visibleH)
        m_remapScrollOffset = itemY + stride - visibleH;
    m_remapScrollOffset = qMax(0, m_remapScrollOffset);
    int maxOff = qMax(0, (int)m_remapItems.size() * stride - visibleH);
    m_remapScrollOffset = qMin(m_remapScrollOffset, maxOff);
    int actionCount = (int)m_remapActionToItemIdx.size();
    if (actionIdx == 0)              m_remapScrollOffset = 0;
    if (actionIdx == actionCount - 1) m_remapScrollOffset = maxOff;
}

// ─── per-screen navigation ────────────────────────────────────────────────────

void SettingsView::handleNavSidebar(int direction)
{
    switch (direction)
    {
    case 0:
        sidebarIndex = (sidebarIndex - 1 + kSidebarCount) % kSidebarCount;
        detailIndex = 0;
        m_detailScrollOffset = 0;
        playSound(2);
        update();
        break;
    case 1:
        sidebarIndex = (sidebarIndex + 1) % kSidebarCount;
        detailIndex = 0;
        m_detailScrollOffset = 0;
        playSound(2);
        update();
        break;
    case 3:
    case 4:
        if (sidebarIndex == kIdxQuit)
        {
            currentScreen = Screen::Detail;
            playSound(1);
            update();
        }
        else if (sidebarIndex == kIdxGamepad || sidebarIndex == kIdxKeyboard)
        {
            QVector<SettingRow> kbRows = rowsFor(sidebarIndex);
            if (!kbRows.isEmpty())
            {
                currentScreen        = Screen::Detail;
                detailIndex          = 0;
                m_detailScrollOffset = 0;
                playSound(1);
                update();
            }
            else
            {
                m_remapIsJoystick   = (sidebarIndex == kIdxGamepad);
                m_remapScrollOffset = 0;
                detailIndex         = 0;
                buildRemapList();
                currentScreen = Screen::Remap;
                playSound(1);
                update();
            }
        }
        else if (sidebarIndex == kIdxStream)
        {
            currentScreen = Screen::Detail;
            playSound(1);
            update();
        }
        else
        {
            QVector<SettingRow> rows = rowsFor(sidebarIndex);
            if (rows.isEmpty()) break;
            currentScreen  = Screen::Detail;
            detailIndex    = 0;
            m_detailScrollOffset = 0;
            playSound(1);
            update();
        }
        break;
    case 5:
        playSound(3);
        emit settingsClosed();
        break;
    default: break;
    }
}

void SettingsView::handleNavDetail(int direction)
{
    if (sidebarIndex == kIdxStream)
    {
        if (direction == 5 || direction == 2)
        { currentScreen = Screen::Sidebar; playSound(3); update(); }
        else if (direction == 4)
            QDesktopServices::openUrl(QUrl("https://www.kingdomhearts.com/1525/us/"));
        return;
    }

    if (sidebarIndex == kIdxQuit)
    {
        if (direction == 5 || direction == 2)
        { currentScreen = Screen::Sidebar; playSound(3); update(); }
        else if (direction == 3 || direction == 4)
        { playSound(4); emit quitGameConfirmed(); }
        return;
    }

    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    int rowCount = rows.size();
    if (rowCount == 0)
    {
        if (direction == 5 || direction == 2)
        { currentScreen = Screen::Sidebar; update(); }
        return;
    }

    // Guard against a stale detailIndex: rowsFor() can shrink while the overlay is open
    // (e.g. the game is stopped), leaving detailIndex past the new end.
    if (detailIndex >= rowCount) detailIndex = rowCount - 1;

    const SettingRow& row = rows[detailIndex];

    auto isRowBlocked = [&]() -> bool {
        const SettingRow& r = rows[detailIndex];
        return r.enabled && !r.enabled();
    };

    auto openCurrentRow = [&]() {
        if (rows[detailIndex].isDynamic)
        {
            if (sidebarIndex == kIdxSystem) populateWifiAdapters();
            if (sidebarIndex == kIdxSound)  populateAudioPacks();
        }
        m_optionScrollOffset = 0;
        optionIndex = readRowValue(sidebarIndex, detailIndex);
        scrollOptionToIdx(optionIndex);
        currentScreen = Screen::OptionList;
        playSound(1);
        update();
    };

    auto openGamepadRemap = [&]() {
        m_remapIsJoystick   = true;
        m_remapScrollOffset = 0;
        detailIndex         = 0;
        buildRemapList();
        currentScreen = Screen::Remap;
        playSound(1);
        update();
    };

    auto openKeyboardRemap = [&]() {
        m_remapIsJoystick   = false;
        m_remapScrollOffset = 0;
        detailIndex         = 0;
        buildRemapList();
        currentScreen = Screen::Remap;
        playSound(1);
        update();
    };

    switch (direction)
    {
    case 0:
        detailIndex = (detailIndex - 1 + rowCount) % rowCount;
        scrollDetailToRow(detailIndex, rows);
        playSound(2);
        update();
        break;
    case 1:
        detailIndex = (detailIndex + 1) % rowCount;
        scrollDetailToRow(detailIndex, rows);
        playSound(2);
        update();
        break;
    case 2:
        if (row.type == SettingRow::Type::Slider)
        {
            int cur = readRowValue(sidebarIndex, detailIndex);
            int nv  = qMax(row.sliderMin, cur - 1);
            if (nv != cur) { writeRowValue(sidebarIndex, detailIndex, nv); update(); }
        }
        else
        { currentScreen = Screen::Sidebar; playSound(3); update(); }
        break;
    case 3:
    {
        if (isRowBlocked()) break;
        if (row.type == SettingRow::Type::Slider)
        {
            int cur = readRowValue(sidebarIndex, detailIndex);
            int nv  = qMin(row.sliderMax, cur + 1);
            if (nv != cur) { writeRowValue(sidebarIndex, detailIndex, nv); update(); }
        }
        else if (row.type == SettingRow::Type::Combobox)
            openCurrentRow();
        else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxGamepad)
            openGamepadRemap();
        else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxKeyboard)
            openKeyboardRemap();
        break;
    }
    case 4:
    {
        if (isRowBlocked()) break;
        if (row.type == SettingRow::Type::Toggle)
        {
            int cur = readRowValue(sidebarIndex, detailIndex);
            writeRowValue(sidebarIndex, detailIndex, cur ? 0 : 1);
            playSound(4);
            update();
        }
        else if (row.type == SettingRow::Type::Combobox)
            openCurrentRow();
        else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxGamepad)
            openGamepadRemap();
        else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxKeyboard)
            openKeyboardRemap();
        break;
    }
    case 5:
        currentScreen = Screen::Sidebar;
        playSound(3);
        update();
        break;
    case 6:
        if (sidebarIndex != kIdxStream && sidebarIndex != kIdxQuit &&
            sidebarIndex != kIdxGamepad && sidebarIndex != kIdxKeyboard)
        {
            m_resettingSection      = true;
            m_resettingConfirmIndex = 1;
            playSound(1);
            update();
        }
        break;
    default: break;
    }
}

void SettingsView::handleNavOptionList(int direction)
{
    QStringList opts = currentOptionList();
    int optCount = opts.size();
    if (optCount == 0) { currentScreen = Screen::Detail; update(); return; }

    switch (direction)
    {
    case 0:
        optionIndex = (optionIndex - 1 + optCount) % optCount;
        scrollOptionToIdx(optionIndex);
        playSound(2);
        update();
        break;
    case 1:
        optionIndex = (optionIndex + 1) % optCount;
        scrollOptionToIdx(optionIndex);
        playSound(2);
        update();
        break;
    case 4:
        writeRowValue(sidebarIndex, detailIndex, optionIndex);
        currentScreen = Screen::Detail;
        playSound(4);
        update();
        break;
    case 5:
    case 2:
        optionIndex = readRowValue(sidebarIndex, detailIndex);
        currentScreen = Screen::Detail;
        playSound(3);
        update();
        break;
    default: break;
    }
}

void SettingsView::handleNavRemap(int direction)
{
    int actionCount = m_remapActionToItemIdx.size();
    if (actionCount == 0)
    {
        if (direction == 5) { currentScreen = Screen::Sidebar; update(); }
        return;
    }

    switch (direction)
    {
    case 0:
        if (detailIndex == 0)
        {
            int dX_, dW_, sY_, bH_, sp_;
            computeDetailGeometry(dX_, dW_, sY_, bH_, sp_);
            const qreal botY_ = contentBottomLineY();
            int visH_ = (int)(botY_) - sY_ - bH_ * 3 - sp_;
            m_remapScrollOffset = qMax(0, (int)m_remapItems.size() * (bH_ + sp_) - visH_);
            detailIndex = actionCount - 1;
        }
        else
        {
            detailIndex -= 1;
            scrollRemapToAction(detailIndex);
        }
        playSound(2);
        update();
        break;
    case 1:
        if (detailIndex == actionCount - 1)
        {
            m_remapScrollOffset = 0;
            detailIndex = 0;
        }
        else
        {
            detailIndex += 1;
            scrollRemapToAction(detailIndex);
        }
        playSound(2);
        update();
        break;
    case 4:
    {
        if (m_remapIsJoystick)
        {
            EmuInstance* emu = m_mainWindow->getEmuInstance();
            SDL_Joystick* joy = emu ? emu->getJoystick() : nullptr;
            if (joy)
            {
                int numAxes = qMin(SDL_JoystickNumAxes(joy), 16);
                for (int i = 0; i < numAxes; i++)
                    m_axesRest[i] = SDL_JoystickGetAxis(joy, i);
            }
        }
        m_bindTarget      = detailIndex;
        m_waitingForBind  = true;
        m_bindArmed       = false;
        m_bindListenStart = m_animClock.elapsed();
        playSound(1);
        update();
        break;
    }
    case 5:
        if (sidebarIndex == kIdxKeyboard && !rowsFor(kIdxKeyboard).isEmpty())
        {
            currentScreen        = Screen::Detail;
            detailIndex          = 0;
            m_detailScrollOffset = 0;
        }
        else
        {
            currentScreen = Screen::Sidebar;
        }
        playSound(3);
        update();
        break;
    case 6:
        m_resettingBindings     = true;
        m_resettingConfirmIndex = 1;
        playSound(1);
        update();
        break;
    case 7: // clear the selected binding (unbound = -1 for joystick, 0 for keyboard)
        writeRemapBinding(detailIndex, m_remapIsJoystick ? -1 : 0);
        playSound(3);
        update();
        break;
    default: break;
    }
}

// ─── main navigation dispatcher ───────────────────────────────────────────────

void SettingsView::handleNavigation(int direction)
{
    if (m_resettingSection)
    {
        switch (direction)
        {
        case 2: m_resettingConfirmIndex = 0; update(); break;
        case 3: m_resettingConfirmIndex = 1; update(); break;
        case 4:
            if (m_resettingConfirmIndex == 0)
            {
                resetSectionToDefaults(sidebarIndex);
                Config::Save();
                playSound(4);
            }
            else playSound(3);
            m_resettingSection = false;
            update();
            break;
        case 5: m_resettingSection = false; playSound(3); update(); break;
        default: break;
        }
        return;
    }

    if (m_resettingBindings)
    {
        switch (direction)
        {
        case 2: m_resettingConfirmIndex = 0; update(); break;
        case 3: m_resettingConfirmIndex = 1; update(); break;
        case 4:
            if (m_resettingConfirmIndex == 0) { resetAllRemapBindings(); playSound(4); }
            else                               playSound(3);
            m_resettingBindings = false;
            update();
            break;
        case 5: m_resettingBindings = false; playSound(3); update(); break;
        default: break;
        }
        return;
    }

    if (currentScreen == Screen::Sidebar)    { handleNavSidebar(direction);    return; }
    if (currentScreen == Screen::Detail)     { handleNavDetail(direction);     return; }
    if (currentScreen == Screen::OptionList) { handleNavOptionList(direction); return; }
    if (currentScreen == Screen::Remap)      { handleNavRemap(direction); }
}


// ─── key events ───────────────────────────────────────────────────────────────

void SettingsView::keyPressEvent(QKeyEvent* event)
{
    if (m_waitingForBind)
    {
        if (!m_remapIsJoystick)
        {
            if (event->key() == Qt::Key_Backspace)
            {
                writeRemapBinding(m_bindTarget, 0);
            }
            else if (event->key() != Qt::Key_Escape)
            {
                int key = event->key();
                int mod = event->modifiers();
                bool ismod = (key == Qt::Key_Control || key == Qt::Key_Alt ||
                              key == Qt::Key_AltGr   || key == Qt::Key_Shift ||
                              key == Qt::Key_Meta);
                if (!ismod) key |= mod;
                else if (isRightModKey(event)) key |= (1 << 31);
                writeRemapBinding(m_bindTarget, key);
            }
            m_waitingForBind = false;
            update();
        }
        // While waiting for any bind (joystick or keyboard), eat all key events.
        event->accept();
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Up:     handleNavigation(0); event->accept(); break;
    case Qt::Key_Down:   handleNavigation(1); event->accept(); break;
    case Qt::Key_Left:   handleNavigation(2); event->accept(); break;
    case Qt::Key_Right:  handleNavigation(3); event->accept(); break;
    case Qt::Key_Return:
    case Qt::Key_Space:  handleNavigation(4); event->accept(); break;
    case Qt::Key_Escape: handleNavigation(5); event->accept(); break;
    case Qt::Key_X:
        if (currentScreen == Screen::Remap || currentScreen == Screen::Detail)
            handleNavigation(6);
        event->accept();
        break;
    case Qt::Key_Delete:
        if (currentScreen == Screen::Remap)
            handleNavigation(7);
        event->accept();
        break;
    default: event->accept(); break;
    }
}

// ─── mouse events ─────────────────────────────────────────────────────────────

void SettingsView::confirmPopupRects(QRect& box, QRect& yesRect, QRect& noRect) const
{
    const int w = width(), h = height();
    const int boxW  = (int)(w * 0.50);
    const int boxH  = (int)(h * 0.38);
    box = QRect((w - boxW) / 2, (h - boxH) / 2, boxW, boxH);
    const int btnH   = (int)qMax(24.0, boxH * 0.20);
    const int btnW   = (int)(boxW * 0.32);
    const int btnY   = box.y() + (int)(boxH * 0.68);
    const int btnGap = (int)(boxW * 0.06);
    const int startX = box.x() + (boxW - btnW * 2 - btnGap) / 2;
    yesRect = QRect(startX, btnY, btnW, btnH);
    noRect  = QRect(startX + btnW + btnGap, btnY, btnW, btnH);
}

void SettingsView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();

    if (m_resettingSection || m_resettingBindings)
    {
        QRect box, yesRect, noRect;
        confirmPopupRects(box, yesRect, noRect);
        int hit = yesRect.contains(pos) ? 0 : noRect.contains(pos) ? 1 : -1;
        if (hit >= 0 && hit != m_resettingConfirmIndex) { m_resettingConfirmIndex = hit; update(); }
        return;
    }

    if (m_draggingSlider)
    {
        float frac  = qBound(0.0f, (float)(pos.x() - m_dragTrackX) / m_dragTrackW, 1.0f);
        int   steps = m_dragSliderMax - m_dragSliderMin + 1;
        int   nv    = m_dragSliderMin + qMin((int)(frac * steps), steps - 1);
        int   cur   = readRowValue(m_dragSidebarIdx, m_dragRowIdx);
        if (nv != cur) { writeRowValue(m_dragSidebarIdx, m_dragRowIdx, nv); update(); }
        return;
    }

    if (currentScreen == Screen::Sidebar)
    {
        int hit = hitTestSidebar(pos);
        if (hit >= 0 && hit != sidebarIndex)
        {
            sidebarIndex = hit;
            detailIndex  = 0;
            m_detailScrollOffset = 0;
            update();
        }
        return;
    }
    if (currentScreen == Screen::Detail)
    {
        int hit = hitTestDetail(pos);
        if (hit >= 0 && hit != detailIndex)
        {
            detailIndex = hit;
            update();
        }
        return;
    }
    if (currentScreen == Screen::OptionList)
    {
        int hit = hitTestOptionList(pos);
        if (hit >= 0 && hit != optionIndex)
        {
            optionIndex = hit;
            update();
        }
        return;
    }
    if (currentScreen == Screen::Remap)
    {
        int hit = hitTestRemap(pos);
        if (hit >= 0 && hit != detailIndex)
        {
            detailIndex = hit;
            update();
        }
    }
}

void SettingsView::wheelEvent(QWheelEvent* event)
{
    int dX_, dW_, sY_, bH_, sp_;
    computeDetailGeometry(dX_, dW_, sY_, bH_, sp_);
    const int h_ = height();
    const qreal bottomLineY_ = contentBottomLineY();
    int delta = -event->angleDelta().y() / 4;

    if (currentScreen == Screen::Detail)
    {
        QVector<SettingRow> rows = rowsFor(sidebarIndex);
        int labelH_   = qMax(12, (int)(bH_ * 0.68));
        int intraGap_ = qMax(2,  (int)(bH_ * 0.10));
        int stride_   = labelH_ + intraGap_ + bH_ + sp_;
        int footerTop_ = (int)(bottomLineY_) - (int)(h_ * 0.09);
        int visH_  = footerTop_ - sY_;
        int maxOff = qMax(0, rows.size() * stride_ - visH_);
        if (maxOff == 0) { event->ignore(); return; }
        m_detailScrollOffset = qBound(0, m_detailScrollOffset + delta, maxOff);
    }
    else if (currentScreen == Screen::Remap)
    {
        int visH_  = (int)(bottomLineY_) - sY_ - bH_ * 3 - sp_;
        int maxOff = qMax(0, (int)m_remapItems.size() * (bH_ + sp_) - visH_);
        if (maxOff == 0) { event->ignore(); return; }
        m_remapScrollOffset = qBound(0, m_remapScrollOffset + delta, maxOff);
    }
    else if (currentScreen == Screen::OptionList)
    {
        int labelH_    = qMax(12, (int)(bH_ * 0.68));
        int intraGap_  = qMax(2,  (int)(bH_ * 0.10));
        int optStartY_ = sY_ + labelH_ + intraGap_;
        int visH_  = (int)(bottomLineY_) - optStartY_ - (int)(h_ * 0.04);
        QStringList opts = currentOptionList();
        int maxOff = qMax(0, opts.size() * (bH_ + sp_) - visH_);
        if (maxOff == 0) { event->ignore(); return; }
        m_optionScrollOffset = qBound(0, m_optionScrollOffset + delta, maxOff);
    }
    else { event->ignore(); return; }
    update();
    event->accept();
}

void SettingsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    QPoint pos = event->pos();

    if (m_resettingBindings || m_resettingSection)
    {
        QRect box, yesRect, noRect;
        confirmPopupRects(box, yesRect, noRect);
        if (yesRect.contains(pos))
        {
            m_resettingConfirmIndex = 0;
            handleNavigation(4);
        }
        else if (noRect.contains(pos))
        {
            m_resettingConfirmIndex = 1;
            handleNavigation(4);
        }
        return;
    }

    // Clicking in sidebar area from Detail/OptionList/Remap exits back to sidebar
    if (currentScreen != Screen::Sidebar && !m_waitingForBind)
    {
        int hit = hitTestSidebar(pos);
        if (hit >= 0)
        {
            sidebarIndex = hit;
            detailIndex  = 0;
            m_detailScrollOffset = 0;
            currentScreen = Screen::Sidebar;
            playSound(3);
            update();
            return;
        }
    }

    if (currentScreen == Screen::Sidebar)
    {
        int hit = hitTestSidebar(pos);
        if (hit >= 0)
        {
            sidebarIndex      = hit;
            detailIndex       = 0;
            m_detailScrollOffset = 0;
            handleNavigation(4);
        }
        return;
    }

    if (currentScreen == Screen::Remap)
    {
        int hit = hitTestRemap(pos);
        if (hit >= 0)
        {
            detailIndex = hit;
            handleNavigation(4);
        }
        return;
    }

    if (currentScreen == Screen::Detail)
    {
        if (sidebarIndex == kIdxStream) { handleNavigation(4); return; }
        int hit = hitTestDetail(pos);
        if (hit >= 0)
        {
            detailIndex = hit;
            QVector<SettingRow> rows = rowsFor(sidebarIndex);
            const SettingRow& row = rows[hit];
            if (row.type == SettingRow::Type::Slider)
            {
                int dX, dW, sY, bH, sp;
                computeDetailGeometry(dX, dW, sY, bH, sp);
                int rowPillW_ = (int)(dW * 0.56) - (int)(bH * 1.7);
                int trackX, trackW;
                sliderTrackGeometry(dX, rowPillW_, bH, trackX, trackW);
                float frac   = qBound(0.0f, (float)(pos.x() - trackX) / trackW, 1.0f);
                int steps    = row.sliderMax - row.sliderMin + 1;
                int nv       = row.sliderMin + qMin((int)(frac * steps), steps - 1);
                writeRowValue(sidebarIndex, hit, nv);
                m_draggingSlider = true;
                m_dragSidebarIdx = sidebarIndex;
                m_dragRowIdx     = hit;
                m_dragTrackX     = trackX;
                m_dragTrackW     = trackW;
                m_dragSliderMin  = row.sliderMin;
                m_dragSliderMax  = row.sliderMax;
                update();
            }
            else
            {
                handleNavigation(4);
            }
        }
        return;
    }

    if (currentScreen == Screen::OptionList)
    {
        int hit = hitTestOptionList(pos);
        if (hit >= 0)
        {
            optionIndex = hit;
            handleNavigation(4);
        }
    }
}

void SettingsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        m_draggingSlider = false;
}

// ─── painting ─────────────────────────────────────────────────────────────────

QColor SettingsView::paintPillFill(QPainter& p, const QPainterPath& path,
                                    QRect r, PillState state)
{
    QColor themeColor = currentThemeColor();
    const qreal radius = r.height() / 2.0;
    QColor textColor;

    if (state == PillState::Selected)
    {
        const int   glowSteps  = 8;
        const qreal glowSpread = 6.0;
        for (int step = 1; step <= glowSteps; step++)
        {
            qreal ex = (qreal)(glowSteps - step + 1) / glowSteps * glowSpread;
            QPainterPath gp;
            gp.addRoundedRect(QRectF(r).adjusted(-ex, -ex * 0.6, ex, ex * 0.6),
                               radius + ex, radius + ex);
            QColor gc = themeColor.lighter(130);
            gc.setAlpha((int)(step * (45.0 / glowSteps)));
            p.fillPath(gp, gc);
        }
        QColor baseFill = themeColor.darker(600);
        baseFill.setAlpha(245);
        p.fillPath(path, baseFill);

        const int   innerSteps = 16;
        const qreal maxInnerW  = r.height() * 0.18;
        p.save();
        p.setClipPath(path);
        for (int step = 1; step <= innerSteps; step++)
        {
            qreal sw = (qreal)(innerSteps - step + 1) / innerSteps * maxInnerW * 2;
            QColor ig = themeColor.lighter(150);
            ig.setAlpha((int)(step * 50.0 / innerSteps));
            p.strokePath(path, QPen(ig, sw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
        p.restore();

        QColor borderColor = themeColor.lighter(140);
        borderColor.setAlpha(100);
        p.strokePath(path, QPen(borderColor, 1.5));
        textColor = QColor(235, 235, 240);
    }
    else
    {
        QLinearGradient grad(r.topLeft(), r.bottomLeft());
        grad.setColorAt(0.0, QColor(10, 10, 10, 230));
        grad.setColorAt(1.0, QColor(7,  7,  7,  230));
        p.fillPath(path, grad);
        QColor borderColor = themeColor;
        borderColor.setAlpha(80);
        p.strokePath(path, QPen(borderColor, 1.5));
        textColor = (state == PillState::Dimmed) ? QColor(88, 88, 92) : QColor(180, 180, 185);
    }

    p.setBrush(Qt::NoBrush);
    return textColor;
}

void SettingsView::paintPillButton(QPainter& p, QRect r, const QString& label,
                                    PillState state, QColor swatchColor)
{
    const qreal radius = r.height() / 2.0;
    QPainterPath path;
    addKHPillPath(path, QRectF(r), radius);

    QColor textColor = paintPillFill(p, path, r, state);

    const int padding = (int)(r.height() * 0.5);
    p.setPen(textColor);

    if (swatchColor.isValid())
    {
        int swatchSize = (int)(r.height() * 0.45);
        int swatchX = r.left() + padding;
        int swatchY = r.top() + (r.height() - swatchSize) / 2;
        p.setBrush(swatchColor);
        p.setPen(Qt::NoPen);
        p.drawEllipse(swatchX, swatchY, swatchSize, swatchSize);
        p.setBrush(Qt::NoBrush);
        p.setPen(textColor);
        QRect textRect(swatchX + swatchSize + (int)(r.height() * 0.2),
                       r.top(), r.width() - padding - swatchSize, r.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    }
    else
    {
        QRect textRect(r.left() + padding, r.top(), r.width() - padding, r.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    }
}

void SettingsView::paintOrbAt(QPainter& p, qreal capCx, qreal capCy, qreal capR)
{
    static const QPixmap lightPixmap(":/ds/menu_light.png");
    if (lightPixmap.isNull()) return;
    const qreal t    = m_animClock.elapsed() / 1000.0;
    const qreal boxCx = capCx + 0.19 * capR;
    const qreal boxCy = capCy - 0.56 * capR;
    const qreal ampX  = 0.55 * capR;
    const qreal ampY  = 0.50 * capR;
    const qreal speed = 0.5;
    const qreal gx = boxCx + ampX * std::cos(2.0 * M_PI * (1.012 * speed) * t - 2.39);
    const qreal gy = boxCy + ampY * std::cos(2.0 * M_PI * (1.265 * speed) * t - 1.57);
    int ls = (int)(capR);
    p.drawPixmap(QRectF(gx - ls / 2.0, gy - ls / 2.0, ls, ls),
                 lightPixmap, lightPixmap.rect());
}

void SettingsView::paintScrollbar(QPainter& p, int x, int y, int w, int h,
                                   int offset, int totalH, int visibleH)
{
    if (totalH <= visibleH) return;
    QColor tc = currentThemeColor();

    QPainterPath track;
    track.addRoundedRect(QRectF(x, y, w, h), 2, 2);
    QColor trackCol = tc; trackCol.setAlpha(120);
    p.strokePath(track, QPen(trackCol, 1));

    int maxOffset = totalH - visibleH;
    int thumbH = qMax(12, h * visibleH / totalH);
    int thumbY = y + (maxOffset > 0 ? (h - thumbH) * offset / maxOffset : 0);
    QPainterPath thumb;
    thumb.addRoundedRect(QRectF(x, thumbY, w, thumbH), 2, 2);
    QColor thumbCol = tc; thumbCol.setAlpha(200);
    p.fillPath(thumb, thumbCol);

    int cx = x + w / 2;
    int triR = 2;
    QColor triCol = tc; triCol.setAlpha(160);

    // Up arrow
    QPainterPath up;
    up.moveTo(cx, y - triR * 2 - 1 - triR);
    up.lineTo(cx + triR, y - triR * 2 - 1 + triR);
    up.lineTo(cx - triR, y - triR * 2 - 1 + triR);
    up.closeSubpath();
    p.fillPath(up, triCol);
    p.strokePath(up, QPen(triCol, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Down arrow
    QPainterPath dn;
    dn.moveTo(cx, y + h + triR * 2 + 1 + triR);
    dn.lineTo(cx + triR, y + h + triR * 2 + 1 - triR);
    dn.lineTo(cx - triR, y + h + triR * 2 + 1 - triR);
    dn.closeSubpath();
    p.fillPath(dn, triCol);
    p.strokePath(dn, QPen(triCol, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void SettingsView::paintBackground(QPainter& p)
{
    const int w = width();
    const int h = height();

    p.fillRect(0, 0, w, h, QColor(10, 10, 10));

    if (!m_background.isNull())
    {
        const int tileW = 1032;
        const int tileH = 518;
        const int offX  = -(tileW / 3) + (int)(80.0 * tileW / 1548);
        const int offY  = -(tileH / 4) - (int)(20.0 * tileH / 776);
        p.setOpacity(0.88);
        for (int ty = offY; ty < h; ty += tileH)
            for (int tx = offX; tx < w; tx += tileW)
                p.drawPixmap(tx, ty, tileW, tileH, m_background);
        p.setOpacity(1.0);
    }

    QColor tc = currentThemeColor();
    p.fillRect(0, 0, w, h, QColor(tc.red(), tc.green(), tc.blue(), 25));

    const qreal kSlope  = 0.60;
    const qreal diagDy  = -(qreal)h * kSlope;
    const qreal diagLen = std::sqrt((qreal)w * w + diagDy * diagDy);
    const qreal ux      = -diagDy / diagLen;
    const qreal uy      = (qreal)w  / diagLen;
    const qreal cover   = std::sqrt(((qreal)w / 2) * ((qreal)w / 2) +
                                    ((qreal)h / 2) * ((qreal)h / 2));
    QLinearGradient band(
        QPointF(w / 2.0 - ux * cover, h / 2.0 - uy * cover),
        QPointF(w / 2.0 + ux * cover, h / 2.0 + uy * cover)
    );
    band.setColorAt(0.00, QColor(0, 0, 0,   0));
    band.setColorAt(0.10, QColor(0, 0, 0,   0));
    band.setColorAt(0.30, QColor(0, 0, 0, 180));
    band.setColorAt(0.50, QColor(0, 0, 0, 230));
    band.setColorAt(0.70, QColor(0, 0, 0, 180));
    band.setColorAt(0.90, QColor(0, 0, 0,   0));
    band.setColorAt(1.00, QColor(0, 0, 0,   0));
    p.fillRect(0, 0, w, h, band);
}

void SettingsView::paintTitleBar(QPainter& p)
{
    const int w = width();
    int barY, barH;
    computeTitleBarGeometry(barY, barH);
    const int barW = (int)(w * 0.40);

    QColor themeColor = currentThemeColor();

    const qreal capR = barH / 2.0;
    QPainterPath path;
    path.moveTo(0, barY);
    path.lineTo(barW - capR, barY);
    path.arcTo(barW - barH, barY, barH, barH, 90, -180);
    path.lineTo(0, barY + barH);
    path.closeSubpath();
    p.fillPath(path, themeColor);

    const int iconSize = qMax(16, (int)(barH * 0.88));
    if (!m_gearPixmap.isNull())
    {
        const int gearX = (int)(barH * 0.28);
        const int gearY = barY + (barH - iconSize) / 2;
        p.drawPixmap(gearX, gearY, iconSize, iconSize, m_gearPixmap);
    }

    QFont font("KHGummi");
    font.setPixelSize(qMax(11, (int)(barH * 0.50)));
    p.setFont(font);
    p.setPen(QColor(0, 0, 0));
    const qreal iconW = iconSize;
    const qreal textX = (int)(barH * 0.28) + iconW + (int)(barH * 0.12);
    p.drawText(QRectF(textX, barY, barW - textX, barH),
               Qt::AlignVCenter | Qt::AlignLeft, locale().titleGameSettings);

    const qreal lineY = barY + barH / 2.0;
    QColor lineColor = themeColor;
    lineColor.setAlpha(200);
    p.setPen(QPen(lineColor, 1));
    p.drawLine(QPointF(barW, lineY), QPointF(w, lineY));
}

void SettingsView::paintBottomLine(QPainter& p)
{
    const int w = width();
    const qreal bottomLineY = contentBottomLineY();
    QColor lineColor = currentThemeColor();
    lineColor.setAlpha(200);
    p.setPen(QPen(lineColor, 1));
    p.drawLine(QPointF(0, bottomLineY), QPointF(w, bottomLineY));
}

void SettingsView::paintSidebar(QPainter& p)
{
    const int w = width();
    const int h = height();

    int sidebarX, sidebarW, startY, btnH, spacing, divX;
    computeSidebarGeometry(sidebarX, sidebarW, startY, btnH, spacing, divX);

    int barY_d, barH_d;
    computeTitleBarGeometry(barY_d, barH_d);
    const int   circleR  = 3;
    const int   divGap   = (int)(h * 0.042);
    const int   divStart = barY_d + barH_d + divGap;
    const qreal topLineY = barY_d + barH_d / 2.0;
    const int   divEnd   = (int)(h - topLineY) - divGap;

    QFont font("KHMenu");
    font.setPixelSize(qMax(11, (int)(btnH * 0.50)));
    p.setFont(font);

    const int activeIndent = (int)(w * 0.025);
    bool dimUnselected = (currentScreen != Screen::Sidebar);

    for (int i = 0; i < kSidebarCount; i++)
    {
        int y = startY + i * (btnH + spacing);
        bool selected = (i == sidebarIndex);
        int indent = selected ? activeIndent : 0;
        int pillLeft  = sidebarX + indent;
        int pillRight = sidebarX + sidebarW + (selected ? activeIndent : 0);
        QRect btnRect(pillLeft, y, pillRight - pillLeft, btnH);
        PillState state = selected ? PillState::Selected
                        : dimUnselected ? PillState::Dimmed : PillState::Normal;
        paintPillButton(p, btnRect, QString::fromUtf8(locale().sidebarLabels[i]), state);

        if (selected && currentScreen == Screen::Sidebar)
        {
            const qreal capR  = btnH / 2.0;
            const qreal capCx = btnRect.right() - capR;
            const qreal capCy = btnRect.center().y();
            paintOrbAt(p, capCx, capCy, capR);
        }
    }

    QColor divColor = currentThemeColor();
    divColor.setAlpha(200);
    p.setPen(QPen(divColor, 1));
    p.drawLine(divX, divStart + circleR, divX, divEnd - circleR);
    p.setBrush(divColor);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPoint(divX, divStart), circleR, circleR);
    p.drawEllipse(QPoint(divX, divEnd),   circleR, circleR);
    p.setBrush(Qt::NoBrush);
}

SettingsView::DetailLayout SettingsView::computeDetailLayout() const
{
    DetailLayout L;
    L.h = height();
    computeDetailGeometry(L.detailX, L.detailW, L.startY, L.btnH, L.spacing);
    L.themeColor = currentThemeColor();
    L.bottomLineY = contentBottomLineY();
    L.topLineY    = L.h - L.bottomLineY;
    L.rowFont = QFont("KHMenu");
    L.rowFont.setPixelSize(qMax(13, (int)(L.btnH * 0.53)));
    L.rowPillW  = (int)(L.detailW * 0.56) - (int)(L.btnH * 1.7);
    L.optGapW   = (int)(L.detailW * 0.06);
    L.optPanelX = L.detailX + L.rowPillW + L.optGapW;
    L.optPanelW = L.detailW - L.rowPillW - L.optGapW;
    L.scrollbarW = 4;
    return L;
}

void SettingsView::paintDetailArea(QPainter& p)
{
    const DetailLayout L = computeDetailLayout();

    if (currentScreen == Screen::Sidebar)          { paintDetailSidebarPreview(p, L); return; }
    if (currentScreen == Screen::Remap)            { paintDetailRemap(p, L); return; }
    if (sidebarIndex == kIdxStream)                { paintDetailStream(p, L); return; }
    if (sidebarIndex == kIdxQuit)                  { paintDetailQuit(p, L); return; }

    paintDetailRows(p, L);
}

void SettingsView::paintDetailSidebarPreview(QPainter& p, const DetailLayout& L)
{
    const int& h          = L.h;
    const int& detailX    = L.detailX;
    const int& detailW    = L.detailW;
    const int& startY     = L.startY;
    const int& btnH       = L.btnH;
    const int& spacing    = L.spacing;
    const qreal& bottomLineY = L.bottomLineY;
    const QColor& themeColor = L.themeColor;
    const int& rowPillW  = L.rowPillW;

    QVector<SettingRow> sRows = rowsFor(sidebarIndex);
        if (!sRows.isEmpty())
        {
            int lH  = qMax(12, (int)(btnH * 0.68));
            int iG  = qMax(2,  (int)(btnH * 0.10));
            int str = lH + iG + btnH + spacing;
            QFont lf("KHMenu"); lf.setPixelSize(qMax(11, (int)(btnH * 0.57)));
            QFont rf("KHMenu"); rf.setPixelSize(qMax(13, (int)(btnH * 0.53)));
            int footerTop = (int)(bottomLineY) - (int)(h * 0.09);

            for (int i = 0; i < sRows.size(); i++)
            {
                const SettingRow& row = sRows[i];
                int lY = startY + i * str;
                int pY = lY + lH + iG;
                if (pY + btnH > footerTop) break;
                if (pY < startY) continue;

                QColor dimLabel(themeColor.red() * 45/100,
                                themeColor.green() * 45/100,
                                themeColor.blue() * 45/100);
                p.setFont(lf);
                p.setPen(dimLabel);
                p.drawText(QRect(detailX, lY, rowPillW, lH), Qt::AlignVCenter | Qt::AlignLeft, row.label);

                QRect pr(detailX, pY, rowPillW, btnH);
                if (row.type == SettingRow::Type::Slider)
                {
                    QPainterPath sp_;
                    addRoundedPillPath(sp_, QRectF(pr));
                    paintPillFill(p, sp_, pr, PillState::Normal);

                    int v = row.read ? row.read() : 0;
                    p.setFont(rf);
                    p.setPen(QColor(80, 80, 85));
                    int numPad = (int)(btnH * 1.1);
                    p.drawText(QRect(detailX + numPad, pY, (int)(btnH * 1.5), btnH),
                               Qt::AlignVCenter | Qt::AlignLeft, QString::number(v));
                    int tX = detailX + numPad + (int)(btnH * 1.5) + (int)(btnH * 0.05);
                    int tW = qMax(1, detailX + rowPillW - (int)(btnH * 1.5) - tX);
                    int tH = qMax(3, (int)(btnH * 0.12));
                    int tMY = pY + btnH / 2;
                    float frac = (row.sliderMax > row.sliderMin)
                                 ? (float)(v - row.sliderMin) / (row.sliderMax - row.sliderMin) : 0.f;
                    int fW = (int)(frac * tW);
                    p.setPen(Qt::NoPen);
                    p.setBrush(QColor(40, 40, 40, 200));
                    p.drawRoundedRect(tX, tMY - tH/2, tW, tH, tH/2.0, tH/2.0);
                    if (fW > 0) { p.setBrush(themeColor.darker(160)); p.drawRect(tX, tMY - tH/2, fW, tH); }
                    int hW = qMax(4, (int)(btnH * 0.14)), hH = qMax(6, (int)(btnH * 0.52));
                    p.setBrush(QColor(160, 160, 165));
                    p.drawRect(tX + fW - hW/2, tMY - hH/2, hW, hH);
                    p.setBrush(Qt::NoBrush);
                }
                else
                {
                    QPainterPath pp_;
                    addRoundedPillPath(pp_, QRectF(pr));
                    QColor tc = paintPillFill(p, pp_, pr, PillState::Normal);
                    QString valStr;
                    if (row.type == SettingRow::Type::Navigate)
                    {
                        valStr = "\xe2\x86\x92";
                    }
                    else if (row.read)
                    {
                        int v = row.read();
                        if (row.type == SettingRow::Type::Toggle)
                            valStr = v ? locale().on : locale().off;
                        else if (row.type == SettingRow::Type::Combobox)
                            valStr = (!row.options.isEmpty() && v >= 0 && v < row.options.size())
                                     ? row.options[v] : rowDynamicValueText(sidebarIndex, i);
                    }
                    QColor dimTc(tc.red() * 45/100, tc.green() * 45/100, tc.blue() * 45/100);
                    p.setFont(rf);
                    p.setPen(dimTc);
                    int pad = (int)(btnH * 0.5);
                    p.drawText(QRect(detailX + pad, pY, rowPillW - pad*2, btnH),
                               Qt::AlignVCenter | Qt::AlignLeft, valStr);
                }
            }

            // Section overview at footer position (same slot as row description in Detail mode)
            int descY = (int)(bottomLineY) - (int)(h * 0.08);
            QFont descFont("KHMenu");
            descFont.setPixelSize(qMax(11, (int)(btnH * 0.43)));
            p.setFont(descFont);
            p.setPen(QColor(180, 180, 185));
            p.drawText(QRect(detailX, descY, detailW, (int)(h * 0.06)),
                       Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                       locale().sectionOverview[sidebarIndex]);
        }
        else
        {
            // Sections with no rows (Quit, Gamepad, etc.) — show centered overview
            QFont overviewFont("KHMenu");
            overviewFont.setPixelSize(qMax(12, (int)(btnH * 0.47)));
            p.setFont(overviewFont);
            p.setPen(QColor(180, 180, 185));
            p.drawText(QRect(detailX, startY, detailW, (int)(h * 0.30)),
                       Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                       locale().sectionOverview[sidebarIndex]);
        }
    }

void SettingsView::paintDetailRemap(QPainter& p, const DetailLayout& L)
{
    const int& h          = L.h;
    const int& detailX    = L.detailX;
    const int& detailW    = L.detailW;
    const int& startY     = L.startY;
    const int& btnH       = L.btnH;
    const int& spacing    = L.spacing;
    const qreal& bottomLineY = L.bottomLineY;
    const QFont& rowFont  = L.rowFont;
    const QColor& themeColor = L.themeColor;
    const int& scrollbarW = L.scrollbarW;
    const int& rowPillW   = L.rowPillW;

    int stride     = btnH + spacing;
    int visibleH   = (int)(bottomLineY) - startY - btnH * 3 - spacing;

    p.setFont(rowFont);

    SDL_GameController* controller;
    if (m_remapIsJoystick)
    {
        EmuInstance* emu = m_mainWindow->getEmuInstance();
        auto& lcfg = emu->getLocalConfig();
        int uid = lcfg.GetInt("JoystickUniqueID");
        controller = SDL_GameControllerOpen(emu->getJoystickIdByUniqueId(uid));
    }

    int actionIdx = 0;
    int focusedItemY = -1;
    for (int itemIdx = 0; itemIdx < m_remapItems.size(); itemIdx++)
    {
        const RemapItem& item = m_remapItems[itemIdx];
        int itemY = startY + itemIdx * stride - m_remapScrollOffset;

        if (itemY + stride < startY || itemY > startY + visibleH)
        {
            if (!item.isHeader) actionIdx++;
            continue;
        }

        if (item.isHeader)
        {
            QFont hdrFont("KHMenu");
            hdrFont.setPixelSize(qMax(11, (int)(btnH * 0.41)));
            p.setFont(hdrFont);
            p.setPen(themeColor);
            p.drawText(QRect(detailX, itemY, detailW, btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, item.label);
            p.setFont(rowFont);
        }
        else
        {
            bool focused = (actionIdx == detailIndex);
            if (focused) focusedItemY = itemY;
            QString bindStr;
            if (m_waitingForBind && focused)
                bindStr = "\xE2\x80\xA6";
            else
            {
                int v = readRemapBinding(actionIdx);
                bindStr = m_remapIsJoystick ? JoyMappingName(controller, v) : keyBindingText(v);
            }

            QPainterPath rp;
            addRoundedPillPath(rp, QRectF(detailX, itemY, rowPillW, btnH));
            QRect remapPillRect(detailX, itemY, rowPillW, btnH);
            paintPillFill(p, rp, remapPillRect, focused ? PillState::Selected : PillState::Normal);

            int pad = (int)(btnH * 0.45);
            p.setPen(focused ? QColor(235, 235, 240) : QColor(180, 180, 185));
            p.drawText(QRect(detailX + pad, itemY, (int)(rowPillW * 0.56), btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, item.label);
            p.setPen(focused ? QColor(200, 200, 210) : QColor(120, 120, 125));
            p.drawText(QRect(detailX + (int)(rowPillW * 0.56), itemY,
                             (int)(rowPillW * 0.40), btnH),
                       Qt::AlignVCenter | Qt::AlignRight, bindStr);

            actionIdx++;
        }
    }

    // Orb on focused remap pill
    if (focusedItemY >= 0)
    {
        qreal capR  = btnH / 2.0;
        qreal capCx = detailX + rowPillW - capR;
        qreal capCy = focusedItemY + btnH / 2.0;
        paintOrbAt(p, capCx, capCy, capR);
    }

    // Scrollbar just right of remap pills
    {
        int sbX    = detailX + rowPillW + (int)(detailW * 0.012);
        int totalH = m_remapItems.size() * (btnH + spacing);
        paintScrollbar(p, sbX, startY, scrollbarW, visibleH,
                       m_remapScrollOffset, totalH, visibleH);
    }

    // Instructional text above hint footer
    QFont instrFont("KHMenu");
    instrFont.setPixelSize(qMax(11, (int)(btnH * 0.45)));
    p.setFont(instrFont);
    p.setPen(QColor(180, 180, 185));
    p.drawText(QRect(detailX, (int)(bottomLineY - btnH * 2 - spacing), detailW, btnH),
               Qt::AlignVCenter | Qt::AlignLeft, QString::fromUtf8(locale().remapSelectPrompt));

    QFont hintFont("KHMenu");
    hintFont.setPixelSize(qMax(11, (int)(btnH * 0.40)));
    p.setFont(hintFont);
    p.setPen(QColor(120, 120, 125));
    p.drawText(QRect(detailX, (int)(bottomLineY - btnH - spacing), detailW, btnH),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString::fromUtf8(locale().remapResetBackHint));

    if (m_remapIsJoystick)
    {
        SDL_GameControllerClose(controller);
    }
}

void SettingsView::paintDetailStream(QPainter& p, const DetailLayout& L)
{
    const int& h          = L.h;
    const int& detailX    = L.detailX;
    const int& detailW    = L.detailW;
    const int& startY     = L.startY;
    const int& btnH       = L.btnH;
    const int& spacing    = L.spacing;
    const qreal& bottomLineY = L.bottomLineY;
    const QFont& rowFont  = L.rowFont;
    const QColor& themeColor = L.themeColor;

    const int margin     = (int)(detailW * 0.04);  // outer margin: title, lines
    const int pillMargin = (int)(detailW * 0.13);  // inner margin: body text, URL pill
    const QString title  = QString::fromUtf8(locale().sidebarLabels[kIdxStream]);

    QFont titleFont("KHGummi");
    titleFont.setPixelSize(qMax(14, (int)(btnH * 0.60)));
    p.setFont(titleFont);
    p.setPen(themeColor);
    p.drawText(QRect(detailX + margin, startY, detailW - margin * 2, btnH),
               Qt::AlignVCenter | Qt::AlignCenter, title);

    // Top gradient line — tied to title text width
    int line1Y = startY + btnH + spacing / 2;
    {
        int titleW = QFontMetrics(titleFont).horizontalAdvance(title);
        int pad    = (int)(detailW * 0.10);
        int tLeft  = detailX + (detailW - titleW) / 2 - pad;
        int tRight = tLeft + titleW + 2 * pad;
        QColor lc = themeColor; lc.setAlpha(180);
        QLinearGradient g(tLeft, 0, tRight, 0);
        g.setColorAt(0.0,  Qt::transparent);
        g.setColorAt(0.06, lc);
        g.setColorAt(0.94, lc);
        g.setColorAt(1.0,  Qt::transparent);
        p.setPen(QPen(QBrush(g), 1));
        p.drawLine(tLeft, line1Y, tRight, line1Y);
    }

    const QString para1 = QString::fromUtf8(locale().streamPara1);
    const QString para2 = QString::fromUtf8(locale().streamPara2);
    const QString para3 = QString::fromUtf8(locale().streamPara3);
    const QString para4 = QString::fromUtf8(locale().streamPara4);

    p.setFont(rowFont);
    p.setPen(QColor(235, 235, 240));

    int textY = line1Y + spacing;
    int paraH = (int)(h * 0.11);
    for (const QString& para : {para1, para2, para3, para4})
    {
        p.drawText(QRect(detailX + pillMargin, textY, detailW - pillMargin * 2, paraH),
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, para);
        textY += paraH + spacing;
    }

    // URL pill — narrower, inner margin
    int pillY   = textY + spacing;
    int pillLft = detailX + pillMargin;
    int pillWd  = detailW - pillMargin * 2;
    QPainterPath urlPath;
    addRoundedPillPath(urlPath, QRectF(pillLft, pillY, pillWd, btnH));
    QRect urlPillRect(pillLft, pillY, pillWd, btnH);
    paintPillFill(p, urlPath, urlPillRect, PillState::Normal);
    p.setFont(rowFont);
    p.setPen(QColor(235, 235, 240));
    p.drawText(urlPillRect, Qt::AlignCenter,
               "https://www.kingdomhearts.com/1525/us/");

    // Bottom gradient line — full outer margin width, one body-font line of space above
    int line2Y = pillY + btnH + QFontMetrics(rowFont).height();
    {
        int lLeft  = detailX + margin;
        int lRight = detailX + detailW - margin;
        QColor lc = themeColor; lc.setAlpha(180);
        QLinearGradient g(lLeft, 0, lRight, 0);
        g.setColorAt(0.0,  Qt::transparent);
        g.setColorAt(0.04, lc);
        g.setColorAt(0.96, lc);
        g.setColorAt(1.0,  Qt::transparent);
        p.setPen(QPen(QBrush(g), 1));
        p.drawLine(lLeft, line2Y, lRight, line2Y);
    }

    p.setFont(rowFont);
    p.setPen(QColor(235, 235, 240));
    p.drawText(QRect(detailX + pillMargin, line2Y + spacing / 2, detailW - pillMargin * 2, btnH),
               Qt::AlignCenter | Qt::AlignVCenter,
               QString::fromUtf8(locale().streamOpenHint));
}

void SettingsView::paintDetailQuit(QPainter& p, const DetailLayout& L)
{
    const int& h          = L.h;
    const int& detailX    = L.detailX;
    const int& detailW    = L.detailW;
    const int& startY     = L.startY;
    const int& btnH       = L.btnH;
    const int& spacing    = L.spacing;
    const QColor& themeColor = L.themeColor;

    const int margin = (int)(detailW * 0.07);
    const QString title = QString::fromUtf8(locale().sidebarLabels[kIdxQuit]);
    QFont titleFont("KHGummi");
    titleFont.setPixelSize(qMax(14, (int)(btnH * 0.60)));
    p.setFont(titleFont);
    p.setPen(themeColor);
    p.drawText(QRect(detailX + margin, startY, detailW - margin * 2, btnH),
               Qt::AlignVCenter | Qt::AlignCenter, title);
    int lineY = startY + btnH + spacing / 2;
    {
        int titleW = QFontMetrics(titleFont).horizontalAdvance(title);
        int pad    = (int)(detailW * 0.10);
        int lLeft  = detailX + (detailW - titleW) / 2 - pad;
        int lRight = lLeft + titleW + 2 * pad;
        QColor lc = themeColor; lc.setAlpha(180);
        QLinearGradient g(lLeft, 0, lRight, 0);
        g.setColorAt(0.0,  Qt::transparent);
        g.setColorAt(0.06, lc);
        g.setColorAt(0.94, lc);
        g.setColorAt(1.0,  Qt::transparent);
        p.setPen(QPen(QBrush(g), 1));
        p.drawLine(lLeft, lineY, lRight, lineY);
    }
    QFont bodyFont("KHMenu");
    bodyFont.setPixelSize(qMax(11, (int)(btnH * 0.47)));
    p.setFont(bodyFont);
    p.setPen(QColor(180, 180, 185));
    p.drawText(QRect(detailX + margin, lineY + spacing, detailW - margin * 2, (int)(h * 0.35)),
               Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
               QString::fromUtf8(locale().quitBody));
}

void SettingsView::paintDetailRows(QPainter& p, const DetailLayout& L)
{
    const int& h          = L.h;
    const int& detailX    = L.detailX;
    const int& detailW    = L.detailW;
    const int& startY     = L.startY;
    const int& btnH       = L.btnH;
    const int& spacing    = L.spacing;
    const qreal& bottomLineY = L.bottomLineY;
    const QFont& rowFont  = L.rowFont;
    const QColor& themeColor = L.themeColor;
    const int& rowPillW   = L.rowPillW;
    const int& scrollbarW = L.scrollbarW;

    // ── Row layout constants ───────────────────────────────────────────────────
    int labelH   = qMax(12, (int)(btnH * 0.68));
    int intraGap = qMax(2,  (int)(btnH * 0.10));
    int stride   = labelH + intraGap + btnH + spacing;

    QFont labelFont("KHMenu");
    labelFont.setPixelSize(qMax(11, (int)(btnH * 0.57)));

    // ── Regular rows ──────────────────────────────────────────────────────────
    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    if (rows.isEmpty()) return;

    bool inOptionList = (currentScreen == Screen::OptionList);
    int focusedPillY  = -1;

    for (int i = 0; i < rows.size(); i++)
    {
        const SettingRow& row = rows[i];
        int labelY = startY + i * stride - m_detailScrollOffset;
        int pillY  = labelY + labelH + intraGap;

        int footerTop = (int)(bottomLineY) - (int)(h * 0.09);
        if (pillY + btnH < startY || pillY + btnH > footerTop) continue;

        bool focused = (i == detailIndex);
        if (focused) focusedPillY = pillY;

        bool greyed = row.enabled && !row.enabled();

        if (greyed) p.setOpacity(0.35);

        p.setFont(labelFont);
        if (inOptionList && i != detailIndex)
        {
            p.setPen(QColor(themeColor.red() * 45/100,
                            themeColor.green() * 45/100,
                            themeColor.blue() * 45/100));
        }
        else
        {
            p.setPen(themeColor);
        }
        p.drawText(QRect(detailX, labelY, rowPillW, labelH),
                   Qt::AlignVCenter | Qt::AlignLeft, row.label);

        // Value in pill
        int val = row.read ? row.read() : 0;
        QString valStr;
        if (row.type == SettingRow::Type::Toggle)
            valStr = val ? locale().on : locale().off;
        else if (row.type == SettingRow::Type::Combobox)
        {
            if (!row.options.isEmpty() && val >= 0 && val < row.options.size())
                valStr = row.options[val];
            else
                valStr = rowDynamicValueText(sidebarIndex, i);
        }
        else if (row.type == SettingRow::Type::Slider)
            valStr = QString::number(val);
        else if (row.type == SettingRow::Type::Navigate)
            valStr = "\xe2\x86\x92"; // →

        QRect pillRect(detailX, pillY, rowPillW, btnH);
        bool isOpenRow = inOptionList && i == detailIndex;
        bool dimRow = inOptionList && i != detailIndex;

        if (row.type == SettingRow::Type::Slider)
        {
            QPainterPath sp2;
            addRoundedPillPath(sp2, QRectF(pillRect));
            QColor textCol = paintPillFill(p, sp2, pillRect,
                focused ? PillState::Selected : dimRow ? PillState::Dimmed : PillState::Normal);
            p.setFont(rowFont);

            int numPad   = (int)(btnH * 1.1);
            p.setPen(textCol);
            p.drawText(QRect(detailX + numPad, pillY, (int)(btnH * 1.5), btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, valStr);

            int trackX, trackW;
            sliderTrackGeometry(detailX, rowPillW, btnH, trackX, trackW);
            int trackMidY = pillY + btnH / 2;
            int trackH    = qMax(3, (int)(btnH * 0.12));
            float frac    = (float)(val - row.sliderMin) /
                            qMax(1, row.sliderMax - row.sliderMin);
            int fillW = (int)(frac * trackW);

            p.setPen(Qt::NoPen);
            p.setBrush(QColor(40, 40, 40, 200));
            p.drawRoundedRect(trackX, trackMidY - trackH / 2, trackW, trackH,
                              trackH / 2, trackH / 2);
            if (fillW > 0)
            {
                p.setBrush(focused ? themeColor : themeColor.darker(160));
                p.drawRect(trackX, trackMidY - trackH / 2, fillW, trackH);
            }
            int handleW_ = qMax(4, (int)(btnH * 0.14));
            int handleH_ = qMax(6, (int)(btnH * 0.52));
            p.setBrush(focused ? themeColor.lighter(140) : QColor(160, 160, 165));
            p.drawRect(trackX + fillW - handleW_ / 2, trackMidY - handleH_ / 2,
                       handleW_, handleH_);
            p.setBrush(Qt::NoBrush);
        }
        else
        {
            QPainterPath pp2;
            if (isOpenRow)
                addArrowPillPath(pp2, QRectF(pillRect));
            else
                addRoundedPillPath(pp2, QRectF(pillRect));
            QColor textCol = paintPillFill(p, pp2, pillRect,
                focused ? PillState::Selected : dimRow ? PillState::Dimmed : PillState::Normal);
            p.setFont(rowFont);
            int padding = (int)(btnH * 0.5);
            p.setPen(textCol);
            p.drawText(QRect(detailX + padding, pillY, rowPillW - padding * 2, btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, valStr);
        }

        if (greyed) p.setOpacity(1.0);
    }

    // Compact note in Game section when no KH ROM is loaded (Language row still shows above)
    if (sidebarIndex == kIdxGame)
    {
        EmuInstance* emu_ = m_mainWindow->getEmuInstance();
        bool khLoaded = emu_ && emu_->plugin && emu_->emuIsActive() &&
            emu_->plugin->supportsKHExtendedSettings();
        if (!khLoaded)
        {
            int noteY = startY + rows.size() * stride + spacing;
            QFont noteFont("KHMenu");
            noteFont.setPixelSize(qMax(11, (int)(btnH * 0.45)));
            p.setFont(noteFont);
            p.setPen(QColor(120, 115, 105));
            p.drawText(QRect(detailX + (int)(detailW * 0.06), noteY,
                             detailW - (int)(detailW * 0.12), (int)(h * 0.25)),
                       Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                       QString::fromUtf8(locale().noKHGameNote));
        }
    }

    if (currentScreen == Screen::Detail && focusedPillY >= 0)
    {
        qreal capR  = btnH / 2.0;
        qreal capCx = detailX + rowPillW - capR;
        qreal capCy = focusedPillY + btnH / 2.0;
        paintOrbAt(p, capCx, capCy, capR);
    }

    // Detail scrollbar just right of setting pills (hidden when OptionList active)
    if (currentScreen == Screen::Detail)
    {
        int totalH  = rows.size() * stride;
        int footerTop_ = (int)(bottomLineY) - (int)(h * 0.09);
        int visH    = footerTop_ - startY;
        int sbX     = detailX + rowPillW + (int)(detailW * 0.012);
        paintScrollbar(p, sbX, startY, scrollbarW, visH,
                       m_detailScrollOffset, totalH, visH);
    }

    if (inOptionList)
        paintOptionListPanel(p, L);

    if (currentScreen == Screen::Detail && detailIndex >= 0 && detailIndex < rows.size())
    {
        int descY = (int)(bottomLineY - (int)(h * 0.08));
        QFont descFont("KHMenu");
        descFont.setPixelSize(qMax(11, (int)(btnH * 0.43)));
        p.setFont(descFont);
        p.setPen(QColor(180, 180, 185));
        p.drawText(QRect(detailX, descY, detailW, (int)(h * 0.06)),
                   Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                   rows[detailIndex].description);
    }
}

// The combobox option panel, overlaid on the right of the Detail area while a Combobox row is open.
void SettingsView::paintOptionListPanel(QPainter& p, const DetailLayout& L)
{
    const int    h          = L.h;
    const int    startY     = L.startY;
    const int    btnH       = L.btnH;
    const int    spacing    = L.spacing;
    const qreal  bottomLineY = L.bottomLineY;
    const QFont& rowFont    = L.rowFont;
    const int    optPanelX  = L.optPanelX;
    const int    optPanelW  = L.optPanelW;
    const int    scrollbarW = L.scrollbarW;

    int labelH   = qMax(12, (int)(btnH * 0.68));
    int intraGap = qMax(2,  (int)(btnH * 0.10));

    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    QStringList opts = currentOptionList();
    int savedVal     = readRowValue(sidebarIndex, detailIndex);
    bool isThemeColor = (detailIndex >= 0 && detailIndex < rows.size() &&
                         rows[detailIndex].tag == SettingRow::Tag::ThemeColorPicker);

    int optStartY = startY + labelH + intraGap;
    int optVisH   = (int)(bottomLineY) - optStartY - (int)(h * 0.04);
    int totalOptH = opts.size() * (btnH + spacing);

    int optScrollGap = scrollbarW + (int)(optPanelW * 0.04);
    int optPillW     = optPanelW - optScrollGap;

    int focusedOptY = -1;
    p.setFont(rowFont);
    for (int i = 0; i < opts.size(); i++)
    {
        int optY = optStartY + i * (btnH + spacing) - m_optionScrollOffset;
        if (optY + btnH < optStartY || optY > optStartY + optVisH) continue;
        QRect r(optPanelX, optY, optPillW, btnH);
        bool foc   = (i == optionIndex);
        bool saved = (i == savedVal);
        if (foc) focusedOptY = optY;
        QColor swatch;
        if (isThemeColor && i < kThemePresetCount)
            swatch = QColor(kThemePresets[i].rgb);
        QPainterPath op;
        addRoundedPillPath(op, QRectF(r));
        QColor textCol = paintPillFill(p, op, r, foc ? PillState::Selected : PillState::Normal);
        p.setPen(textCol);
        int pad = (int)(btnH * 0.5);
        if (swatch.isValid())
        {
            int swatchSz = (int)(btnH * 0.45);
            int swatchX = r.left() + pad;
            int swatchY = r.top() + (btnH - swatchSz) / 2;
            p.setBrush(swatch); p.setPen(Qt::NoPen);
            p.drawEllipse(swatchX, swatchY, swatchSz, swatchSz);
            p.setBrush(Qt::NoBrush); p.setPen(textCol);
            QString txt = (saved ? "\xE2\x9C\x93 " : "  ") + opts[i];
            p.drawText(QRect(swatchX + swatchSz + (int)(btnH * 0.2),
                             r.top(), r.width() - pad - swatchSz, btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, txt);
        }
        else
        {
            QString txt = (saved ? "\xE2\x9C\x93 " : "  ") + opts[i];
            p.drawText(QRect(r.left() + pad, r.top(), r.width() - pad, btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, txt);
        }
    }

    if (focusedOptY >= 0)
    {
        qreal capR  = btnH / 2.0;
        qreal capCx = optPanelX + optPillW - capR;
        qreal capCy = focusedOptY + btnH / 2.0;
        paintOrbAt(p, capCx, capCy, capR);
    }

    // Option panel scrollbar (only for overflowing lists like Audio Pack)
    if (totalOptH > optVisH)
    {
        int optScrollbarX = optPanelX + optPanelW - scrollbarW - (int)(optPanelW * 0.02);
        paintScrollbar(p, optScrollbarX, optStartY, scrollbarW, optVisH,
                       m_optionScrollOffset, totalOptH, optVisH);
    }
}

void SettingsView::paintActionBar(QPainter& p)
{
    const int w = width();
    const int h = height();
    const qreal bottomLineY = contentBottomLineY();
    const int barY = (int)(bottomLineY + (int)(h * 0.010));

    QColor themeColor = currentThemeColor();

    const char* hintGlyphs[8] = {};
    const char* hintLabels[8] = {};
    int hintCount = 0;
    auto pushHint = [&](const char* g, const char* l) {
        if (hintCount < 8) { hintGlyphs[hintCount] = g; hintLabels[hintCount] = l; hintCount++; }
    };

    std::string confirmButtonLabel = "A";
    std::string cancelButtonLabel = "B";
    std::string resetButtonLabel = "X";
    std::string clearButtonLabel = "Y";

    EmuInstance* emu = m_mainWindow->getEmuInstance();
    auto& lcfg = emu->getLocalConfig();
    int uid = lcfg.GetInt("JoystickUniqueID");
    if (uid > 0)
    {
        Config::Table joycfg = lcfg.GetTable("Joystick." + std::to_string(uid));
        int confirmBtn = joycfg.GetInt("A");
        int backBtn = joycfg.GetInt("B");
        int resetBtn = joycfg.GetInt("X");
        int clearBtn = joycfg.GetInt("Y");

        if (confirmBtn >= 0 && backBtn >= 0 && resetBtn >= 0 && clearBtn >= 0) {
            SDL_GameController* controller = SDL_GameControllerOpen(emu->getJoystickIdByUniqueId(uid));

            confirmButtonLabel = JoyMappingName(controller, confirmBtn).toStdString();
            cancelButtonLabel = JoyMappingName(controller, backBtn).toStdString();
            resetButtonLabel = JoyMappingName(controller, resetBtn).toStdString();
            clearButtonLabel = JoyMappingName(controller, clearBtn).toStdString();

            SDL_GameControllerClose(controller);
        }
    }

    const SettingsLocale& loc = locale();
    if (m_waitingForBind)
    {
        pushHint(cancelButtonLabel.c_str(), loc.hintCancel);
    }
    else if (m_resettingBindings || m_resettingSection)
    {
        pushHint(confirmButtonLabel.c_str(), loc.hintYes);
        pushHint(cancelButtonLabel.c_str(), loc.hintNo);
    }
    else if (currentScreen == Screen::Sidebar)
    {
        pushHint(confirmButtonLabel.c_str(), loc.hintEnter);
        pushHint(cancelButtonLabel.c_str(), loc.hintClose);
    }
    else if (currentScreen == Screen::OptionList)
    {
        pushHint(confirmButtonLabel.c_str(), loc.hintConfirm);
        pushHint(cancelButtonLabel.c_str(), loc.hintCancel);
    }
    else if (currentScreen == Screen::Remap)
    {
        pushHint(confirmButtonLabel.c_str(), loc.hintRebind);
        pushHint(clearButtonLabel.c_str(), loc.hintClear);
        pushHint(resetButtonLabel.c_str(), loc.hintResetAll);
        pushHint(cancelButtonLabel.c_str(), loc.hintBack);
    }
    else if (currentScreen == Screen::Detail)
    {
        if (sidebarIndex == kIdxStream)
        {
            pushHint(confirmButtonLabel.c_str(), loc.hintOpenWebsite);
            pushHint(cancelButtonLabel.c_str(), loc.hintBack);
        }
        else if (sidebarIndex == kIdxQuit)
        {
            pushHint(confirmButtonLabel.c_str(), loc.hintQuitGame);
            pushHint(cancelButtonLabel.c_str(), loc.hintBack);
        }
        else
        {
            QVector<SettingRow> rows = rowsFor(sidebarIndex);
            if (!rows.isEmpty() && detailIndex >= 0 && detailIndex < rows.size())
            {
                const SettingRow& row = rows[detailIndex];
                if (row.type == SettingRow::Type::Toggle)
                    pushHint(confirmButtonLabel.c_str(), loc.hintToggle);
                else if (row.type == SettingRow::Type::Combobox)
                    pushHint(confirmButtonLabel.c_str(), loc.hintSelect);
                else if (row.type == SettingRow::Type::Slider)
                    pushHint("\xE2\x97\x80 \xE2\x96\xB6", loc.hintAdjust);
            }
            if (sidebarIndex != kIdxStream && sidebarIndex != kIdxQuit &&
                sidebarIndex != kIdxGamepad && sidebarIndex != kIdxKeyboard)
                pushHint(resetButtonLabel.c_str(), loc.hintReset);
            pushHint(cancelButtonLabel.c_str(), loc.hintBack);
        }
    }

    if (hintCount == 0) return;

    const int iconH   = qMax(14, (int)(h * 0.032));
    const float iconWMultiplier = (((float)iconH)/14.0)/1.8;
    const int gap     = (int)(w * 0.04);
    const int labelSz = qMax(10, (int)(h * 0.018));

    QFont iconFont("KHMenu");
    iconFont.setPixelSize(qMax(9, (int)(iconH * 0.62)));
    QFont labelFont("KHMenu");
    labelFont.setPixelSize(labelSz);

    int totalW = 0;
    p.setFont(iconFont);
    for (int i = 0; i < hintCount; i++)
    {
        totalW += (int)((p.fontMetrics().horizontalAdvance(hintGlyphs[i]) + 10) * iconWMultiplier);
    }
    p.setFont(labelFont);
    for (int i = 0; i < hintCount; i++)
    {
        totalW += (int)(iconH * 0.3);
        totalW += p.fontMetrics().horizontalAdvance(hintLabels[i]);
        totalW += gap;
    }
    totalW -= gap;

    int x = (w - totalW) / 2;
    for (int i = 0; i < hintCount; i++)
    {
        QColor boxFill(30, 30, 30, 220);
        QColor boxBorder = themeColor.darker(150);
        boxBorder.setAlpha(128);
        p.setBrush(boxFill);
        p.setPen(QPen(boxBorder, 1.5));
        p.setFont(iconFont);
        int iconW = (int)((p.fontMetrics().horizontalAdvance(hintGlyphs[i]) + 10) * iconWMultiplier);
        QRect iconRect(x, barY, iconW, iconH);
        p.drawRoundedRect(iconRect, 3, 3);
        p.setPen(Qt::white);
        p.drawText(iconRect, Qt::AlignCenter, hintGlyphs[i]);

        x += iconW + (int)(iconH * 0.3);

        p.setFont(labelFont);
        p.setPen(QColor(200, 200, 205));
        int labelW = p.fontMetrics().horizontalAdvance(hintLabels[i]);
        p.drawText(QRect(x, barY, labelW, iconH),
                   Qt::AlignVCenter | Qt::AlignLeft, hintLabels[i]);
        x += labelW + gap;
    }
}

void SettingsView::paintResetConfirmation(QPainter& p)
{
    p.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 160));

    QRect box, yesRect, noRect;
    confirmPopupRects(box, yesRect, noRect);
    const int boxW = box.width(), boxH = box.height();

    p.fillRect(box, QColor(20, 20, 30, 220));
    p.setPen(QPen(QColor(200, 200, 200), 2));
    p.drawRect(box);

    QFont titleFont("KHMenu");
    titleFont.setPixelSize(qMax(13, (int)(boxH * 0.12)));
    const SettingsLocale& loc = locale();
    p.setFont(titleFont);
    p.setPen(Qt::white);
    QString confirmText = m_resettingSection
        ? QString::fromUtf8(loc.resetSectionPrompt).arg(QString::fromUtf8(loc.sidebarLabels[sidebarIndex]))
        : QString::fromUtf8(loc.resetAllBindings);
    p.drawText(QRect(box.x() + (int)(boxW * 0.05), box.y() + (int)(boxH * 0.08),
                     (int)(boxW * 0.90), (int)(boxH * 0.52)),
               Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, confirmText);

    const int btnH = yesRect.height();
    QFont btnFont("KHMenu");
    btnFont.setPixelSize(qMax(12, (int)(btnH * 0.50)));
    p.setFont(btnFont);
    paintPillButton(p, yesRect,
                    QString("A ") + QString::fromUtf8(loc.pillYes),
                    m_resettingConfirmIndex == 0 ? PillState::Selected : PillState::Normal);
    paintPillButton(p, noRect,
                    QString("B ") + QString::fromUtf8(loc.pillNo),
                    m_resettingConfirmIndex == 1 ? PillState::Selected : PillState::Normal);
}

void SettingsView::paintCaptureOverlay(QPainter& p)
{
    const int w = width();
    const int h = height();
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 180));

    QFont font("KHMenu");
    font.setPixelSize(qMax(14, (int)(h * 0.028)));
    p.setFont(font);
    const SettingsLocale& loc = locale();
    p.setPen(Qt::white);
    QString prompt = QString::fromUtf8(m_remapIsJoystick ? loc.captureController : loc.captureKey);
    p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, prompt);

    QString subText;
    if (!m_remapIsJoystick)
        subText = QString::fromUtf8(loc.captureCancelKeyboard);
    else if (!m_bindArmed)
        subText = QString::fromUtf8(loc.captureReleaseHint);
    else
    {
        qint64 remainMs = kBindAssignTimeoutMs - (m_animClock.elapsed() - m_bindArmTime);
        int secs = (int)((remainMs + 999) / 1000);
        if (secs < 0) secs = 0;
        subText = QString::fromUtf8(loc.captureCancelCountdown).arg(secs);
    }

    QFont subFont("KHMenu");
    subFont.setPixelSize(qMax(10, (int)(h * 0.018)));
    p.setFont(subFont);
    p.setPen(QColor(0xA0, 0x98, 0x80));
    p.drawText(QRect(0, h / 2 + (int)(h * 0.04), w, (int)(h * 0.04)),
               Qt::AlignCenter, subText);
}

void SettingsView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    paintBackground(p);
    paintTitleBar(p);
    paintBottomLine(p);
    paintSidebar(p);
    paintDetailArea(p);
    paintActionBar(p);

    if (m_waitingForBind)
        paintCaptureOverlay(p);
    else if (m_resettingBindings || m_resettingSection)
        paintResetConfirmation(p);
}
