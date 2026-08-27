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
#include "PauseMenuOverlay.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QResizeEvent>
#include <QFile>
#include <QStringList>
#include <algorithm>

// Paints the active subtitle line(s) near the bottom of a w*h area: white fill with a black
// outline (built as a QPainterPath so the stroke sits cleanly around the glyphs), in a
// Comic-Sans-style font, centered and stacked upward for multi-line cues.
static void paintSubtitle(QPainter& p, int w, int h, const QString& text)
{
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    float viewportAspectRatio = ((float)w)/(float)h;
    float videoAspectRatio = 16.0/9.0;
    bool shorterThanWidescreen = viewportAspectRatio < videoAspectRatio;
    float videoHeight = shorterThanWidescreen ? ((float)w)/videoAspectRatio : (float)h;

    QFont font;
    font.setFamilies({ "Comic Hearts" });
    font.setStyleHint(QFont::SansSerif);
    qreal px = qMax(2.0, videoHeight * 0.049);
    font.setPixelSize((int)px);
    font.setLetterSpacing(QFont::AbsoluteSpacing, px / 16.0);
    font.setWordSpacing(px / 3.5);
    QFontMetricsF fm(font);

    const qreal outline = qMax(2.0, px * 0.15);
    const QStringList lines = text.split('\n');
    const qreal lineH = fm.height();

    // The KH HD cutscenes are always in 16:9, so the baseline depends if the viewport
    // is taller, equal, or shorter than the cutscenes in height.
    qreal bottomline = h * 0.89;
    if (shorterThanWidescreen)
    {
        bottomline = (((float)h) - videoHeight)/2 + (videoHeight * 0.89);
    }

    // Anchor the bottom line near 89% of the height; stack earlier lines above it.
    qreal baseline = bottomline + fm.ascent() - (lines.size() - 1) * lineH;

    for (const QString& line : lines) {
        QPainterPath path;
        qreal tw = fm.horizontalAdvance(line);
        path.addText((w - tw) / 2.0, baseline, font, line);
        p.strokePath(path, QPen(QColor(0, 0, 0), outline, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.fillPath(path, QColor(255, 255, 255));
        baseline += lineH;
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

    m_pauseMenu = new PauseMenuOverlay(this);

    // Warm up the subtitle font path once at startup so the first subtitle that paints during a
    // cutscene doesn't trigger synchronous font registration + glyph rasterization on the GUI thread
    // (which caused a visible hitch right as subtitles first appeared). Painting into a throwaway
    // off-screen pixmap forces Qt's font engine to do its one-time
    // work now, when a few ms is imperceptible.
    {
        QPixmap warm(8, 8);
        warm.fill(Qt::transparent);
        QPainter wp(&warm);
        paintSubtitle(wp, 8, 8, QStringLiteral("Ag"));
    }
}

void CutsceneVideoView::setMenuVisible(bool visible)
{
    m_pauseMenu->setMenuVisible(visible);
    m_pauseMenu->setVisible(visible);
    if (visible) {
        m_pauseMenu->raise();
    }
}

void CutsceneVideoView::setMenuSelection(int selection)
{
    m_pauseMenu->setSelection(selection);
}

void CutsceneVideoView::setMenuLanguage(int language)
{
    m_pauseMenu->setLanguage(language);
}

void CutsceneVideoView::setMenuSizeModifier(double modifier)
{
    m_pauseMenu->setSizeModifier(modifier);
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
    m_pauseMenu->setGeometry(QRect(QPoint(0, 0), s.toSize()));
}

void CutsceneVideoView::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);

    // Subtitles render in device/pixel coordinates, beneath the pause menu (which is a separate
    // widget stacked on top - see m_pauseMenu). drawForeground runs once per video frame; we
    // render the cue into a cached pixmap and only rebuild it when the text or viewport size
    // changes, so steady-state cost is a single blit.
    if (m_activeCue >= 0 && m_activeCue < m_cues.size()) {
        const QString& text = m_cues[m_activeCue].text;
        const QSize vp = viewport()->size();
        if (m_subtitleCacheText != text || m_subtitleCacheSize != vp) {
            const qreal dpr = devicePixelRatioF();
            m_subtitlePixmap = QPixmap(vp * dpr);
            m_subtitlePixmap.setDevicePixelRatio(dpr);
            m_subtitlePixmap.fill(Qt::transparent);
            QPainter pp(&m_subtitlePixmap);
            paintSubtitle(pp, vp.width(), vp.height(), text);
            m_subtitleCacheText = text;
            m_subtitleCacheSize = vp;
        }
        painter->save();
        painter->resetTransform();
        painter->drawPixmap(0, 0, m_subtitlePixmap);
        painter->restore();
    }
}

// Parses an SRT timestamp ("HH:MM:SS,mmm", also accepting '.' as the decimal separator and stray
// surrounding whitespace) into milliseconds. Sets *ok to whether parsing succeeded.
static qint64 parseSrtTimeMs(const QString& raw, bool* ok)
{
    if (ok) *ok = false;
    const QString s = raw.trimmed();
    int sep = s.indexOf(',');
    if (sep < 0) sep = s.indexOf('.');
    const QString hms = (sep >= 0) ? s.left(sep) : s;
    QString frac = (sep >= 0) ? s.mid(sep + 1) : QString();

    const QStringList parts = hms.split(':');
    if (parts.size() != 3) {
        return 0;
    }
    bool okH = false, okM = false, okS = false;
    const qint64 h = parts[0].toLongLong(&okH);
    const qint64 m = parts[1].toLongLong(&okM);
    const qint64 sec = parts[2].toLongLong(&okS);
    if (!okH || !okM || !okS) {
        return 0;
    }
    qint64 ms = 0;
    if (!frac.isEmpty()) {
        frac = (frac + "000").left(3); // pad/truncate fractional part to milliseconds
        bool okF = false;
        ms = frac.toLongLong(&okF);
        if (!okF) {
            return 0;
        }
    }
    if (ok) *ok = true;
    return ((h * 60 + m) * 60 + sec) * 1000 + ms;
}

void CutsceneVideoView::loadSubtitles(const QString& filePath)
{
    m_cues.clear();
    m_activeCue = -1;
    m_subtitleCacheText.clear();  // invalidate the cached render for the new cutscene

    if (filePath.isEmpty()) {
        if (viewport()) {
            viewport()->update();
        }
        return;
    }

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }
    // SubRip (.srt): UTF-8, cues separated by a blank line. Each cue is an optional numeric index
    // line, then "HH:MM:SS,mmm --> HH:MM:SS,mmm", then one or more text lines (real line breaks).
    const QString contents = QString::fromUtf8(f.readAll());
    f.close();

    // Parse line by line so stray CRLF (from editing in Notepad etc.) and missing trailing blank
    // lines are tolerated. A cue is "open" once we see its timing line; its text accumulates until
    // the next blank line or timing line.
    qint64 curStart = 0, curEnd = 0;
    QStringList curText;
    bool haveTiming = false;
    auto flushCue = [&]() {
        if (haveTiming && !curText.isEmpty()) {
            m_cues.append({ curStart, curEnd, curText.join('\n') });
        }
        haveTiming = false;
        curText.clear();
    };

    const QStringList lines = contents.split('\n');
    for (QString line : lines) {
        if (line.endsWith('\r')) {
            line.chop(1); // tolerate CRLF line endings
        }
        const int arrow = line.indexOf("-->");
        if (arrow >= 0) {
            // A timing line starts a new cue; flush whatever we were accumulating.
            flushCue();
            bool okStart = false, okEnd = false;
            qint64 start = parseSrtTimeMs(line.left(arrow), &okStart);
            qint64 end = parseSrtTimeMs(line.mid(arrow + 3), &okEnd);
            if (okStart && okEnd) {
                curStart = start;
                curEnd = end;
                haveTiming = true;
            }
            continue;
        }
        if (line.trimmed().isEmpty()) {
            flushCue(); // blank line ends the current cue
            continue;
        }
        if (!haveTiming) {
            continue; // index line (or stray text) before a timing line: ignore
        }
        curText.append(line);
    }
    flushCue(); // last cue may have no trailing blank line

    std::sort(m_cues.begin(), m_cues.end(),
              [](const SubtitleCue& a, const SubtitleCue& b) { return a.startMs < b.startMs; });

    if (viewport()) {
        viewport()->update();
    }
}

void CutsceneVideoView::setPlaybackPosition(qint64 ms)
{
    int found = -1;
    for (int i = 0; i < m_cues.size(); i++) {
        if (ms >= m_cues[i].startMs && ms < m_cues[i].endMs) {
            found = i;
            break;
        }
    }
    if (found != m_activeCue) {
        m_activeCue = found;
        if (viewport()) {
            viewport()->update();
        }
    }
}
