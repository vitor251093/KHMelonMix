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

#include "SettingsView.h"
#include "MainWindowSettings.h"
#include "EmuInstance.h"
#include "plugins/Plugin.h"

using namespace Plugins;

static const char* kSidebarLabels[] = {
    "General",
    "Display",
    "Sound",
    "Gamepad",
    "Keyboard",
    "Before You Stream\xE2\x80\xA6", // ellipsis
    "Quit Game",
};
static constexpr int kSidebarCount = 7;

SettingsView::SettingsView(MainWindowSettings* mainWindow)
    : QWidget(nullptr),
      m_mainWindow(mainWindow)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    m_background = QPixmap(":/ds/settings_bg.png");

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() { update(); });

    m_joyTimer = new QTimer(this);
    m_joyTimer->setInterval(30);
    connect(m_joyTimer, &QTimer::timeout, this, &SettingsView::pollJoystick);
}

void SettingsView::resetToFirstScreen()
{
    currentScreen = Screen::Sidebar;
    sidebarIndex = 0;
    detailIndex = 0;
    optionIndex = 0;
    m_confirmingQuit = false;
    m_confirmIndex = 0;
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
}

void SettingsView::pollJoystick()
{
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    if (!emu) return;
    SDL_Joystick* joy = emu->getJoystick();
    if (!joy) return;

    qint64 now = m_animClock.elapsed();
    if (now - m_lastJoyInput < 150) return;

    Uint8 hat = SDL_JoystickGetHat(joy, 0);
    bool up    = (hat & SDL_HAT_UP)    != 0;
    bool down  = (hat & SDL_HAT_DOWN)  != 0;
    bool left  = (hat & SDL_HAT_LEFT)  != 0;
    bool right = (hat & SDL_HAT_RIGHT) != 0;

    // South button = confirm (cross/A), East button = back (circle/B)
    bool confirm = SDL_JoystickGetButton(joy, 0) != 0;
    bool back    = SDL_JoystickGetButton(joy, 1) != 0;

    int dir = -1;
    if (up)      dir = 0;
    else if (down)  dir = 1;
    else if (left)  dir = 2;
    else if (right) dir = 3;
    else if (confirm) dir = 4;
    else if (back)    dir = 5;

    if (dir < 0) return;

    m_lastJoyInput = now;
    handleNavigation(dir);
}

void SettingsView::playSound(int kind)
{
    m_mainWindow->playCutsceneMenuSound(kind);
}

void SettingsView::handleNavigation(int direction)
{
    if (m_confirmingQuit)
    {
        switch (direction)
        {
        case 2: // left
            m_confirmIndex = 0;
            update();
            break;
        case 3: // right
            m_confirmIndex = 1;
            update();
            break;
        case 4: // confirm
            if (m_confirmIndex == 0)
            {
                playSound(4);
                emit quitGameConfirmed();
            }
            else
            {
                m_confirmingQuit = false;
                playSound(3);
                update();
            }
            break;
        case 5: // back/escape — cancel only, stay in settings
            m_confirmingQuit = false;
            playSound(3);
            update();
            break;
        default:
            break;
        }
        return;
    }

    if (currentScreen == Screen::Sidebar)
    {
        switch (direction)
        {
        case 0: // up
            sidebarIndex = (sidebarIndex - 1 + kSidebarCount) % kSidebarCount;
            playSound(2);
            update();
            break;
        case 1: // down
            sidebarIndex = (sidebarIndex + 1) % kSidebarCount;
            playSound(2);
            update();
            break;
        case 3: // right
        case 4: // confirm
            if (sidebarIndex == kSidebarCount - 1) // Quit Game
            {
                m_confirmingQuit = true;
                m_confirmIndex = 1; // default to No
                playSound(1);
                update();
            }
            // categories 0-5: no detail panels in PR #1
            break;
        case 5: // back
            playSound(3);
            emit settingsClosed();
            break;
        default:
            break;
        }
    }
}

void SettingsView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Up:    handleNavigation(0); event->accept(); break;
    case Qt::Key_Down:  handleNavigation(1); event->accept(); break;
    case Qt::Key_Left:  handleNavigation(2); event->accept(); break;
    case Qt::Key_Right: handleNavigation(3); event->accept(); break;
    case Qt::Key_Return:
    case Qt::Key_Space: handleNavigation(4); event->accept(); break;
    case Qt::Key_Escape: handleNavigation(5); event->accept(); break;
    default: QWidget::keyPressEvent(event); break;
    }
}

// ─── painting ─────────────────────────────────────────────────────────────────

