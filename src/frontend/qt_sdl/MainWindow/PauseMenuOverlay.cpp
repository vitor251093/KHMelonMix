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

#include "PauseMenuOverlay.h"
#include "InnerShadowPainter.h"

#include <QPainter>
#include <QTimer>
#include <cmath>
#include <QPainterPath>
#include <QTransform>
#include <QPixmap>

static void paintPauseMenu(QPainter& p, int w, int h, int selection, double t, const QString& title,
                            const QString& subtitle, const QStringList& buttonLabels, double menuSizeModifier,
                            bool darkenBackground)
{
    static const QPixmap handCursorPixmap(":/ds/menu_hand.png");
    static const QPixmap glowPixmap(":/ds/menu_light.png");
    static const QPixmap selectedButtonPixmap(":/ds/button_selected.png");
    static const QPixmap unselectedButtonPixmap(":/ds/button_unselected.png");
    static const QPixmap pauseLabelPixmap(":/ds/pause_label.png");

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (darkenBackground) {
        p.fillRect(QRect(0, 0, w, h), QColor(0, 0, 0, 120));
    }

    const int centerX = w / 2;

    double pauseSizeModifier = (15.0/13.0) * menuSizeModifier;
    double buttonsSizeModifier = 1.25 * menuSizeModifier;

    const bool hasSubtitle = !subtitle.isEmpty();

    double bottomMargin = (hasSubtitle ? -0.01 : 0.03) * menuSizeModifier;
    double titleCenterY = h * (0.5 - bottomMargin - 0.13 * menuSizeModifier - (hasSubtitle ? 0.045 * menuSizeModifier : 0.0));
    double subtitleCenterY = h * (0.5 - bottomMargin);
    double firstButtonY = h * (0.5 - bottomMargin + (hasSubtitle ? 0.055 * menuSizeModifier : 0.0));

    if (!pauseLabelPixmap.isNull()) {
        const qreal titleImageHeight = h * 0.15 * pauseSizeModifier;
        const qreal titleImageWidth = titleImageHeight * pauseLabelPixmap.width() / pauseLabelPixmap.height();
        const QRectF titleImageRect(centerX - titleImageWidth / 2.0, titleCenterY - titleImageHeight / 2.0,
                                     titleImageWidth, titleImageHeight);
        p.drawPixmap(titleImageRect, pauseLabelPixmap, pauseLabelPixmap.rect());
    }

    QFont titleFont("KHGummi");
    qreal titlePixelSize = h * 0.064 * pauseSizeModifier;
    titleFont.setPixelSize((int)titlePixelSize);
    p.setFont(titleFont);

    const QString& titleText = title;
    QFontMetrics titleFontMetrics(titleFont);
    qreal titleTextWidth = titleFontMetrics.horizontalAdvance(titleText);

    // Build the glyphs as a path to make changes to it
    qreal titleBaselineY = ((int)titleCenterY) + (titleFontMetrics.ascent() - titleFontMetrics.descent()) / 2.0;
    QPainterPath titlePath;
    titlePath.addText(centerX - titleTextWidth / 2.0, titleBaselineY, titleFont, titleText);

    // Squeeze the text horizontally by 20%
    QTransform titleSqueeze;
    titleSqueeze.translate(centerX, 0.0).scale(0.8, 1.0).translate(-centerX, 0.0);
    titlePath = titleSqueeze.map(titlePath);

    // Black outline
    const qreal titleOutlineWidth = titlePixelSize * 0.15 * pauseSizeModifier;
    p.strokePath(titlePath, QPen(QColor(0, 0, 0), titleOutlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Text color
    p.fillPath(titlePath, QColor(255, 255, 255));

    static QImage cachedTitleShadow;
    static QPointF cachedTitleShadowPos;
    static QString cachedTitleShadowText;
    static QRectF cachedTitleShadowBounds;
    const QRectF titleBounds = titlePath.boundingRect();
    if (cachedTitleShadowText != titleText || cachedTitleShadowBounds != titleBounds) {
        const int blurRadius = (int)(titlePixelSize * 0.03 * pauseSizeModifier);
        cachedTitleShadow = InnerShadowPainter::renderInnerBorderShadow(titlePath, blurRadius, 235, &cachedTitleShadowPos);
        cachedTitleShadowText = titleText;
        cachedTitleShadowBounds = titleBounds;
    }
    p.drawImage(cachedTitleShadowPos, cachedTitleShadow);

    // Secondary label (e.g. a confirmation prompt), centered in the gap opened up above.
    if (hasSubtitle) {
        QFont subtitleFont("DFSouGei-W5G-KH25");
        subtitleFont.setPixelSize((int)(h * 0.058 * buttonsSizeModifier));
        QFontMetrics subtitleFontMetrics(subtitleFont);
        p.setFont(subtitleFont);

        const qreal subtitleTextWidth = subtitleFontMetrics.horizontalAdvance(subtitle);
        const QRect subtitleInkBox = subtitleFontMetrics.tightBoundingRect(subtitle);
        const qreal subtitleBaselineY = subtitleCenterY - subtitleInkBox.top() - subtitleInkBox.height() / 2.0;
        const qreal subtitleX = centerX - subtitleTextWidth / 2.0;

        const qreal subtitleShadowOffset = subtitleFontMetrics.height() * 0.07 * buttonsSizeModifier;
        p.setPen(QColor(0, 0, 0));
        p.drawText(QPointF(subtitleX + subtitleShadowOffset, subtitleBaselineY + subtitleShadowOffset), subtitle);

        p.setPen(Qt::white);
        p.drawText(QPointF(subtitleX, subtitleBaselineY), subtitle);
    }

    // Buttons
    int buttonHeight = (int)(h * 0.075 * buttonsSizeModifier);
    const QPixmap& referenceButtonPixmap = !selectedButtonPixmap.isNull() ? selectedButtonPixmap : unselectedButtonPixmap;
    const qreal buttonAspectRatio = (!referenceButtonPixmap.isNull() && referenceButtonPixmap.height() > 0)
        ? (qreal)referenceButtonPixmap.width() / referenceButtonPixmap.height() : 5.3;
    int buttonWidth = (int)(buttonHeight * buttonAspectRatio);
    int buttonSpacing = (int)(buttonHeight * 0.175);

    QFont buttonFont("DFSouGei-W5G-KH25");
    buttonFont.setPixelSize((int)(h * 0.056 * buttonsSizeModifier));
    // Some localized labels are wider than the English ones (e.g. German "Überspringen").
    // Shrink the font until the widest label fits the button's flat area - the rounded caps
    // eat ~buttonHeight of width - so labels stay inside the button rather than being clipped.
    {
        const qreal availableTextWidth = buttonWidth - buttonHeight;
        QFontMetrics shrinkFontMetrics(buttonFont);
        int widestLabelWidth = 0;
        for (const QString& label : buttonLabels) {
            widestLabelWidth = qMax(widestLabelWidth, shrinkFontMetrics.horizontalAdvance(label));
        }
        if (widestLabelWidth > availableTextWidth && availableTextWidth > 0) {
            buttonFont.setPixelSize((int)(buttonFont.pixelSize() * availableTextWidth / widestLabelWidth));
        }
    }
    p.setFont(buttonFont);
    const QFontMetrics buttonFontMetrics(buttonFont);

    for (int i = 0; i < buttonLabels.size(); i++)
    {
        QRect buttonRect(centerX - buttonWidth / 2, ((int)firstButtonY) + i * (buttonHeight + buttonSpacing),
                          buttonWidth, buttonHeight);
        bool isSelected = (selection == i);

        const QPixmap& buttonBackgroundPixmap = isSelected ? selectedButtonPixmap : unselectedButtonPixmap;
        if (!buttonBackgroundPixmap.isNull()) {
            p.drawPixmap(buttonRect, buttonBackgroundPixmap, buttonBackgroundPixmap.rect());
        }

        // Center on the label's own ink box rather than QFontMetrics::height/ascent - the
        // button font's vertical metrics don't line up with its glyphs' visual center, so
        // Qt::AlignCenter alone would sit noticeably off from the pill's middle.
        p.setBrush(Qt::NoBrush);
        const QString& labelText = buttonLabels[i];
        const QRect labelInkBox = buttonFontMetrics.tightBoundingRect(labelText); // relative to baseline (top is negative)
        const qreal labelTextWidth = buttonFontMetrics.horizontalAdvance(labelText);
        const qreal labelBaselineY = (buttonRect.center().y() - labelInkBox.top() - labelInkBox.height() / 2.0) - (h * 0.0025 * buttonsSizeModifier);
        const qreal labelX = buttonRect.center().x() - labelTextWidth / 2.0;

        // Drop shadow cast down-and-right, offset from the label itself.
        const qreal labelShadowOffset = buttonFontMetrics.height() * 0.07 * buttonsSizeModifier;
        p.setPen(QColor(0, 0, 0));
        p.drawText(QPointF(labelX + labelShadowOffset, labelBaselineY + labelShadowOffset), labelText);

        p.setPen(Qt::white);
        p.drawText(QPointF(labelX, labelBaselineY), labelText);

        if (isSelected) {
            // Glow accent. The KH2 glow doesn't circle the cap; it wanders a small
            // Lissajous figure (~4:5 frequency ratio) inside a square region in the
            // upper-right of the rounded cap. The constants below were measured from
            // the reference footage (relative to the cap centre / radius).
            if (!glowPixmap.isNull()) {
                const qreal capRadius = buttonHeight / 2.0;
                const qreal capCenterX = buttonRect.right() - capRadius;
                const qreal capCenterY = buttonRect.center().y();
                const qreal glowBoxCenterX = capCenterX + 0.19 * capRadius;
                const qreal glowBoxCenterY = capCenterY - 0.56 * capRadius;
                const qreal glowAmplitudeX = 0.55 * capRadius;
                const qreal glowAmplitudeY = 0.50 * capRadius;
                // Scale both frequencies together so the 4:5 figure keeps its shape,
                // just traced more slowly than the reference.
                const qreal glowSpeed = 0.5;
                const qreal glowX = glowBoxCenterX + glowAmplitudeX * std::cos(2.0 * M_PI * (1.012 * glowSpeed) * t - 2.39);
                const qreal glowY = glowBoxCenterY + glowAmplitudeY * std::cos(2.0 * M_PI * (1.265 * glowSpeed) * t - 1.57);

                int glowSize = (int)(buttonHeight * 0.5);
                p.drawPixmap(QRectF(glowX - glowSize / 2.0, glowY - glowSize / 2.0, glowSize, glowSize), glowPixmap,
                             glowPixmap.rect());
            }
            // Pointing-hand cursor to the left of the selected button, beckoning with a
            // small horizontal bob toward the entry.
            if (!handCursorPixmap.isNull()) {
                qreal handAspectRatio = handCursorPixmap.height() > 0
                    ? (qreal)handCursorPixmap.width() / handCursorPixmap.height() : 1.0;
                int handHeight = (int)(buttonHeight * 1.15);
                int handWidth = (int)(handHeight * handAspectRatio);
                qreal handBobOffset = buttonHeight * 0.15 * std::sin(t * (2.0 * M_PI / 1.2));
                int handX = buttonRect.left() - handWidth + (int)(buttonHeight * 0.20 + handBobOffset);
                int handY = buttonRect.center().y() - handHeight / 2;
                p.drawPixmap(QRect(handX, handY, handWidth, handHeight), handCursorPixmap);
            }
        }
    }
}

PauseMenuOverlay::PauseMenuOverlay(QWidget* parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    hide();

    // ~60 FPS repaint while the menu is up, so the hand/glow animations stay smooth.
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() { update(); });
}

void PauseMenuOverlay::setMenuVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    if (visible) {
        m_animClock.restart();
        m_animTimer->start();
    } else {
        m_animTimer->stop();
    }
}

void PauseMenuOverlay::setSelection(int selection)
{
    m_selection = selection;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setTitle(const QString& title)
{
    m_title = title;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setSubtitle(const QString& subtitle)
{
    m_subtitle = subtitle;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setButtonLabels(const QStringList& labels)
{
    m_buttonLabels = labels;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setSizeModifier(double modifier)
{
    m_sizeModifier = modifier;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::setDarkenBackground(bool darken)
{
    m_darkenBackground = darken;
    if (m_visible) {
        update();
    }
}

void PauseMenuOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    paint(painter, width(), height());
}

QImage PauseMenuOverlay::renderToImage(const QSize& pixelSize) const
{
    QImage image(pixelSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    paint(painter, pixelSize.width(), pixelSize.height());
    return image;
}

void PauseMenuOverlay::paint(QPainter& painter, int w, int h) const
{
    const double t = m_animClock.isValid() ? m_animClock.elapsed() / 1000.0 : 0.0;
    paintPauseMenu(painter, w, h, m_selection, t, m_title, m_subtitle, m_buttonLabels, m_sizeModifier, m_darkenBackground);
}
