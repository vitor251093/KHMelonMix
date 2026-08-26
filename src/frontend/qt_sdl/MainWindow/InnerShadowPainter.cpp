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

#include "InnerShadowPainter.h"

#include <QPainter>
#include <cmath>

void InnerShadowPainter::boxBlurAlpha(std::vector<uchar>& buffer, int width, int height, int radius)
{
    if (radius <= 0 || width <= 0 || height <= 0) return;

    std::vector<uchar> tmp(buffer.size());
    const int windowSize = radius * 2 + 1;

    for (int pass = 0; pass < 3; pass++) {
        // Horizontal pass: buffer -> tmp
        for (int y = 0; y < height; y++) {
            const int rowStart = y * width;
            int sum = 0;
            for (int x = -radius; x <= radius; x++)
                sum += buffer[rowStart + qBound(0, x, width - 1)];
            for (int x = 0; x < width; x++) {
                tmp[rowStart + x] = (uchar)(sum / windowSize);
                sum += buffer[rowStart + qBound(0, x + radius + 1, width - 1)]
                     - buffer[rowStart + qBound(0, x - radius, width - 1)];
            }
        }
        // Vertical pass: tmp -> buffer
        for (int x = 0; x < width; x++) {
            int sum = 0;
            for (int y = -radius; y <= radius; y++)
                sum += tmp[qBound(0, y, height - 1) * width + x];
            for (int y = 0; y < height; y++) {
                buffer[y * width + x] = (uchar)(sum / windowSize);
                sum += tmp[qBound(0, y + radius + 1, height - 1) * width + x]
                     - tmp[qBound(0, y - radius, height - 1) * width + x];
            }
        }
    }
}

QImage InnerShadowPainter::renderInnerBorderShadow(const QPainterPath& path, int blurRadius, int peakAlpha, QPointF* outTopLeft)
{
    const QRectF bounds = path.boundingRect();
    const int margin = blurRadius + 2;
    const int imgWidth = qMax(1, (int)std::ceil(bounds.width()) + margin * 2);
    const int imgHeight = qMax(1, (int)std::ceil(bounds.height()) + margin * 2);
    const QPointF topLeft(bounds.left() - margin, bounds.top() - margin);
    if (outTopLeft) *outTopLeft = topLeft;

    // Rasterize the glyph silhouette: opaque where filled, transparent elsewhere.
    QImage silhouette(imgWidth, imgHeight, QImage::Format_ARGB32_Premultiplied);
    silhouette.fill(Qt::transparent);
    {
        QPainter maskPainter(&silhouette);
        maskPainter.setRenderHint(QPainter::Antialiasing, true);
        maskPainter.translate(-topLeft);
        maskPainter.fillPath(path, Qt::black);
    }
    const QImage silhouetteAlpha = silhouette.convertToFormat(QImage::Format_Alpha8);

    // Complement of the silhouette: opaque outside the glyphs, transparent
    // inside. Blurring this and clipping it back to the silhouette is what
    // makes the border "bleed" inward instead of spreading outward.
    std::vector<uchar> outside((size_t)imgWidth * imgHeight);
    for (int y = 0; y < imgHeight; y++) {
        const uchar* row = silhouetteAlpha.constScanLine(y);
        for (int x = 0; x < imgWidth; x++)
            outside[(size_t)y * imgWidth + x] = 255 - row[x];
    }
    boxBlurAlpha(outside, imgWidth, imgHeight, blurRadius);

    QImage shadow(imgWidth, imgHeight, QImage::Format_ARGB32_Premultiplied);
    shadow.fill(Qt::transparent);
    for (int y = 0; y < imgHeight; y++) {
        const uchar* maskRow = silhouetteAlpha.constScanLine(y);
        QRgb* outRow = reinterpret_cast<QRgb*>(shadow.scanLine(y));
        for (int x = 0; x < imgWidth; x++) {
            const int a = outside[(size_t)y * imgWidth + x] * maskRow[x] * peakAlpha / (255 * 255);
            outRow[x] = qRgba(0, 0, 0, a); // premultiplied black: colour bytes stay 0 regardless of alpha
        }
    }
    return shadow;
}
