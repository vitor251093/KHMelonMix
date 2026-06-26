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
#include "MainWindowSettings.h"
#include "EmuInstance.h"
#include "plugins/Plugin.h"
#include "plugins/PluginKingdomHeartsDays.h"
#include "plugins/PluginKingdomHeartsReCoded.h"

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

static const char* kSidebarLabels[] = {
    "Game",
    "Emulation",
    "Display",
    "Sound",
    "Gamepad",
    "Keyboard",
    "System",
    "Before You Stream\xE2\x80\xA6",
    "Quit Game",
};
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

static const char* kSectionOverview[] = {
    "Settings specific to the loaded Kingdom Hearts game.",
    "Configure console type, frame rate, and runtime behavior.",
    "Renderer, resolution, synchronization, and visual theme.",
    "Volume, audio quality, and microphone input.",
    "Remap controller buttons and analog stick assignments.",
    "Remap keyboard bindings for DS buttons and hotkeys.",
    "Network mode, battery simulation, and firmware options.",
    "Streaming guidelines and copyright information.",
    "Exit the current game session.",
};

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

static QString joyBindingText(int id)
{
    if (id == -1) return "None";
    bool hasbtn = ((id & 0xFFFF) != 0xFFFF);
    QString str;
    if (hasbtn)
    {
        if (id & 0x100)
        {
            int hatnum = ((id >> 4) & 0xF) + 1;
            switch (id & 0xF)
            {
            case 0x1: str = QString("Hat %1 Up").arg(hatnum);    break;
            case 0x2: str = QString("Hat %1 Right").arg(hatnum); break;
            case 0x4: str = QString("Hat %1 Down").arg(hatnum);  break;
            case 0x8: str = QString("Hat %1 Left").arg(hatnum);  break;
            default:  str = QString("Hat %1").arg(hatnum);        break;
            }
        }
        else
        {
            str = QString("Button %1").arg((id & 0xFFFF) + 1);
        }
    }
    if (id & 0x10000)
    {
        int axisnum = ((id >> 24) & 0xF) + 1;
        if (hasbtn) str += " / ";
        switch ((id >> 20) & 0xF)
        {
        case 0: str += QString("Axis %1 +").arg(axisnum);  break;
        case 1: str += QString("Axis %1 -").arg(axisnum);  break;
        case 2: str += QString("Trigger %1").arg(axisnum); break;
        }
    }
    return str.isEmpty() ? "None" : str;
}

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
    EmuInstance* emu = m_mainWindow ? m_mainWindow->getEmuInstance() : nullptr;

    switch (idx)
    {
    case kIdxGame:
    {
        if (!emu || !emu->plugin) break;
        u32 gc = emu->plugin->getGameCode();
        bool isDays    = PluginKingdomHeartsDays::isCart(gc);
        bool isReCoded = PluginKingdomHeartsReCoded::isCart(gc);
        if (!isDays && !isReCoded) break;

        rows.append({ SettingRow::Type::Combobox, "Language",
            "Firmware language used for cutscene subtitles and pause menus.",
            {"English", "Japanese", "French", "German", "Italian", "Spanish"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Fast-forward Loading",
            "Speed through loading screens automatically.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Skip Cutscenes Instantly",
            "Press a button during a cutscene to skip it immediately.", {}, 0, 0 });
        if (isDays)
            rows.append({ SettingRow::Type::Toggle, "Disable \"His\" Memories",
                "Remove the additional memory episodes added by this port.", {}, 0, 0 });
        break;
    }
    case kIdxEmulation:
        rows.append({ SettingRow::Type::Combobox, "Console Type",
            "Emulated console. Changes take effect after restarting the game.",
            {"DS", "DSi"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Direct Boot",
            "Skip the DS boot sequence and launch the game directly.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "FPS Limit",
            "Cap emulation speed to the target frame rate.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Target FPS",
            "Frame rate cap for normal play.",
            {"30", "60", "120"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Fast-forward FPS",
            "Speed cap while fast-forward is held.",
            {"2\xC3\x97", "4\xC3\x97", "8\xC3\x97", "Unlimited"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Slowmo FPS",
            "Frame rate cap while slow-motion is active.",
            {"25%", "50%", "75%"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Mute Fast-forward",
            "Silence audio while fast-forward is held.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Pause on Lost Focus",
            "Pause emulation when the window loses focus.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "Hide Mouse",
            "Hide the mouse cursor while the emulator window is active.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Hide Mouse After",
            "Automatically hide the cursor after this period of inactivity.",
            {"3 seconds", "5 seconds", "10 seconds", "30 seconds"}, 0, 0 });
        break;
    case kIdxDisplay:
        rows.append({ SettingRow::Type::Combobox, "3D Renderer",
            "Graphics renderer for 3D scenes. OpenGL modes require GPU support.",
            {"Software", "OpenGL", "OpenGL Compute"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "3D Resolution",
            "Internal rendering scale for 3D graphics.",
            {"1\xC3\x97", "2\xC3\x97",  "3\xC3\x97",  "4\xC3\x97",
             "5\xC3\x97", "6\xC3\x97",  "7\xC3\x97",  "8\xC3\x97",
             "9\xC3\x97", "10\xC3\x97", "11\xC3\x97", "12\xC3\x97",
             "13\xC3\x97","14\xC3\x97", "15\xC3\x97", "16\xC3\x97"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "VSync",
            "Synchronize rendering to your display refresh rate.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Theme Color",
            "Settings menu accent color inspired by each Kingdom Hearts title.",
            {"Auto", "358/2 Days", "Re:coded", "Kingdom Hearts",
             "Birth by Sleep", "KH III", "Dream Drop Distance", "Melody of Memory", "KH IV"},
            0, 0 });
        if (emu && emu->plugin)
        {
            u32 gc2 = emu->plugin->getGameCode();
            if (PluginKingdomHeartsDays::isCart(gc2) || PluginKingdomHeartsReCoded::isCart(gc2))
            {
                rows.append({ SettingRow::Type::Toggle, "Enhanced Graphics",
                    "Enable high-resolution backgrounds and models.", {}, 0, 0 });
                rows.append({ SettingRow::Type::Toggle, "Single Screen Mode",
                    "Display only the top screen, scaled to fill the window.", {}, 0, 0 });
                rows.append({ SettingRow::Type::Toggle, "Show Subtitles",
                    "Display subtitles during HD cutscene playback.", {}, 0, 0 });
                rows.append({ SettingRow::Type::Slider, "HUD Scale",
                    "Scale the heads-up display elements.", {}, 1, 10 });
            }
        }
        break;
    case kIdxSound:
        rows.append({ SettingRow::Type::Slider, "Volume",
            "Game audio output level.", {}, 0, 10 });
        rows.append({ SettingRow::Type::Combobox, "Interpolation",
            "Audio resampling quality. Higher settings improve fidelity but use more CPU.",
            {"None", "Linear", "Cosine", "Cubic", "Gaussian"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Bit Depth",
            "Audio sample bit depth. 10-bit mimics original DS hardware.",
            {"Auto", "10-bit", "16-bit"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "DSi Volume Sync",
            "Sync DS audio volume with the DSi firmware volume level.", {}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "Mic Input",
            "Microphone input source for games that use the DS microphone.",
            {"None", "External", "Noise", "WAV"}, 0, 0 });
        if (emu && emu->plugin)
        {
            u32 gc2 = emu->plugin->getGameCode();
            if (PluginKingdomHeartsDays::isCart(gc2) || PluginKingdomHeartsReCoded::isCart(gc2))
                rows.append({ SettingRow::Type::Combobox, "Audio Pack",
                    "Replacement music pack for in-game audio.", {}, 0, 0 });
        }
        break;
    case kIdxKeyboard:
    {
        if (!emu || !emu->plugin) break;
        u32 gc = emu->plugin->getGameCode();
        if (!PluginKingdomHeartsDays::isCart(gc) && !PluginKingdomHeartsReCoded::isCart(gc)) break;
        rows.append({ SettingRow::Type::Slider, "Camera Sensitivity",
            "Speed of the touch-screen camera controls.", {}, 1, 10 });
        rows.append({ SettingRow::Type::Navigate, "Key Bindings",
            "Assign keyboard keys to DS buttons and hotkeys.", {}, 0, 0 });
        break;
    }
    case kIdxSystem:
        rows.append({ SettingRow::Type::Combobox, "WiFi Mode",
            "Connection mode for wireless features.",
            {"Indirect", "Direct"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "WiFi Adapter",
            "Network adapter used for Direct Mode. Only active when Direct is selected.",
            {}, 0, 0 }); // options populated dynamically
        rows.append({ SettingRow::Type::Combobox, "DS Battery",
            "Simulated DS battery level for games that check it.",
            {"Low", "Okay"}, 0, 0 });
        rows.append({ SettingRow::Type::Combobox, "DSi Battery Level",
            "Simulated DSi battery charge level.",
            {"Almost Empty", "Low", "Half", "High", "Full"}, 0, 0 });
        rows.append({ SettingRow::Type::Toggle, "DSi Charging",
            "Whether the simulated DSi battery is charging.", {}, 0, 0 });
        break;
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
}

void SettingsView::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    m_animTimer->stop();
    m_joyTimer->stop();
    m_pendingClose    = false;
    m_waitingForBind  = false;
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
            rows[detailIndex].label == "Theme Color" &&
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

// ─── geometry ─────────────────────────────────────────────────────────────────

void SettingsView::computeSidebarGeometry(int& sX, int& sW, int& sY,
                                           int& bH, int& sp, int& dX) const
{
    const int w = width();
    const int h = height();
    const int barY_d   = (int)(h * 0.11);
    const int barH_d   = (int)(h * 0.045);
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

// ─── config read / write ──────────────────────────────────────────────────────

int SettingsView::readRowValue(int sidebar, int row) const
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return 0;
    auto& gcfg = emu->getGlobalConfig();
    auto& lcfg = emu->getLocalConfig();

    switch (sidebar)
    {
    case kIdxGame:
    {
        if (!emu->plugin) return 0;
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        switch (row)
        {
        case 0: // Language — stored: 0=Jap 1=Eng 2=Fr 3=Ger 4=It 5=Sp
        {        //           display: 0=Eng 1=Jap 2=Fr 3=Ger 4=It 5=Sp
            int v = gcfg.GetInt("Instance0.Firmware.Language");
            if (v == 1) return 0; // English
            if (v == 0) return 1; // Japanese
            return v;
        }
        case 1: return gcfg.GetBool(prefix + "FastForwardLoadingScreens") ? 1 : 0;
        case 2: return gcfg.GetBool(prefix + "InstantSkipCutsceneOnStart") ? 1 : 0;
        case 3: return gcfg.GetBool(prefix + "DaysDisableHisMemories")     ? 1 : 0;
        }
        break;
    }
    case kIdxEmulation:
        switch (row)
        {
        case 0: return gcfg.GetInt("Emu.ConsoleType");
        case 1: return gcfg.GetBool("Emu.DirectBoot") ? 1 : 0;
        case 2: return gcfg.GetBool("LimitFPS")        ? 1 : 0;
        case 3: // Target FPS
        {
            double v = gcfg.GetDouble("TargetFPS");
            for (int i = 0; i < kTargetFPSCount; i++)
                if (qAbs(v - kTargetFPSPresets[i]) < 0.5) return i;
            return 1; // default 60
        }
        case 4: // Fast-forward FPS
        {
            double v = gcfg.GetDouble("FastForwardFPS");
            for (int i = 0; i < kFFPSCount; i++)
                if (qAbs(v - kFFPSPresets[i]) < 0.5) return i;
            return 3; // default Unlimited
        }
        case 5: // Slowmo FPS
        {
            double v = gcfg.GetDouble("SlowmoFPS");
            for (int i = 0; i < kSlowmoFPSCount; i++)
                if (qAbs(v - kSlowmoFPSPresets[i]) < 0.5) return i;
            return 1; // default 50%
        }
        case 6: return gcfg.GetBool("MuteFastForward")  ? 1 : 0;
        case 7: return gcfg.GetBool("PauseLostFocus")   ? 1 : 0;
        case 8: return gcfg.GetBool("Mouse.Hide")       ? 1 : 0;
        case 9: // Hide Mouse After
        {
            int v = gcfg.GetInt("Mouse.HideSeconds");
            for (int i = 0; i < kMouseHideCount; i++)
                if (v == kMouseHidePresets[i]) return i;
            return 0;
        }
        }
        break;
    case kIdxDisplay:
        switch (row)
        {
        case 0: return gcfg.GetInt("3D.Renderer");
        case 1: return qBound(0, gcfg.GetInt("3D.GL.ScaleFactor") - 1, 15);
        case 2: return gcfg.GetBool("Screen.VSync") ? 1 : 0;
        case 3:
        {
            QString saved = gcfg.GetQString("KHSettings.ThemeColor");
            if (!saved.isEmpty())
            {
                QColor c(saved);
                for (int i = 1; i < kThemePresetCount; i++)
                    if (QColor(kThemePresets[i].rgb).rgb() == c.rgb()) return i;
            }
            return 0;
        }
        case 4: case 5: case 6: case 7:
        {
            if (!emu->plugin) return 0;
            std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
            if (row == 4) return gcfg.GetBool(prefix + "DisableEnhancedGraphics") ? 0 : 1;
            if (row == 5) return gcfg.GetBool(prefix + "DisableSingleScreenMode") ? 0 : 1;
            if (row == 6) return gcfg.GetBool(prefix + "SubtitlesEnabled")        ? 1 : 0;
            if (row == 7) { int v = gcfg.GetInt(prefix + "HUDScale"); return v == 0 ? 4 : v; }
            return 0;
        }
        }
        break;
    case kIdxSound:
        switch (row)
        {
        case 0: return lcfg.GetInt("Audio.Volume") / 25;
        case 1: return gcfg.GetInt("Audio.Interpolation");
        case 2: return gcfg.GetInt("Audio.BitDepth");
        case 3: return lcfg.GetBool("Audio.DSiVolumeSync") ? 1 : 0;
        case 4: return gcfg.GetInt("Mic.InputType");
        case 5: // Audio Pack (dynamic combobox; moved from kIdxGame)
        {
            if (!emu->plugin) return 0;
            if (m_audioPackNames.isEmpty())
            {
                m_audioPackNames.clear();
                m_audioPackNames.append("None");
                auto names = emu->plugin->audioPackNames();
                for (const auto& n : names)
                    m_audioPackNames.append(QString::fromStdString(n));
            }
            std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
            QString saved = gcfg.GetQString(prefix + "AudioPack");
            if (saved.isEmpty()) return 0;
            for (int i = 1; i < m_audioPackNames.size(); i++)
                if (m_audioPackNames[i] == saved) return i;
            return 0;
        }
        }
        break;
    case kIdxKeyboard:
    {
        if (!emu->plugin) return 0;
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        switch (row)
        {
        case 0: { int v = gcfg.GetInt(prefix + "CameraSensitivity"); return v == 0 ? 3 : v; }
        }
        break;
    }
    case kIdxSystem:
        switch (row)
        {
        case 0: return gcfg.GetBool("LAN.DirectMode") ? 1 : 0;
        case 1: // WiFi Adapter (dynamic)
        {
            QString saved = gcfg.GetQString("LAN.Device");
            for (int i = 0; i < m_wifiAdapters.size(); i++)
                if (m_wifiAdapters[i] == saved) return i;
            return 0;
        }
        case 2: return gcfg.GetBool("DS.Battery.LevelOkay") ? 1 : 0;
        case 3: // DSi Battery Level (0-15 → 5 presets)
        {
            int v = gcfg.GetInt("DSi.Battery.Level");
            for (int i = 0; i < kDsiBattCount; i++)
                if (v == kDsiBattLevels[i]) return i;
            return 2; // default Half
        }
        case 4: return gcfg.GetBool("DSi.Battery.Charging") ? 1 : 0;
        }
        break;
    }
    return 0;
}

void SettingsView::writeRowValue(int sidebar, int row, int newValue)
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;
    auto& gcfg = emu->getGlobalConfig();
    auto& lcfg = emu->getLocalConfig();

    switch (sidebar)
    {
    case kIdxGame:
    {
        if (!emu->plugin) break;
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        switch (row)
        {
        case 0: // Language — display: 0=Eng 1=Jap 2-5 same
        {
            int stored = (newValue == 0) ? 1 : (newValue == 1) ? 0 : newValue;
            gcfg.SetInt("Instance0.Firmware.Language", stored);
            break;
        }
        case 1: gcfg.SetBool(prefix + "FastForwardLoadingScreens",  newValue); break;
        case 2: gcfg.SetBool(prefix + "InstantSkipCutsceneOnStart", newValue); break;
        case 3: gcfg.SetBool(prefix + "DaysDisableHisMemories",     newValue); break;
        }
        Config::Save();
        emu->plugin->shouldInvalidateConfigs = true;
        break;
    }
    case kIdxEmulation:
        switch (row)
        {
        case 0: gcfg.SetInt("Emu.ConsoleType", newValue); break;
        case 1: gcfg.SetBool("Emu.DirectBoot", newValue != 0); break;
        case 2: gcfg.SetBool("LimitFPS", newValue != 0); break;
        case 3: if (newValue < kTargetFPSCount)
                    gcfg.SetDouble("TargetFPS", kTargetFPSPresets[newValue]); break;
        case 4: if (newValue < kFFPSCount)
                    gcfg.SetDouble("FastForwardFPS", kFFPSPresets[newValue]); break;
        case 5: if (newValue < kSlowmoFPSCount)
                    gcfg.SetDouble("SlowmoFPS", kSlowmoFPSPresets[newValue]); break;
        case 6: gcfg.SetBool("MuteFastForward", newValue != 0); break;
        case 7: gcfg.SetBool("PauseLostFocus",  newValue != 0); break;
        case 8: gcfg.SetBool("Mouse.Hide",      newValue != 0); break;
        case 9: if (newValue < kMouseHideCount)
                    gcfg.SetInt("Mouse.HideSeconds", kMouseHidePresets[newValue]); break;
        }
        Config::Save();
        break;
    case kIdxDisplay:
        switch (row)
        {
        case 0:
            gcfg.SetInt("3D.Renderer", newValue);
            emu->getEmuThread()->updateVideoSettings();
            break;
        case 1:
            gcfg.SetInt("3D.GL.ScaleFactor", newValue + 1);
            emu->getEmuThread()->updateVideoSettings();
            break;
        case 2:
            gcfg.SetBool("Screen.VSync", newValue != 0);
            emu->getEmuThread()->updateVideoSettings();
            break;
        case 3:
            if (newValue >= 0 && newValue < kThemePresetCount)
            {
                if (newValue == 0)
                    gcfg.SetQString("KHSettings.ThemeColor", QString());
                else
                    gcfg.SetQString("KHSettings.ThemeColor",
                                    QColor(kThemePresets[newValue].rgb).name());
            }
            break;
        case 4: case 5: case 6: case 7:
            if (emu->plugin)
            {
                std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                if (row == 4) gcfg.SetBool(prefix + "DisableEnhancedGraphics", !newValue);
                else if (row == 5) gcfg.SetBool(prefix + "DisableSingleScreenMode", !newValue);
                else if (row == 6) gcfg.SetBool(prefix + "SubtitlesEnabled",         newValue);
                else if (row == 7) gcfg.SetInt(prefix + "HUDScale",                  newValue);
                emu->plugin->shouldInvalidateConfigs = true;
            }
            break;
        }
        Config::Save();
        break;
    case kIdxSound:
        switch (row)
        {
        case 0:
        {
            int vol = qBound(0, newValue * 25, 256);
            lcfg.SetInt("Audio.Volume", vol);
            emu->setAudioVolume(vol);
            break;
        }
        case 1: gcfg.SetInt("Audio.Interpolation", newValue); break;
        case 2: gcfg.SetInt("Audio.BitDepth",       newValue); break;
        case 3: lcfg.SetBool("Audio.DSiVolumeSync", newValue != 0); break;
        case 4: gcfg.SetInt("Mic.InputType",        newValue); break;
        case 5: // Audio Pack (moved from kIdxGame)
            if (emu->plugin)
            {
                std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
                if (newValue == 0)
                    gcfg.SetQString(prefix + "AudioPack", QString());
                else if (newValue > 0 && newValue < m_audioPackNames.size())
                    gcfg.SetQString(prefix + "AudioPack", m_audioPackNames[newValue]);
                emu->plugin->shouldInvalidateConfigs = true;
            }
            break;
        }
        Config::Save();
        emu->applyAudioSettings();
        break;
    case kIdxKeyboard:
        if (emu->plugin && row == 0)
        {
            std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
            gcfg.SetInt(prefix + "CameraSensitivity", newValue);
            Config::Save();
            emu->plugin->shouldInvalidateConfigs = true;
        }
        break;
    case kIdxSystem:
        switch (row)
        {
        case 0: gcfg.SetBool("LAN.DirectMode", newValue != 0); break;
        case 1: // WiFi Adapter
            if (newValue >= 0 && newValue < m_wifiAdapters.size())
                gcfg.SetQString("LAN.Device",
                                m_wifiAdapters[newValue]);
            break;
        case 2: gcfg.SetBool("DS.Battery.LevelOkay", newValue != 0); break;
        case 3: if (newValue < kDsiBattCount)
                    gcfg.SetInt("DSi.Battery.Level", kDsiBattLevels[newValue]); break;
        case 4: gcfg.SetBool("DSi.Battery.Charging", newValue != 0); break;
        }
        Config::Save();
        break;
    }
}

QString SettingsView::rowDynamicValueText(int sidebar, int row) const
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return {};
    auto& gcfg = emu->getGlobalConfig();
    if (sidebar == kIdxSound && row == 5 && emu->plugin)
    {
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        QString saved = gcfg.GetQString(prefix + "AudioPack");
        return saved.isEmpty() ? "None" : saved;
    }
    if (sidebar == kIdxSystem && row == 1)
        return gcfg.GetQString("LAN.Device");
    return {};
}

QStringList SettingsView::currentOptionList() const
{
    QVector<SettingRow> rows = rowsFor(sidebarIndex);
    if (detailIndex < 0 || detailIndex >= rows.size()) return {};
    const SettingRow& row = rows[detailIndex];
    if (!row.options.isEmpty()) return row.options;
    // Dynamic lists
    if (sidebarIndex == kIdxSound  && detailIndex == 5) return m_audioPackNames;
    if (sidebarIndex == kIdxSystem && detailIndex == 1) return m_wifiAdapters;
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
    m_audioPackNames.append("None");
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
            for (int i = 0; i < (int)labels.size(); i++)
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
    auto& gcfg = emu->getGlobalConfig();
    auto& lcfg = emu->getLocalConfig();

    switch (idx)
    {
    case kIdxGame:
    {
        if (!emu->plugin) return;
        std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
        gcfg.SetInt("Instance0.Firmware.Language",          1); // English
        gcfg.SetBool(prefix + "FastForwardLoadingScreens", false);
        gcfg.SetBool(prefix + "InstantSkipCutsceneOnStart", false);
        gcfg.SetBool(prefix + "DaysDisableHisMemories",    false);
        emu->plugin->shouldInvalidateConfigs = true;
        break;
    }
    case kIdxEmulation:
        gcfg.SetInt("Emu.ConsoleType",              0);
        gcfg.SetBool("Emu.DirectBoot",              true);
        gcfg.SetBool("LimitFPS",                    true);
        gcfg.SetDouble("TargetFPS",                 60.0);
        gcfg.SetDouble("FastForwardFPS",            1000.0);
        gcfg.SetDouble("SlowmoFPS",                 30.0);
        gcfg.SetBool("MuteFastForward",             false);
        gcfg.SetBool("PauseLostFocus",              false);
        gcfg.SetBool("Mouse.Hide",                  true);
        gcfg.SetInt("Mouse.HideSeconds",            3);
        gcfg.SetInt("Instance0.Firmware.Language",  1);
        if (emu->plugin) emu->plugin->shouldInvalidateConfigs = true;
        break;
    case kIdxDisplay:
        gcfg.SetInt("3D.Renderer",               1);
        gcfg.SetInt("3D.GL.ScaleFactor",         3);
        gcfg.SetBool("Screen.VSync",             false);
        gcfg.SetQString("KHSettings.ThemeColor", QString());
        if (emu->plugin)
        {
            std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
            gcfg.SetBool(prefix + "DisableEnhancedGraphics", false);
            gcfg.SetBool(prefix + "DisableSingleScreenMode", false);
            gcfg.SetBool(prefix + "SubtitlesEnabled",        true);
            gcfg.SetInt(prefix + "HUDScale",                 4);
            emu->plugin->shouldInvalidateConfigs = true;
        }
        emu->getEmuThread()->updateVideoSettings();
        break;
    case kIdxSound:
        lcfg.SetInt("Audio.Volume",         256);
        gcfg.SetInt("Audio.Interpolation",   0);
        gcfg.SetInt("Audio.BitDepth",        0);
        lcfg.SetBool("Audio.DSiVolumeSync",  false);
        gcfg.SetInt("Mic.InputType",         1);
        if (emu->plugin)
        {
            std::string prefix = emu->plugin->tomlUniqueIdentifier() + ".";
            gcfg.SetQString(prefix + "AudioPack", QString());
            emu->plugin->shouldInvalidateConfigs = true;
        }
        emu->setAudioVolume(256);
        emu->applyAudioSettings();
        break;
    case kIdxSystem:
        gcfg.SetBool("LAN.DirectMode",              false);
        gcfg.SetBool("DS.Battery.LevelOkay",        true);
        gcfg.SetInt("DSi.Battery.Level",            15);
        gcfg.SetBool("DSi.Battery.Charging",        true);
        break;
    }
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
    int rowPillW = (int)(dW * 0.56);
    if (pos.x() > dX + rowPillW) return -1;

    int labelH   = qMax(10, (int)(bH * 0.45));
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

    int rowPillW  = (int)(dW * 0.56);
    int optGapW   = (int)(dW * 0.06);
    int optPanelX = dX + rowPillW + optGapW;
    if (pos.x() < optPanelX) return -1;

    int labelH    = qMax(10, (int)(bH * 0.45));
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

void SettingsView::pollJoystick()
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;
    SDL_Joystick* joy = emu->getJoystick();
    if (!joy) return;
    SDL_JoystickUpdate();

    if (m_waitingForBind && m_remapIsJoystick)
    {
        int numButtons = SDL_JoystickNumButtons(joy);
        for (int i = 0; i < numButtons; i++)
        {
            if (SDL_JoystickGetButton(joy, i))
            {
                writeRemapBinding(m_bindTarget, i);
                m_waitingForBind = false;
                update();
                return;
            }
        }
        int numHats = SDL_JoystickNumHats(joy);
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
                update();
                return;
            }
        }
        int numAxes = qMin(SDL_JoystickNumAxes(joy), 16);
        for (int i = 0; i < numAxes; i++)
        {
            Sint16 val  = SDL_JoystickGetAxis(joy, i);
            Sint16 rest = m_axesRest[i];
            if (abs(val - rest) >= 16384)
            {
                int axisType;
                if (rest < -16384) axisType = 2;
                else               axisType = (val > rest) ? 0 : 1;
                int encoded = 0x10000 | (axisType << 20) | (i << 24);
                writeRemapBinding(m_bindTarget, encoded);
                m_waitingForBind = false;
                update();
                return;
            }
        }
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
    bool yBtn    = emu->joystickButtonDown(emu->getJoyMapping(11)) != 0;

    int dir = -1;
    if      (up)      dir = 0;
    else if (down)    dir = 1;
    else if (left)    dir = 2;
    else if (right)   dir = 3;
    else if (confirm) dir = 4;
    else if (backNow) dir = 5;
    else if (yBtn && (currentScreen == Screen::Remap || currentScreen == Screen::Detail)) dir = 6;

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

// ─── navigation ───────────────────────────────────────────────────────────────

void SettingsView::handleNavigation(int direction)
{
    // ── Section Reset confirmation ─────────────────────────────────────────────
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

    // ── Reset All confirmation ─────────────────────────────────────────────────
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

    // ── Sidebar ────────────────────────────────────────────────────────────────
    if (currentScreen == Screen::Sidebar)
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
                if (sidebarIndex == kIdxKeyboard && !kbRows.isEmpty())
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
        return;
    }

    // ── Detail ─────────────────────────────────────────────────────────────────
    if (currentScreen == Screen::Detail)
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

        const SettingRow& row = rows[detailIndex];

        // ─ helpers ─────────────────────────────────────────────────────────────
        auto scrollDetailToRow = [&](int idx) {
            int dX, dW, sY, bH, sp;
            computeDetailGeometry(dX, dW, sY, bH, sp);
            const int h = height();
            const qreal topLineY    = (int)(h * 0.11) + (int)(h * 0.045) / 2.0;
            const qreal bottomLineY = h - topLineY;
            int labelH   = qMax(10, (int)(bH * 0.45));
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
        };

        switch (direction)
        {
        case 0:
            detailIndex = (detailIndex - 1 + rowCount) % rowCount;
            scrollDetailToRow(detailIndex);
            playSound(2);
            update();
            break;
        case 1:
            detailIndex = (detailIndex + 1) % rowCount;
            scrollDetailToRow(detailIndex);
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
            // Fix 14/15: block interaction with greyed rows
            if (sidebarIndex == kIdxSystem)
            {
                EmuInstance* emu_ = m_mainWindow->getEmuInstance();
                int ct = emu_ ? emu_->getGlobalConfig().GetInt("Emu.ConsoleType") : 0;
                if ((detailIndex == 2 && ct == 1) || (detailIndex == 3 && ct == 0) ||
                    (detailIndex == 4 && ct == 0)) break;
            }
            if (sidebarIndex == kIdxSound && detailIndex == 3)
            {
                EmuInstance* emu_ = m_mainWindow->getEmuInstance();
                int ct = emu_ ? emu_->getGlobalConfig().GetInt("Emu.ConsoleType") : 0;
                if (ct == 0) break;
            }
            if (row.type == SettingRow::Type::Slider)
            {
                int cur = readRowValue(sidebarIndex, detailIndex);
                int nv  = qMin(row.sliderMax, cur + 1);
                if (nv != cur) { writeRowValue(sidebarIndex, detailIndex, nv); update(); }
            }
            else if (row.type == SettingRow::Type::Combobox)
            {
                if (sidebarIndex == kIdxSystem && detailIndex == 1 &&
                    readRowValue(kIdxSystem, 0) == 0) break;
                if (sidebarIndex == kIdxSystem && detailIndex == 1)
                    populateWifiAdapters();
                if (sidebarIndex == kIdxSound && detailIndex == 5)
                    populateAudioPacks();
                m_optionScrollOffset = 0;
                optionIndex = readRowValue(sidebarIndex, detailIndex);
                currentScreen = Screen::OptionList;
                playSound(1);
                update();
            }
            else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxKeyboard)
            {
                m_remapIsJoystick   = false;
                m_remapScrollOffset = 0;
                detailIndex         = 0;
                buildRemapList();
                currentScreen = Screen::Remap;
                playSound(1);
                update();
            }
            break;
        }
        case 4:
        {
            // Fix 14/15: block interaction with greyed rows
            if (sidebarIndex == kIdxSystem)
            {
                EmuInstance* emu_ = m_mainWindow->getEmuInstance();
                int ct = emu_ ? emu_->getGlobalConfig().GetInt("Emu.ConsoleType") : 0;
                if ((detailIndex == 2 && ct == 1) || (detailIndex == 3 && ct == 0) ||
                    (detailIndex == 4 && ct == 0)) break;
            }
            if (sidebarIndex == kIdxSound && detailIndex == 3)
            {
                EmuInstance* emu_ = m_mainWindow->getEmuInstance();
                int ct = emu_ ? emu_->getGlobalConfig().GetInt("Emu.ConsoleType") : 0;
                if (ct == 0) break;
            }
            if (row.type == SettingRow::Type::Toggle)
            {
                int cur = readRowValue(sidebarIndex, detailIndex);
                writeRowValue(sidebarIndex, detailIndex, cur ? 0 : 1);
                playSound(4);
                update();
            }
            else if (row.type == SettingRow::Type::Combobox)
            {
                if (sidebarIndex == kIdxSystem && detailIndex == 1 &&
                    readRowValue(kIdxSystem, 0) == 0) break;
                if (sidebarIndex == kIdxSystem && detailIndex == 1)
                    populateWifiAdapters();
                if (sidebarIndex == kIdxSound && detailIndex == 5)
                    populateAudioPacks();
                m_optionScrollOffset = 0;
                optionIndex = readRowValue(sidebarIndex, detailIndex);
                currentScreen = Screen::OptionList;
                playSound(1);
                update();
            }
            else if (row.type == SettingRow::Type::Navigate && sidebarIndex == kIdxKeyboard)
            {
                m_remapIsJoystick   = false;
                m_remapScrollOffset = 0;
                detailIndex         = 0;
                buildRemapList();
                currentScreen = Screen::Remap;
                playSound(1);
                update();
            }
            break;
        }
        case 5:
            currentScreen = Screen::Sidebar;
            playSound(3);
            update();
            break;
        case 6: // Fix 13: X — Reset section to defaults
            if (sidebarIndex != kIdxStream && sidebarIndex != kIdxQuit &&
                sidebarIndex != kIdxGamepad && sidebarIndex != kIdxKeyboard)
            {
                // Suppress for Game section when no KH ROM is loaded
                if (sidebarIndex == kIdxGame)
                {
                    EmuInstance* emu_ = m_mainWindow->getEmuInstance();
                    bool khLoaded = emu_ && emu_->plugin &&
                        (PluginKingdomHeartsDays::isCart(emu_->plugin->getGameCode()) ||
                         PluginKingdomHeartsReCoded::isCart(emu_->plugin->getGameCode()));
                    if (!khLoaded) break;
                }
                m_resettingSection      = true;
                m_resettingConfirmIndex = 1;
                playSound(1);
                update();
            }
            break;
        default: break;
        }
        return;
    }

    // ── Option List ────────────────────────────────────────────────────────────
    if (currentScreen == Screen::OptionList)
    {
        QStringList opts = currentOptionList();
        int optCount = opts.size();
        if (optCount == 0) { currentScreen = Screen::Detail; update(); return; }

        auto scrollOptionToIdx = [&](int idx) {
            int dX_, dW_, sY_, bH_, sp_;
            computeDetailGeometry(dX_, dW_, sY_, bH_, sp_);
            const int h_ = height();
            const qreal topLineY_    = (int)(h_ * 0.11) + (int)(h_ * 0.045) / 2.0;
            const qreal bottomLineY_ = h_ - topLineY_;
            int labelH_ = qMax(10, (int)(bH_ * 0.45));
            int intraGap_ = qMax(2, (int)(bH_ * 0.10));
            int optStartY_ = sY_ + labelH_ + intraGap_;
            int visH = (int)(bottomLineY_) - optStartY_ - (int)(h_ * 0.04);
            int optY = idx * (bH_ + sp_);
            if (optY < m_optionScrollOffset)
                m_optionScrollOffset = optY;
            else if (optY + bH_ + sp_ > m_optionScrollOffset + visH)
                m_optionScrollOffset = optY + bH_ + sp_ - visH;
            m_optionScrollOffset = qMax(0, m_optionScrollOffset);
        };

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
        return;
    }

    // ── Remap ──────────────────────────────────────────────────────────────────
    if (currentScreen == Screen::Remap)
    {
        int actionCount = m_remapActionToItemIdx.size();
        if (actionCount == 0)
        {
            if (direction == 5) { currentScreen = Screen::Sidebar; update(); }
            return;
        }

        auto scrollRemapToAction = [&](int actionIdx) {
            if (actionIdx < 0 || actionIdx >= m_remapActionToItemIdx.size()) return;
            int itemIdx = m_remapActionToItemIdx[actionIdx];
            int dX, dW, sY, bH, sp;
            computeDetailGeometry(dX, dW, sY, bH, sp);
            int stride   = bH + sp;
            const int h  = height();
            const qreal topLineY    = (int)(h * 0.11) + (int)(h * 0.045) / 2.0;
            const qreal bottomLineY = h - topLineY;
            int visibleH = (int)(bottomLineY) - sY - bH * 2;
            int itemY = itemIdx * stride;
            if (itemY < m_remapScrollOffset)
                m_remapScrollOffset = itemY;
            else if (itemY + stride > m_remapScrollOffset + visibleH)
                m_remapScrollOffset = itemY + stride - visibleH;
            m_remapScrollOffset = qMax(0, m_remapScrollOffset);
        };

        switch (direction)
        {
        case 0:
            if (detailIndex == 0)
            {
                int dX_, dW_, sY_, bH_, sp_;
                computeDetailGeometry(dX_, dW_, sY_, bH_, sp_);
                const int h_ = height();
                const qreal botY_ = h_ - ((int)(h_ * 0.11) + (int)(h_ * 0.045) / 2.0);
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
            m_bindTarget     = detailIndex;
            m_waitingForBind = true;
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
        case 6: // Y — Reset All
            m_resettingBindings     = true;
            m_resettingConfirmIndex = 1;
            playSound(1);
            update();
            break;
        default: break;
        }
    }
}

// ─── key events ───────────────────────────────────────────────────────────────

void SettingsView::keyPressEvent(QKeyEvent* event)
{
    if (m_waitingForBind && !m_remapIsJoystick)
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
    default: event->accept(); break;
    }
}

// ─── mouse events ─────────────────────────────────────────────────────────────

void SettingsView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();

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
    const qreal topLineY_    = (int)(h_ * 0.11) + (int)(h_ * 0.045) / 2.0;
    const qreal bottomLineY_ = h_ - topLineY_;
    int delta = -event->angleDelta().y() / 4;

    if (currentScreen == Screen::Detail)
    {
        QVector<SettingRow> rows = rowsFor(sidebarIndex);
        int labelH_   = qMax(10, (int)(bH_ * 0.45));
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
        int labelH_    = qMax(10, (int)(bH_ * 0.45));
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
        if (pos.x() < width() / 2) handleNavigation(2);
        else                        handleNavigation(3);
        handleNavigation(4);
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
            sidebarIndex = hit;
            detailIndex  = 0;
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
                // Compute track geometry matching paintDetailArea
                int dX, dW, sY, bH, sp;
                computeDetailGeometry(dX, dW, sY, bH, sp);
                int labelH   = qMax(10, (int)(bH * 0.45));
                int intraGap = qMax(2,  (int)(bH * 0.10));
                int rowPillW_ = (int)(dW * 0.56);
                int numPad_  = (int)(bH * 0.75);
                int trackX   = dX + numPad_ + (int)(rowPillW_ * 0.28) + (int)(bH * 0.15);
                int trackEndX = dX + rowPillW_ - (int)(bH * 0.4);
                int trackW   = qMax(1, trackEndX - trackX);
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
                                    QRect r, bool selected, bool dimUnselected)
{
    QColor themeColor = currentThemeColor();
    const qreal radius = r.height() / 2.0;
    QColor textColor;

    if (selected)
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
        textColor = dimUnselected ? QColor(88, 88, 92) : QColor(180, 180, 185);
    }

    p.setBrush(Qt::NoBrush);
    return textColor;
}

void SettingsView::paintPillButton(QPainter& p, QRect r, const QString& label,
                                    bool selected, bool /*highlighted*/,
                                    QColor swatchColor, bool dimUnselected)
{
    const qreal radius = r.height() / 2.0;
    QPainterPath path;
    addKHPillPath(path, QRectF(r), radius);

    QColor textColor = paintPillFill(p, path, r, selected, dimUnselected);

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
    const int w    = width();
    const int h    = height();
    const int barH = (int)(h * 0.045);
    const int barY = (int)(h * 0.11);
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
               Qt::AlignVCenter | Qt::AlignLeft, "GAME SETTINGS");

    const qreal lineY = barY + barH / 2.0;
    QColor lineColor = themeColor;
    lineColor.setAlpha(200);
    p.setPen(QPen(lineColor, 1));
    p.drawLine(QPointF(barW, lineY), QPointF(w, lineY));
}

void SettingsView::paintBottomLine(QPainter& p)
{
    const int w = width();
    const int h = height();
    const qreal topLineY    = (int)(h * 0.11) + (int)(h * 0.045) / 2.0;
    const qreal bottomLineY = h - topLineY;
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

    const int   barY_d   = (int)(h * 0.11);
    const int   barH_d   = (int)(h * 0.045);
    const int   circleR  = 3;
    const int   divGap   = (int)(h * 0.042);
    const int   divStart = barY_d + barH_d + divGap;
    const qreal topLineY = barY_d + barH_d / 2.0;
    const int   divEnd   = (int)(h - topLineY) - divGap;

    QFont font("KHMenu");
    font.setPixelSize(qMax(11, (int)(btnH * 0.50)));
    p.setFont(font);

    // Fix 1: indent is always active for the selected item
    const int activeIndent = (int)(w * 0.025);
    // Fix 3: dim unselected items once a section is entered
    bool dimUnselected = (currentScreen != Screen::Sidebar);

    for (int i = 0; i < kSidebarCount; i++)
    {
        int y = startY + i * (btnH + spacing);
        bool selected = (i == sidebarIndex);
        int indent = selected ? activeIndent : 0;
        int pillLeft  = sidebarX + indent;
        int pillRight = sidebarX + sidebarW + (selected ? activeIndent : 0);
        QRect btnRect(pillLeft, y, pillRight - pillLeft, btnH);
        paintPillButton(p, btnRect, QString::fromUtf8(kSidebarLabels[i]), selected, false,
                        QColor(), dimUnselected);

        // Fix 2: orb only visible in Sidebar screen on selected item
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

void SettingsView::paintDetailArea(QPainter& p)
{
    const int w = width();
    const int h = height();

    int detailX, detailW, startY, btnH, spacing;
    computeDetailGeometry(detailX, detailW, startY, btnH, spacing);

    QColor themeColor = currentThemeColor();

    const qreal topLineY    = (int)(h * 0.11) + (int)(h * 0.045) / 2.0;
    const qreal bottomLineY = h - topLineY;

    QFont rowFont("KHMenu");
    rowFont.setPixelSize(qMax(11, (int)(btnH * 0.48)));

    // Fix 9 geometry
    int rowPillW  = (int)(detailW * 0.56);
    int optGapW   = (int)(detailW * 0.06);
    int optPanelX = detailX + rowPillW + optGapW;
    int optPanelW = detailW - rowPillW - optGapW;

    int scrollbarW = 4;

    // Fix 11 — overview text when in Sidebar screen
    if (currentScreen == Screen::Sidebar)
    {
        QFont overviewFont("KHMenu");
        overviewFont.setPixelSize(qMax(10, (int)(btnH * 0.42)));
        p.setFont(overviewFont);
        p.setPen(QColor(180, 180, 185));
        p.drawText(QRect(detailX, startY, detailW, (int)(h * 0.30)),
                   Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                   kSectionOverview[sidebarIndex]);
        return;
    }

    // ── Remap screen ──────────────────────────────────────────────────────────
    if (currentScreen == Screen::Remap)
    {
        int remapPillW = rowPillW;
        int stride     = btnH + spacing;
        int visibleH   = (int)(bottomLineY) - startY - btnH * 3 - spacing;

        p.setFont(rowFont);

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
                hdrFont.setPixelSize(qMax(9, (int)(btnH * 0.36)));
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
                    bindStr = m_remapIsJoystick ? joyBindingText(v) : keyBindingText(v);
                }

                QPainterPath rp;
                addRoundedPillPath(rp, QRectF(detailX, itemY, remapPillW, btnH));
                QRect remapPillRect(detailX, itemY, remapPillW, btnH);
                paintPillFill(p, rp, remapPillRect, focused, false);

                int pad = (int)(btnH * 0.45);
                p.setPen(focused ? QColor(235, 235, 240) : QColor(180, 180, 185));
                p.drawText(QRect(detailX + pad, itemY, (int)(remapPillW * 0.56), btnH),
                           Qt::AlignVCenter | Qt::AlignLeft, item.label);
                p.setPen(focused ? QColor(200, 200, 210) : QColor(120, 120, 125));
                p.drawText(QRect(detailX + (int)(remapPillW * 0.56), itemY,
                                 (int)(remapPillW * 0.40), btnH),
                           Qt::AlignVCenter | Qt::AlignRight, bindStr);

                actionIdx++;
            }
        }

        // Orb on focused remap pill
        if (focusedItemY >= 0)
        {
            qreal capR  = btnH / 2.0;
            qreal capCx = detailX + remapPillW - capR;
            qreal capCy = focusedItemY + btnH / 2.0;
            paintOrbAt(p, capCx, capCy, capR);
        }

        // Scrollbar just right of remap pills
        {
            int sbX    = detailX + remapPillW + (int)(detailW * 0.012);
            int totalH = m_remapItems.size() * (btnH + spacing);
            paintScrollbar(p, sbX, startY, scrollbarW, visibleH,
                           m_remapScrollOffset, totalH, visibleH);
        }

        // Instructional text above hint footer
        QFont instrFont("KHMenu");
        instrFont.setPixelSize(qMax(9, (int)(btnH * 0.40)));
        p.setFont(instrFont);
        p.setPen(QColor(180, 180, 185));
        p.drawText(QRect(detailX, (int)(bottomLineY - btnH * 2 - spacing), detailW, btnH),
                   Qt::AlignVCenter | Qt::AlignLeft, "Select an action to assign.");

        QFont hintFont("KHMenu");
        hintFont.setPixelSize(qMax(9, (int)(btnH * 0.35)));
        p.setFont(hintFont);
        p.setPen(QColor(120, 120, 125));
        p.drawText(QRect(detailX, (int)(bottomLineY - btnH - spacing), detailW, btnH),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   "X Reset All  |  B Back to Sidebar");
        return;
    }

    // ── Before You Stream ─────────────────────────────────────────────────────
    if (sidebarIndex == kIdxStream)
    {
        const int margin     = (int)(detailW * 0.04);  // outer margin: title, lines
        const int pillMargin = (int)(detailW * 0.13);  // inner margin: body text, URL pill

        QFont titleFont("KHGummi");
        titleFont.setPixelSize(qMax(12, (int)(btnH * 0.55)));
        p.setFont(titleFont);
        p.setPen(themeColor);
        p.drawText(QRect(detailX + margin, startY, detailW - margin * 2, btnH),
                   Qt::AlignVCenter | Qt::AlignCenter,
                   "Before You Stream\xE2\x80\xA6");

        // Top gradient line — tied to title text width
        int line1Y = startY + btnH + spacing / 2;
        {
            int titleW = QFontMetrics(titleFont).horizontalAdvance("Before You Stream\xE2\x80\xA6");
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

        const QString para1 =
            "This game is a copyrighted work. The copyright is held by The Walt Disney "
            "Company and a collaboration of authors representing The Walt Disney Company. "
            "Additionally, the copyright of certain characters is held by Square Enix Co., Ltd.";
        const QString para2 =
            "You are free to stream this game in non-commercial contexts. However, using "
            "streams of the game to primarily provide or listen to the music is prohibited "
            "even in such non-commercial contexts.";
        const QString para3 =
            "For information on the terms of use relating to streaming the game, please see "
            "the official KINGDOM HEARTS site.";

        p.setFont(rowFont);
        p.setPen(QColor(235, 235, 240));

        int textY = line1Y + spacing;
        int paraH = (int)(h * 0.13);
        for (const QString& para : {para1, para2, para3})
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
        paintPillFill(p, urlPath, urlPillRect, false, false);
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
                   "Open a browser to view the official website.");
        return;
    }

    // ── Quit Game ─────────────────────────────────────────────────────────────
    if (sidebarIndex == kIdxQuit)
    {
        const int margin = (int)(detailW * 0.07);
        QFont titleFont("KHGummi");
        titleFont.setPixelSize(qMax(12, (int)(btnH * 0.55)));
        p.setFont(titleFont);
        p.setPen(themeColor);
        p.drawText(QRect(detailX + margin, startY, detailW - margin * 2, btnH),
                   Qt::AlignVCenter | Qt::AlignCenter, "Quit Game");
        int lineY = startY + btnH + spacing / 2;
        {
            int titleW = QFontMetrics(titleFont).horizontalAdvance("Quit Game");
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
        bodyFont.setPixelSize(qMax(9, (int)(btnH * 0.42)));
        p.setFont(bodyFont);
        p.setPen(QColor(180, 180, 185));
        p.drawText(QRect(detailX + margin, lineY + spacing, detailW - margin * 2, (int)(h * 0.35)),
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                   "You are about to quit the current game. "
                   "Any progress since your last save will be lost.");
        return;
    }

    // ── Game placeholder (no KH game running) ─────────────────────────────────
    if (sidebarIndex == kIdxGame)
    {
        EmuInstance* emu = m_mainWindow->getEmuInstance();
        bool khLoaded = emu && emu->plugin &&
                        (PluginKingdomHeartsDays::isCart(emu->plugin->getGameCode()) ||
                         PluginKingdomHeartsReCoded::isCart(emu->plugin->getGameCode()));
        if (!khLoaded)
        {
            QColor tc = currentThemeColor();
            const int margin     = (int)(detailW * 0.04);
            const int pillMargin = (int)(detailW * 0.13);

            QFont titleFont("KHGummi");
            titleFont.setPixelSize(qMax(12, (int)(btnH * 0.55)));

            QString titleText = "No Kingdom Hearts Game Loaded";
            int titleW = QFontMetrics(titleFont).horizontalAdvance(titleText);
            if (titleW > detailW - margin * 2)
            {
                titleText = "No Game Loaded";
                titleW = QFontMetrics(titleFont).horizontalAdvance(titleText);
            }

            p.setFont(titleFont);
            p.setPen(tc);
            p.drawText(QRect(detailX + margin, startY, detailW - margin * 2, btnH),
                       Qt::AlignVCenter | Qt::AlignCenter, titleText);

            int lineY = startY + btnH + spacing / 2;
            {
                int pad   = (int)(detailW * 0.04);
                int lLeft  = detailX + (detailW - titleW) / 2 - pad;
                int lRight = lLeft + titleW + 2 * pad;
                QColor lc = tc; lc.setAlpha(180);
                QLinearGradient g(lLeft, 0, lRight, 0);
                g.setColorAt(0.0,  Qt::transparent);
                g.setColorAt(0.06, lc);
                g.setColorAt(0.94, lc);
                g.setColorAt(1.0,  Qt::transparent);
                p.setPen(QPen(QBrush(g), 1));
                p.drawLine(lLeft, lineY, lRight, lineY);
            }

            QFont noGameBodyFont("KHMenu");
            noGameBodyFont.setPixelSize(qMax(9, (int)(btnH * 0.42)));
            p.setFont(noGameBodyFont);
            p.setPen(QColor(180, 180, 185));
            p.drawText(QRect(detailX + pillMargin, lineY + spacing, detailW - pillMargin * 2, (int)(h * 0.30)),
                       Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                       "No Kingdom Hearts game is currently running.\n"
                       "Open a ROM from File \xE2\x80\xBA Open ROM to get started.");
            return;
        }
    }

    // ── Row layout constants ───────────────────────────────────────────────────
    int labelH   = qMax(10, (int)(btnH * 0.45));
    int intraGap = qMax(2,  (int)(btnH * 0.10));
    int stride   = labelH + intraGap + btnH + spacing;

    QFont labelFont("KHMenu");
    labelFont.setPixelSize(qMax(9, (int)(btnH * 0.42)));

    // Greying helpers
    bool wifiDirect = (sidebarIndex == kIdxSystem) ? (readRowValue(kIdxSystem, 0) == 1) : true;
    int consoleType = 0;
    if (sidebarIndex == kIdxSystem || sidebarIndex == kIdxSound)
    {
        EmuInstance* emu = m_mainWindow->getEmuInstance();
        if (emu) consoleType = emu->getGlobalConfig().GetInt("Emu.ConsoleType");
    }

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

        // Fix 14: DS/DSi greying in System
        bool greyed = (sidebarIndex == kIdxSystem && i == 1 && !wifiDirect);
        if (sidebarIndex == kIdxSystem)
        {
            if (i == 2 && consoleType == 1) greyed = true; // DS Battery in DSi mode
            if (i == 3 && consoleType == 0) greyed = true; // DSi Battery in DS mode
            if (i == 4 && consoleType == 0) greyed = true; // DSi Charging in DS mode
        }
        // Fix 15: DSi Volume Sync greying
        if (sidebarIndex == kIdxSound && i == 3 && consoleType == 0) greyed = true;

        if (greyed) p.setOpacity(0.35);

        // Label above pill (dimmed when OptionList is open for another row)
        p.setFont(labelFont);
        p.setPen(inOptionList && i != detailIndex ? QColor(70, 65, 55) : themeColor);
        p.drawText(QRect(detailX, labelY, rowPillW, labelH),
                   Qt::AlignVCenter | Qt::AlignLeft, row.label);

        // Value in pill
        int val = readRowValue(sidebarIndex, i);
        QString valStr;
        if (row.type == SettingRow::Type::Toggle)
            valStr = val ? "On" : "Off";
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

        // Fix 6: rounded pills; arrow pill for open row
        QRect pillRect(detailX, pillY, rowPillW, btnH);
        bool isOpenRow = inOptionList && i == detailIndex;
        // Fix 7: dim non-active rows during OptionList
        bool dimRow = inOptionList && i != detailIndex;

        if (row.type == SettingRow::Type::Slider)
        {
            QPainterPath sp2;
            addRoundedPillPath(sp2, QRectF(pillRect));
            QColor textCol = paintPillFill(p, sp2, pillRect, focused, dimRow);
            p.setFont(rowFont);

            int numPad   = (int)(btnH * 0.75);
            p.setPen(textCol);
            p.drawText(QRect(detailX + numPad, pillY, (int)(rowPillW * 0.28), btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, valStr);

            int trackX    = detailX + numPad + (int)(rowPillW * 0.28) + (int)(btnH * 0.15);
            int trackEndX = detailX + rowPillW - (int)(btnH * 0.4);
            int trackW    = qMax(1, trackEndX - trackX);
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
            int handleW_ = qMax(3, (int)(btnH * 0.10));
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
            QColor textCol = paintPillFill(p, pp2, pillRect, focused, dimRow);
            p.setFont(rowFont);
            int padding = (int)(btnH * 0.5);
            p.setPen(textCol);
            p.drawText(QRect(detailX + padding, pillY, rowPillW - padding * 2, btnH),
                       Qt::AlignVCenter | Qt::AlignLeft, valStr);
        }

        if (greyed) p.setOpacity(1.0);
    }

    // Fix 2: orb on focused detail row pill (only in Detail screen, only when rows non-empty)
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

    // ── Option list panel ─────────────────────────────────────────────────────
    if (inOptionList)
    {
        QStringList opts = currentOptionList();
        int savedVal     = readRowValue(sidebarIndex, detailIndex);
        bool isThemeColor = (detailIndex >= 0 && detailIndex < rows.size() &&
                             rows[detailIndex].label == "Theme Color");

        int labelH2   = qMax(10, (int)(btnH * 0.45));
        int intraGap2 = qMax(2,  (int)(btnH * 0.10));
        int optStartY = startY + labelH2 + intraGap2;
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
            QColor textCol = paintPillFill(p, op, r, foc, false);
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

        // Fix 2: orb on focused option pill
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

    // Fix 10: description footer — centered
    if (currentScreen == Screen::Detail && detailIndex >= 0 && detailIndex < rows.size())
    {
        int descY = (int)(bottomLineY - (int)(h * 0.08));
        QFont descFont("KHMenu");
        descFont.setPixelSize(qMax(9, (int)(btnH * 0.38)));
        p.setFont(descFont);
        p.setPen(QColor(180, 180, 185));
        p.drawText(QRect(detailX, descY, detailW, (int)(h * 0.06)),
                   Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                   rows[detailIndex].description);
    }
}

void SettingsView::paintActionBar(QPainter& p)
{
    const int w = width();
    const int h = height();
    const qreal topLineY    = (int)(h * 0.11) + (int)(h * 0.045) / 2.0;
    const qreal bottomLineY = h - topLineY;
    const int barY = (int)(bottomLineY + (int)(h * 0.010));

    QColor themeColor = currentThemeColor();

    const char* hintGlyphs[8] = {};
    const char* hintLabels[8] = {};
    int hintCount = 0;
    auto pushHint = [&](const char* g, const char* l) {
        if (hintCount < 8) { hintGlyphs[hintCount] = g; hintLabels[hintCount] = l; hintCount++; }
    };

    if (m_waitingForBind)
    {
        pushHint("B", "Cancel");
    }
    else if (m_resettingBindings || m_resettingSection)
    {
        pushHint("A", "Yes");
        pushHint("B", "No");
    }
    else if (currentScreen == Screen::Sidebar)
    {
        pushHint("A", "Enter");
        pushHint("B", "Close");
    }
    else if (currentScreen == Screen::OptionList)
    {
        pushHint("A", "Confirm");
        pushHint("B", "Cancel");
    }
    else if (currentScreen == Screen::Remap)
    {
        pushHint("A", "Rebind");
        pushHint("X", "Reset All");
        pushHint("B", "Back");
    }
    else if (currentScreen == Screen::Detail)
    {
        if (sidebarIndex == kIdxStream)
        {
            pushHint("A", "Open Website");
            pushHint("B", "Back");
        }
        else if (sidebarIndex == kIdxQuit)
        {
            pushHint("A", "Quit Game");
            pushHint("B", "Back");
        }
        else
        {
            QVector<SettingRow> rows = rowsFor(sidebarIndex);
            if (!rows.isEmpty() && detailIndex >= 0 && detailIndex < rows.size())
            {
                const SettingRow& row = rows[detailIndex];
                if (row.type == SettingRow::Type::Toggle)
                    pushHint("A", "Toggle");
                else if (row.type == SettingRow::Type::Combobox)
                    pushHint("A", "Select");
                else if (row.type == SettingRow::Type::Slider)
                    pushHint("\xE2\x97\x80 \xE2\x96\xB6", "Adjust");
            }
            if (sidebarIndex != kIdxStream && sidebarIndex != kIdxQuit &&
                sidebarIndex != kIdxGamepad && sidebarIndex != kIdxKeyboard)
                pushHint("X", "Reset");
            pushHint("B", "Back");
        }
    }

    if (hintCount == 0) return;

    const int iconH   = qMax(14, (int)(h * 0.032));
    const int iconW   = iconH;
    const int gap     = (int)(w * 0.04);
    const int labelSz = qMax(10, (int)(h * 0.018));

    QFont iconFont("KHMenu");
    iconFont.setPixelSize(qMax(9, (int)(iconH * 0.62)));
    QFont labelFont("KHMenu");
    labelFont.setPixelSize(labelSz);

    p.setFont(labelFont);
    int totalW = 0;
    for (int i = 0; i < hintCount; i++)
    {
        totalW += iconW + (int)(iconH * 0.3);
        totalW += p.fontMetrics().horizontalAdvance(hintLabels[i]);
        totalW += gap;
    }
    totalW -= gap;

    int x = (w - totalW) / 2;
    for (int i = 0; i < hintCount; i++)
    {
        QRect iconRect(x, barY, iconW, iconH);
        QColor boxFill(30, 30, 30, 220);
        QColor boxBorder = themeColor.darker(150);
        boxBorder.setAlpha(128);
        p.setBrush(boxFill);
        p.setPen(QPen(boxBorder, 1.5));
        p.drawRoundedRect(iconRect, 3, 3);
        p.setFont(iconFont);
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
    const int w = width();
    const int h = height();
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 160));

    const int boxW = (int)(w * 0.50);
    const int boxH = (int)(h * 0.28);
    const QRect box((w - boxW) / 2, (h - boxH) / 2, boxW, boxH);
    p.fillRect(box, QColor(20, 20, 30, 220));
    p.setPen(QPen(QColor(200, 200, 200), 2));
    p.drawRect(box);

    QFont titleFont("KHMenu");
    titleFont.setPixelSize(qMax(13, (int)(boxH * 0.22)));
    p.setFont(titleFont);
    p.setPen(Qt::white);
    QString confirmText = m_resettingSection
        ? QString("Reset %1 to defaults?").arg(kSidebarLabels[sidebarIndex])
        : "Reset all bindings?";
    p.drawText(QRect(box.x(), box.y() + (int)(boxH * 0.10), boxW, (int)(boxH * 0.35)),
               Qt::AlignCenter, confirmText);

    const int btnH = (int)qMax(24.0, boxH * 0.22);
    const int btnW = (int)(boxW * 0.32);
    const int btnY = box.y() + (int)(boxH * 0.58);
    const int btnGap = (int)(boxW * 0.06);
    const int startX = box.x() + (boxW - btnW * 2 - btnGap) / 2;

    QFont btnFont("KHMenu");
    btnFont.setPixelSize(qMax(12, (int)(btnH * 0.50)));
    p.setFont(btnFont);
    paintPillButton(p, QRect(startX, btnY, btnW, btnH),
                    "A Yes", m_resettingConfirmIndex == 0, false);
    paintPillButton(p, QRect(startX + btnW + btnGap, btnY, btnW, btnH),
                    "B No", m_resettingConfirmIndex == 1, false);
}

void SettingsView::paintCaptureOverlay(QPainter& p)
{
    const int w = width();
    const int h = height();
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 180));

    QFont font("KHMenu");
    font.setPixelSize(qMax(14, (int)(h * 0.028)));
    p.setFont(font);
    p.setPen(Qt::white);
    QString prompt = m_remapIsJoystick ? "Press a controller button\xE2\x80\xA6"
                                       : "Press a key\xE2\x80\xA6";
    p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, prompt);

    QFont subFont("KHMenu");
    subFont.setPixelSize(qMax(10, (int)(h * 0.018)));
    p.setFont(subFont);
    p.setPen(QColor(0xA0, 0x98, 0x80));
    p.drawText(QRect(0, h / 2 + (int)(h * 0.04), w, (int)(h * 0.04)),
               Qt::AlignCenter, "Press B to cancel");
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
