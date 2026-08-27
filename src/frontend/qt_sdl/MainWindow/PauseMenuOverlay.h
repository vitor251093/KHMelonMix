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

#ifndef PAUSEMENUOVERLAY_H
#define PAUSEMENUOVERLAY_H

#include <QWidget>
#include <QElapsedTimer>
#include <QImage>
#include <QSize>

class QTimer;
class QPainter;

// Holds the state and rendering logic for the KH2-style cutscene pause menu (darkened
// background, "PAUSE" title, Continue/Skip options with the selected one highlighted,
// animated hand/glow), usable two ways:
//
//  - As an ordinary composited child widget: stack it as a child of whatever it should
//    appear on top of, resize it to match that widget, and show()+raise() it alongside
//    setMenuVisible(true). This is what CutsceneVideoView and ScreenPanelNative do; it
//    works because both paint through Qt's normal paint event / compositor. It never
//    intercepts input (Qt::WA_TransparentForMouseEvents), so the widget underneath keeps
//    receiving mouse/touch/keyboard events.
//  - Headless, via renderToImage(): for a surface that renders itself outside Qt's paint
//    system (e.g. ScreenPanelGL, which swaps an OpenGL buffer directly) - keep the widget
//    hidden, drive it purely through the setters below, and rasterize a frame on demand
//    to composite it however that surface needs (e.g. as a texture).
//
// Either way, selection is driven entirely through setSelection, matching the DS's own
// button-driven menu - this class has no input handling of its own.
class PauseMenuOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit PauseMenuOverlay(QWidget* parent = nullptr);

    void setMenuVisible(bool visible);
    bool isMenuVisible() const { return m_visible; }
    void setSelection(int selection);
    // Language for the pause-menu labels, in DS firmware Language order (0=ja, 1=en, 2=fr,
    // 3=de, 4=it, 5=es). Out-of-range values fall back to English.
    void setLanguage(int language);
    void setSizeModifier(double modifier);

    // Renders the current frame into a freshly-allocated transparent, premultiplied-alpha
    // image of the given (device-pixel) size, independent of this widget's own on-screen
    // visibility/geometry. For consumers that composite the menu themselves (see above).
    QImage renderToImage(const QSize& pixelSize) const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paint(QPainter& painter, int w, int h) const;

    bool m_visible = false;
    int m_selection = 0; // 0 = Continue, 1 = Skip
    int m_language = 1;  // firmware Language order (0=ja, 1=en, 2=fr, 3=de, 4=it, 5=es)
    double m_sizeModifier = 0.5;

    // Drives the hand-bob and glow-orbit animations while the menu is visible.
    QTimer* m_animTimer = nullptr;
    QElapsedTimer m_animClock;
};

#endif // PAUSEMENUOVERLAY_H
