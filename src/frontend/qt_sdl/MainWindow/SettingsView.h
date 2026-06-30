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

#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <functional>
#include <string>
#include <QWidget>
#include "SettingsLocale.h"
#include <QVector>
#include <QStringList>
#include <QElapsedTimer>
#include <QTimer>
#include <QPixmap>
#include <QColor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QHideEvent>

class MainWindowSettings;

struct SettingRow
{
    enum class Type { Toggle, Combobox, Slider, Readonly, Navigate };
    enum class Tag  { None, ThemeColorPicker };
    Type        type;
    QString     label;
    QString     description;
    QStringList options;
    int         sliderMin = 0;
    int         sliderMax = 0;
    Tag         tag  = Tag::None;
    bool        isDynamic = false;
    std::function<bool()>    enabled;
    std::function<int()>     read;
    std::function<void(int)> write;
    // Restores this row to its default and applies the same side effect as write(). Set only on
    // rows that participate in "Reset section"; resetSectionToDefaults() iterates and calls it.
    std::function<void()>    reset;
    // Dynamic rows (isDynamic) supply their option list and the text shown in the pill through
    // these, so the dispatchers don't have to special-case each category by index.
    std::function<QStringList()> dynamicOptions;
    std::function<QString()>     dynamicValueText;
};

// Visual state of a "pill" button/row. Replaces the old (bool selected, bool dimUnselected) pair
// so call sites can't transpose the two flags.
enum class PillState { Normal, Selected, Dimmed };

struct RemapItem
{
    bool        isHeader  = false;
    QString     label;
    std::string configKey; // empty for headers
};

class SettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsView(MainWindowSettings* mainWindow);
    void resetToFirstScreen();