void SettingsView::paintPillButton(QPainter& p, QRect r, const QString& label, bool selected, bool highlighted)
{
    QPainterPath path;
    path.addRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    if (selected)
    {
        grad.setColorAt(0.0, QColor(0, 200, 200));
        grad.setColorAt(1.0, QColor(0, 140, 140));
        p.fillPath(path, grad);
        p.strokePath(path, QPen(QColor(180, 255, 255), 2));
    }
    else
    {
        grad.setColorAt(0.0, QColor(50, 50, 50));
        grad.setColorAt(1.0, QColor(25, 25, 25));
        p.fillPath(path, grad);
        p.strokePath(path, QPen(QColor(120, 120, 120), 1));
    }

    p.setPen(Qt::white);
    p.setBrush(Qt::NoBrush);
    p.drawText(r, Qt::AlignCenter, label);
}

void SettingsView::paintBackground(QPainter& p)
{
    const int w = width();
    const int h = height();

    // Tile the background texture
    if (!m_background.isNull())
    {
        int bw = m_background.width();
        int bh = m_background.height();
        for (int y = 0; y < h; y += bh)
            for (int x = 0; x < w; x += bw)
                p.drawPixmap(x, y, m_background);
    }
    else
    {
        p.fillRect(0, 0, w, h, QColor(20, 20, 20));
    }

    // Theme color tint
    EmuInstance* emu = m_mainWindow->getEmuInstance();
    QColor tint(60, 60, 60);
    if (emu && emu->plugin)
    {
        ThemeColor tc = emu->plugin->defaultThemeColor();
        tint = QColor(tc.r, tc.g, tc.b);
    }
    tint.setAlpha(80);
    p.fillRect(0, 0, w, h, tint);

    // Diagonal visibility gradient: dark lower-left, transparent upper-right
    QLinearGradient diag(QPointF(0, h), QPointF(w, 0));
    diag.setColorAt(0.0, QColor(0, 0, 0, 190));
    diag.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(0, 0, w, h, diag);
}

