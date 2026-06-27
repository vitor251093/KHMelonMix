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
};

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
    bool m_resettingBindings     = false;
    bool m_resettingSection      = false;
    int  m_resettingConfirmIndex = 1;
    Sint16 m_axesRest[16]        = {};

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

    // Dynamic option lists (m_audioPackNames populated lazily in const context)
    QStringList         m_wifiAdapters;
    mutable QStringList m_audioPackNames;

    QElapsedTimer m_animClock;
    QTimer* m_animTimer  = nullptr;
    QTimer* m_joyTimer   = nullptr;
    qint64  m_lastJoyInput = 0;

    QPixmap m_background;
    QPixmap m_gearPixmap;

    MainWindowSettings* m_mainWindow;

    QColor currentThemeColor() const;
    const SettingsLocale& locale() const;
    void scrollOptionToIdx(int idx);

    void paintBackground(QPainter& p);
    void paintTitleBar(QPainter& p);
    void paintBottomLine(QPainter& p);
    void paintSidebar(QPainter& p);
    void paintDetailArea(QPainter& p);
    void paintActionBar(QPainter& p);
    QColor paintPillFill(QPainter& p, const QPainterPath& path,
                         QRect r, bool selected, bool dimUnselected);
    void paintPillButton(QPainter& p, QRect r, const QString& label,
                         bool selected, QColor swatchColor = QColor(),
                         bool dimUnselected = false);
    void paintOrbAt(QPainter& p, qreal capCx, qreal capCy, qreal capR);
    void paintScrollbar(QPainter& p, int x, int y, int w, int h,
                        int offset, int totalH, int visibleH);
    void paintResetConfirmation(QPainter& p);
    void paintCaptureOverlay(QPainter& p);
    void confirmPopupRects(QRect& box, QRect& yesRect, QRect& noRect) const;

    void computeSidebarGeometry(int& sidebarX, int& sidebarW,
                                int& startY, int& btnH, int& spacing,
                                int& divX) const;
    void computeDetailGeometry(int& detailX, int& detailW,
                               int& startY, int& btnH, int& spacing) const;

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
    void playSound(int kind);
    void pollJoystick();
};

#endif // SETTINGSVIEW_H