signals:
    void settingsClosed();
    void quitGameConfirmed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    enum class Screen { Sidebar, Detail, OptionList, Remap };
    Screen currentScreen = Screen::Sidebar;
    int sidebarIndex = 0;
    int detailIndex  = 0;
    int optionIndex  = 0;

    bool m_pendingClose          = false;
    bool m_waitingForBind        = false;
    int  m_bindTarget            = 0;
    bool m_bindArmed             = false;
    qint64 m_bindListenStart     = 0;
    qint64 m_bindArmTime         = 0;
    bool m_resettingBindings     = false;
    bool m_resettingSection      = false;
    int  m_resettingConfirmIndex = 1;
    Sint16 m_axesRest[16]        = {};

    // Controller bind listen timeouts (ms): cancel if the pad never reaches rest,
    // or if no input arrives once armed. Keeps every button bindable (no reserved cancel button).
    static constexpr qint64 kBindPreInputTimeoutMs = 5000;
    static constexpr qint64 kBindAssignTimeoutMs   = 5000;

    // Detail + remap + option scroll state
    int  m_detailScrollOffset = 0;
    int  m_remapScrollOffset  = 0;
    int  m_optionScrollOffset = 0;
    bool m_remapIsJoystick   = false;
    QVector<RemapItem> m_remapItems;
    QVector<int>       m_remapActionToItemIdx;

    // Slider drag state
    bool m_draggingSlider = false;
    int  m_dragSidebarIdx = 0;
    int  m_dragRowIdx     = 0;
    int  m_dragTrackX     = 0;
    int  m_dragTrackW     = 1;
    int  m_dragSliderMin  = 0;
    int  m_dragSliderMax  = 1;

    // Dynamic option lists, (re)populated by populateWifiAdapters() / populateAudioPacks()
    // when their option panel is opened.
    QStringList m_wifiAdapters;
    QStringList m_audioPackNames;

    QElapsedTimer m_animClock;
    QTimer* m_animTimer  = nullptr;
    QTimer* m_joyTimer   = nullptr;
    qint64  m_lastJoyInput = 0;

    QPixmap m_background;
    QPixmap m_gearPixmap;

    MainWindowSettings* m_mainWindow;

    struct DetailLayout {
        int h;
        int detailX, detailW;
        int startY, btnH, spacing;
        qreal topLineY, bottomLineY;
        QFont rowFont;
        QColor themeColor;
        int rowPillW, optGapW, optPanelX, optPanelW;
        int scrollbarW = 4;
    };
    DetailLayout computeDetailLayout() const;

    QColor currentThemeColor() const;
    const SettingsLocale& locale() const;
    void scrollOptionToIdx(int idx);

    void paintBackground(QPainter& p);
    void paintTitleBar(QPainter& p);
    void paintBottomLine(QPainter& p);
    void paintSidebar(QPainter& p);
    void paintDetailArea(QPainter& p);
    void paintDetailSidebarPreview(QPainter& p, const DetailLayout& L);
    void paintDetailRemap(QPainter& p, const DetailLayout& L);
    void paintDetailStream(QPainter& p, const DetailLayout& L);
    void paintDetailQuit(QPainter& p, const DetailLayout& L);
    void paintDetailRows(QPainter& p, const DetailLayout& L);
    void paintOptionListPanel(QPainter& p, const DetailLayout& L);
    void paintActionBar(QPainter& p);
    QColor paintPillFill(QPainter& p, const QPainterPath& path,
                         QRect r, PillState state);
    void paintPillButton(QPainter& p, QRect r, const QString& label,
                         PillState state, QColor swatchColor = QColor());
    void paintOrbAt(QPainter& p, qreal capCx, qreal capCy, qreal capR);
    void paintScrollbar(QPainter& p, int x, int y, int w, int h,
                        int offset, int totalH, int visibleH);
    void paintResetConfirmation(QPainter& p);
    void paintCaptureOverlay(QPainter& p);
    void confirmPopupRects(QRect& box, QRect& yesRect, QRect& noRect) const;

    void computeTitleBarGeometry(int& barY, int& barH) const;
    // Y of the lower horizontal accent line that brackets the content area (the upper line sits at
    // height() - this). Derived from the title-bar geometry; used throughout paint and scroll.
    qreal contentBottomLineY() const;
    void computeSidebarGeometry(int& sidebarX, int& sidebarW,
                                int& startY, int& btnH, int& spacing,
                                int& divX) const;
    void computeDetailGeometry(int& detailX, int& detailW,
                               int& startY, int& btnH, int& spacing) const;
    // Horizontal track geometry for a Slider row's pill. Shared by the paint path and the
    // click hit-test so click-to-value mapping always matches what is drawn.
    void sliderTrackGeometry(int detailX, int rowPillW, int btnH,
                             int& trackX, int& trackW) const;

    int  hitTestSidebar(QPoint pos) const;
    int  hitTestDetail(QPoint pos) const;
    int  hitTestOptionList(QPoint pos) const;
    int  hitTestRemap(QPoint pos) const;

    QVector<SettingRow> rowsFor(int sidebarIndex) const;
    int  readRowValue(int sidebar, int row) const;
    void writeRowValue(int sidebar, int row, int newValue);
    QString rowDynamicValueText(int sidebar, int row) const;

    QStringList currentOptionList() const;
    void        populateWifiAdapters();
    void        populateAudioPacks();

    void buildRemapList();
    int  readRemapBinding(int actionIdx) const;
    void writeRemapBinding(int actionIdx, int value);
    void resetAllRemapBindings();
    void resetSectionToDefaults(int idx);

    void handleNavigation(int direction);
    void handleNavSidebar(int direction);
    void handleNavDetail(int direction);
    void handleNavOptionList(int direction);
    void handleNavRemap(int direction);
    void scrollDetailToRow(int idx, const QVector<SettingRow>& rows);
    void scrollRemapToAction(int actionIdx);
    void playSound(int kind);
    void pollJoystick();
    // The controller-rebind capture state machine (arming gate, timeouts, button/hat/axis/trigger
    // scan). Runs only while m_waitingForBind; split out of pollJoystick, which otherwise just
    // polls for menu navigation.
    void pollBindCapture(SDL_Joystick* joy);
};

#endif // SETTINGSVIEW_H
