/*
    Copyright 2016-2023 melonDS team

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

#include "CutsceneVideoView.h"

#include <QPainter>
#include <QTimer>
#include <cmath>
#include <QPainterPath>
#include <QPolygonF>
#include <QPixmap>
#include <QFontDatabase>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QResizeEvent>

// Registers the KH fonts (once) so the families are usable by name below.
static void ensureCutsceneMenuAssets()
{
    static bool loaded = false;
    if (loaded) {
        return;
    }
    loaded = true;
    QFontDatabase::addApplicationFont(":/ds/KHMenu.ttf");
    QFontDatabase::addApplicationFont(":/ds/KHGummi.ttf");
}

// Paints the Skip/Continue menu in device (pixel) coordinates over a w*h area.
// 't' is the animation clock in seconds, used to drive the hand bob and the glow
// orbit so the selected entry matches the live KH2 pause menu.
static void paintCutsceneSkipMenu(QPainter& p, int w, int h, int selection, double t)
{
    ensureCutsceneMenuAssets();
    static const QPixmap handPixmap(":/ds/menu_hand.png");
    static const QPixmap lightPixmap(":/ds/menu_light.png");

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Darken the paused frame behind the menu
    p.fillRect(QRect(0, 0, w, h), QColor(0, 0, 0, 120));

    const int cx = w / 2;

    // "PAUSE" title in the stylised KH title font, italicised
    QFont titleFont("KHGummi");
    qreal titlePx = qMax(20.0, h * 0.060);
    titleFont.setPixelSize((int)titlePx);
    titleFont.setItalic(true);
    p.setFont(titleFont);

    const QString title = "PAUSE";
    QFontMetrics tfm(titleFont);
    int titleW = tfm.horizontalAdvance(title);
    int titleH = tfm.height();
    int titleY = (int)(h * 0.36);

    p.setPen(QPen(QColor(255, 255, 255), 1));
    p.drawText(QRect(cx - titleW, titleY - titleH, titleW * 2, titleH * 2),
               Qt::AlignHCenter | Qt::AlignVCenter, title);

    // Decorative rule line above and below the title, with equal vertical gaps around
    // the actual rendered glyphs (use the tight ink box so italic metrics don't skew it).
    int baseline = titleY - titleH / 2 + tfm.ascent();
    QRect inkBox = tfm.tightBoundingRect(title); // relative to the baseline (top is negative)
    int inkTop = baseline + inkBox.top();
    int inkBottom = baseline + inkBox.top() + inkBox.height();
    int linePad = (int)(titleH * 0.50);
    int lineHalf = (int)qMax((qreal)titleW * 0.70, w * 0.17);
    p.setPen(QPen(QColor(230, 230, 230), 2));
    int lineYTop = inkTop - linePad;
    int lineYBot = inkBottom + linePad;
    p.drawLine(cx - lineHalf, lineYTop, cx + lineHalf, lineYTop);
    p.drawLine(cx - lineHalf, lineYBot, cx + lineHalf, lineYBot);

    // Continue / Skip buttons in the rounded KH menu-entry font
    const char* labels[2] = { "Continue", "Skip" };
    int btnW = (int)(w * 0.26);
    int btnH = (int)qMax(28.0, h * 0.075);
    int spacing = (int)(btnH * 0.35);
    int firstY = (int)(h * 0.50);

    QFont btnFont("KHMenu");
    btnFont.setPixelSize((int)qMax(15.0, h * 0.040));
    p.setFont(btnFont);

    for (int i = 0; i < 2; i++)
    {
        QRect btnRect(cx - btnW / 2, firstY + i * (btnH + spacing), btnW, btnH);
        bool isSelected = (selection == i);

        QPainterPath path;
        path.addRoundedRect(btnRect, btnH / 2.0, btnH / 2.0);

        QLinearGradient grad(btnRect.topLeft(), btnRect.bottomLeft());
        if (isSelected) {
            grad.setColorAt(0.0, QColor(220, 40, 40));
            grad.setColorAt(1.0, QColor(150, 20, 20));
            p.fillPath(path, grad);
            // stroke only (drawPath would also fill with the current brush)
            p.strokePath(path, QPen(QColor(255, 230, 230), 2));
        } else {
            grad.setColorAt(0.0, QColor(70, 70, 70));
            grad.setColorAt(1.0, QColor(35, 35, 35));
            p.fillPath(path, grad);
            p.strokePath(path, QPen(QColor(180, 180, 180), 2));
        }

        p.setPen(Qt::white);
        p.setBrush(Qt::NoBrush);
        p.drawText(btnRect, Qt::AlignCenter, labels[i]);

        if (isSelected) {
            // Glow accent. The KH2 glow doesn't circle the cap; it wanders a small
            // Lissajous figure (~4:5 frequency ratio) inside a square region in the
            // upper-right of the rounded cap. The constants below were measured from
            // the reference footage (relative to the cap centre / radius).
            if (!lightPixmap.isNull()) {
                const qreal capR = btnH / 2.0;
                const qreal capCx = btnRect.right() - capR;
                const qreal capCy = btnRect.center().y();
                const qreal boxCx = capCx + 0.19 * capR;
                const qreal boxCy = capCy - 0.56 * capR;
                const qreal ampX  = 0.55 * capR;
                const qreal ampY  = 0.50 * capR;
                // Scale both frequencies together so the 4:5 figure keeps its shape,
                // just traced more slowly than the reference.
                const qreal speed = 0.5;
                const qreal gx = boxCx + ampX * std::cos(2.0 * M_PI * (1.012 * speed) * t - 2.39);
                const qreal gy = boxCy + ampY * std::cos(2.0 * M_PI * (1.265 * speed) * t - 1.57);

                int ls = (int)(btnH * 0.5);
                p.drawPixmap(QRectF(gx - ls / 2.0, gy - ls / 2.0, ls, ls), lightPixmap,
                             lightPixmap.rect());
            }
            // Pointing-hand cursor to the left of the selected button, beckoning with a
            // small horizontal bob toward the entry.
            if (!handPixmap.isNull()) {
                qreal aspect = handPixmap.height() > 0
                    ? (qreal)handPixmap.width() / handPixmap.height() : 1.0;
                int hh = (int)(btnH * 1.15);
                int hwid = (int)(hh * aspect);
                qreal bob = btnH * 0.15 * std::sin(t * (2.0 * M_PI / 1.2));
                int hx = btnRect.left() - hwid + (int)(btnH * 0.20 + bob);
                int hy = btnRect.center().y() - hh / 2;
                p.drawPixmap(QRect(hx, hy, hwid, hh), handPixmap);
            }
        }
    }
}

CutsceneVideoView::CutsceneVideoView(QWidget* parent) :
    QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setAspectRatioMode(Qt::KeepAspectRatio);
    m_scene->addItem(m_videoItem);

    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::black);
    setCursor(Qt::BlankCursor);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    // ~60 FPS repaint while the menu is up, so the hand/glow animations stay smooth.
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        if (viewport()) {
            viewport()->update();
        }
    });
}

void CutsceneVideoView::setMenuVisible(bool visible)
{
    if (m_menuVisible == visible) {
        return;
    }
    m_menuVisible = visible;
    if (visible) {
        m_animClock.restart();
        m_animTimer->start();
    } else {
        m_animTimer->stop();
    }
    if (viewport()) {
        viewport()->update();
    }
}

void CutsceneVideoView::setMenuSelection(int selection)
{
    m_menuSelection = selection;
    if (m_menuVisible && viewport()) {
        viewport()->update();
    }
}

void CutsceneVideoView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    QSizeF s = viewport()->size();
    m_scene->setSceneRect(0, 0, s.width(), s.height());
    if (m_videoItem) {
        // KeepAspectRatio letterboxes the video within this size; center it.
        m_videoItem->setSize(s);
        m_videoItem->setPos(0, 0);
    }
}

void CutsceneVideoView::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);

    if (!m_menuVisible) {
        return;
    }

    // Draw the menu in device/pixel coordinates over the whole viewport.
    const double t = m_animClock.isValid() ? m_animClock.elapsed() / 1000.0 : 0.0;
    painter->save();
    painter->resetTransform();
    paintCutsceneSkipMenu(*painter, viewport()->width(), viewport()->height(), m_menuSelection, t);
    painter->restore();
}
