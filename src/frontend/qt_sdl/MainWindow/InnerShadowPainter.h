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

#ifndef INNERSHADOWPAINTER_H
#define INNERSHADOWPAINTER_H

#include <QImage>
#include <QPainterPath>
#include <QPointF>
#include <vector>

// Renders a soft shadow that hugs the inside edge of a filled shape and fades
// toward its interior - like a CSS inset shadow with no offset.
class InnerShadowPainter
{
public:
    // Rasterizes the glyph/shape silhouette of 'path', blurs its background,
    // and clips that blur back to the silhouette, so darkening is strongest
    // right at the border and fades out toward the middle of each stroke.
    // 'outTopLeft' receives where to draw the returned image so it lines up
    // with 'path' again (in the same coordinate space 'path' was built in).
    static QImage renderInnerBorderShadow(const QPainterPath& path, int blurRadius, int peakAlpha, QPointF* outTopLeft);

private:
    // Approximates a Gaussian blur on an 8-bit alpha buffer via three passes of
    // a sliding-window box blur - a standard, cheap approximation, since three
    // box blurs in a row already look very close to one true Gaussian blur.
    static void boxBlurAlpha(std::vector<uchar>& buffer, int width, int height, int radius);
};

#endif // INNERSHADOWPAINTER_H