void SettingsView::paintTitleBar(QPainter& p)
{
    const int w = width();
    const int h = height();
    const int barH = (int)(h * 0.08);
    const QRect barRect(0, 0, w, barH);

    QLinearGradient grad(barRect.topLeft(), barRect.bottomLeft());
    grad.setColorAt(0.0, QColor(180, 130, 0));
    grad.setColorAt(1.0, QColor(100, 70, 0));
    p.fillRect(barRect, grad);

    QFont font("KHGummi");
    font.setPixelSize(qMax(16, (int)(barH * 0.55)));
    font.setItalic(true);
    p.setFont(font);
    p.setPen(QColor(15, 5, 0));
    p.drawText(barRect.adjusted((int)(w * 0.03), 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft,
               "\xE2\x9A\x99 GAME SETTINGS");
}

void SettingsView::paintSidebar(QPainter& p)
{
    const int w = width();
    const int h = height();
    const double t = m_animClock.elapsed() / 1000.0;
    const int titleBarH = (int)(h * 0.08);

    // Width is fixed to accommodate longest label
    const int sidebarW = (int)(w * 0.30);
    const int sidebarX = (int)(w * 0.03);

    const int btnH = (int)qMax(28.0, h * 0.072);
    const int spacing = (int)(btnH * 0.35);
    const int totalH = kSidebarCount * btnH + (kSidebarCount - 1) * spacing;
    const int startY = titleBarH + (h - titleBarH - totalH) / 2;

    QFont font("KHMenu");
    font.setPixelSize(qMax(12, (int)(btnH * 0.48)));
    p.setFont(font);

    static const QPixmap handPixmap(":/ds/menu_hand.png");
    static const QPixmap lightPixmap(":/ds/menu_light.png");

    for (int i = 0; i < kSidebarCount; i++)
    {
        int y = startY + i * (btnH + spacing);
        QRect btnRect(sidebarX, y, sidebarW, btnH);
        bool selected = (i == sidebarIndex);
        paintPillButton(p, btnRect, QString::fromUtf8(kSidebarLabels[i]), selected, false);

        if (selected)
        {
            // Glow orbit — Lissajous 4:5 on the right cap of the selected pill
            if (!lightPixmap.isNull())
            {
                const qreal capR = btnH / 2.0;
                const qreal capCx = btnRect.right() - capR;
                const qreal capCy = btnRect.center().y();
                const qreal boxCx = capCx + 0.19 * capR;
                const qreal boxCy = capCy - 0.56 * capR;
                const qreal ampX  = 0.55 * capR;
                const qreal ampY  = 0.50 * capR;
                const qreal speed = 0.5;
                const qreal gx = boxCx + ampX * std::cos(2.0 * M_PI * (1.012 * speed) * t - 2.39);
                const qreal gy = boxCy + ampY * std::cos(2.0 * M_PI * (1.265 * speed) * t - 1.57);
                int ls = (int)(btnH * 0.5);
                p.drawPixmap(QRectF(gx - ls / 2.0, gy - ls / 2.0, ls, ls), lightPixmap, lightPixmap.rect());
            }

            // Hand cursor with bob animation
            if (!handPixmap.isNull())
            {
                qreal aspect = handPixmap.height() > 0
                    ? (qreal)handPixmap.width() / handPixmap.height() : 1.0;
                int hh = (int)(btnH * 1.15);
                int hwid = (int)(hh * aspect);
                qreal bob = btnH * 0.15 * std::sin(t * (2.0 * M_PI / 1.2));
                int hx = btnRect.left() - hwid + (int)(btnH * 0.20 + bob);
                int hy = btnRect.center().y() - hh / 2;
                p.drawPixmap(QRect(hx, hy, hwid, hh), handPixmap);
            }

            // Connector dot between sidebar and detail area
            const int dotR = qMax(4, btnH / 8);
            const int dotX = sidebarX + sidebarW + (int)(w * 0.02);
            const int dotY = btnRect.center().y();
            p.setBrush(QColor(255, 140, 0));
            p.setPen(Qt::NoPen);
            p.drawEllipse(QPoint(dotX, dotY), dotR, dotR);
            p.setPen(Qt::white);
        }
    }
}

void SettingsView::paintDetailArea(QPainter& p)
{
    const int w = width();
    const int h = height();
    const int titleBarH = (int)(h * 0.08);
    const int sidebarW = (int)(w * 0.30);
    const int sidebarX = (int)(w * 0.03);
    const int detailX = sidebarX + sidebarW + (int)(w * 0.05);
    const int detailW = w - detailX - (int)(w * 0.03);
    const int detailH = h - titleBarH - (int)(h * 0.04);
    const QRect detailRect(detailX, titleBarH + (int)(h * 0.02), detailW, detailH);

    p.fillRect(detailRect, QColor(15, 15, 15, 200));
}

void SettingsView::paintQuitConfirmation(QPainter& p)
{
    const int w = width();
    const int h = height();

    // Semi-transparent overlay
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 160));

    const int boxW = (int)(w * 0.50);
    const int boxH = (int)(h * 0.30);
    const int boxX = (w - boxW) / 2;
    const int boxY = (h - boxH) / 2;
    const QRect boxRect(boxX, boxY, boxW, boxH);

    p.fillRect(boxRect, QColor(20, 20, 30, 220));
    p.setPen(QPen(QColor(200, 200, 200), 2));
    p.drawRect(boxRect);

    // "Are you sure?" text
    QFont titleFont("KHMenu");
    titleFont.setPixelSize(qMax(14, (int)(boxH * 0.22)));
    p.setFont(titleFont);
    p.setPen(Qt::white);
    QRect textRect(boxX, boxY + (int)(boxH * 0.10), boxW, (int)(boxH * 0.35));
    p.drawText(textRect, Qt::AlignCenter, "Are you sure?");

    // Yes / No buttons
    const int btnH = (int)qMax(24.0, boxH * 0.22);
    const int btnW = (int)(boxW * 0.32);
    const int btnY = boxY + (int)(boxH * 0.60);
    const int gap  = (int)(boxW * 0.06);
    const int totalBtnsW = btnW * 2 + gap;
    const int btnStartX = boxX + (boxW - totalBtnsW) / 2;

    QFont btnFont("KHMenu");
    btnFont.setPixelSize(qMax(12, (int)(btnH * 0.50)));
    p.setFont(btnFont);

    // Yes (index 0)
    QRect yesRect(btnStartX, btnY, btnW, btnH);
    paintPillButton(p, yesRect, "\xE2\x8A\x97 Yes", m_confirmIndex == 0, false);

    // No (index 1)
    QRect noRect(btnStartX + btnW + gap, btnY, btnW, btnH);
    paintPillButton(p, noRect, "\xE2\x8A\x95 No", m_confirmIndex == 1, false);
}

void SettingsView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    paintBackground(p);
    paintTitleBar(p);
    paintSidebar(p);
    paintDetailArea(p);

    if (m_confirmingQuit)
        paintQuitConfirmation(p);
}
