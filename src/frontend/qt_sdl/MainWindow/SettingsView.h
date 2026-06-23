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

#include <QWidget>
#include <QElapsedTimer>
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>
#include <QShowEvent>
#include <QHideEvent>

class MainWindowSettings;

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
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    enum class Screen { Sidebar, Detail, OptionList };
    Screen currentScreen = Screen::Sidebar;
    int sidebarIndex = 0;
    int detailIndex = 0;
    int optionIndex = 0;

    bool m_confirmingQuit = false;
    int m_confirmIndex = 0; // 0=Yes, 1=No

    QElapsedTimer m_animClock;
    QTimer* m_animTimer = nullptr;
    QTimer* m_joyTimer = nullptr;
    qint64 m_lastJoyInput = 0;

    QPixmap m_background;

    MainWindowSettings* m_mainWindow;

    void paintBackground(QPainter& p);
    void paintTitleBar(QPainter& p);
    void paintSidebar(QPainter& p);
    void paintDetailArea(QPainter& p);
    void paintPillButton(QPainter& p, QRect r, const QString& label, bool selected, bool highlighted);
    void paintQuitConfirmation(QPainter& p);
    void handleNavigation(int direction);
    void playSound(int kind);
    void pollJoystick();
};

#endif // SETTINGSVIEW_H
