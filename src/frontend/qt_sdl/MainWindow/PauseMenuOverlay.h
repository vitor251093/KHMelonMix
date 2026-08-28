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
#include <QString>
#include <QStringList>

class QTimer;
class QPainter;

class PauseMenuOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit PauseMenuOverlay(QWidget* parent = nullptr);

    void setMenuVisible(bool visible);
    bool isMenuVisible() const { return m_visible; }
    void setSelection(int selection);
    void setTitle(const QString& title);
    void setSubtitle(const QString& subtitle);
    void setButtonLabels(const QStringList& labels);
    void setSizeModifier(double modifier);
    void setDarkenBackground(bool darken);

    QImage renderToImage(const QSize& pixelSize) const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paint(QPainter& painter, int w, int h) const;

    bool m_visible = false;
    int m_selection = 0;
    QString m_title = QStringLiteral("PAUSE");
    QString m_subtitle;
    QStringList m_buttonLabels = { QStringLiteral("Continue"), QStringLiteral("Skip") };
    double m_sizeModifier = 0.5;
    bool m_darkenBackground = true;

    // Drives the hand-bob and glow-orbit animations while the menu is visible.
    QTimer* m_animTimer = nullptr;
    QElapsedTimer m_animClock;
};

#endif // PAUSEMENUOVERLAY_H
