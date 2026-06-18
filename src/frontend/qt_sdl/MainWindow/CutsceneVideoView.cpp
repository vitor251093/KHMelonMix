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
#include <QFile>
#include <QStringList>
#include <algorithm>

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
    QFontDatabase::addApplicationFont(":/ds/ComicHearts.otf");
}

// Paints the active subtitle line(s) near the bottom of a w*h area: white fill with a black
// outline (built as a QPainterPath so the stroke sits cleanly around the glyphs), in a
// Comic-Sans-style font, centered and stacked upward for multi-line cues.
static void paintSubtitle(QPainter& p, int w, int h, const QString& text)
{
    ensureCutsceneMenuAssets();

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    float viewportAspectRatio = ((float)w)/(float)h;
    float videoAspectRatio = 16.0/9.0;
    bool shorterThanWidescreen = viewportAspectRatio < videoAspectRatio;
    float videoHeight = shorterThanWidescreen ? ((float)w)/videoAspectRatio : (float)h;

    QFont font;
    font.setFamilies({ "Comic Hearts" });
    font.setStyleHint(QFont::SansSerif);
    qreal px = videoHeight * 0.049;
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

// Localized pause-menu strings, indexed by DS firmware Language order
// (0=ja, 1=en, 2=fr, 3=de, 4=it, 5=es). "Continue" wording matches the KH HD Collection's own
// movie/pause menu; "Skip" has no collection equivalent (its theater uses "Return to Title
// Screen"), so it uses standard translations - Italian "Salta" and Spanish "Saltar" do match
// the collection's in-game skip wording. The title is the stylized "PAUSE" header: uppercased
// to match the English logo (none of these words carry accents, so all-caps is safe) and using
// the title *noun* per language (Pause/Pausa) rather than the collection's verb action label
// ("Pausieren"/"Metti in pausa"). Japanese katakana ポーズ has no case. Stored as UTF-8 narrow
// literals (this file is UTF-8, like subtitleLanguageFolder's folder names) and decoded via
// QString::fromUtf8 at use.
struct CutsceneMenuStrings { const char* title; const char* cont; const char* skip; };
static CutsceneMenuStrings cutsceneMenuStrings(int language)
{
    static const CutsceneMenuStrings table[6] = {
        { "ポーズ",  "つづける",     "スキップ" },        // 0 Japanese
        { "PAUSE",  "Continue",    "Skip" },            // 1 English
        { "PAUSE",  "Continuer",   "Passer" },          // 2 French
        { "PAUSE",  "Fortfahren",  "Überspringen" },    // 3 German
        { "PAUSA",  "Continua",    "Salta" },           // 4 Italian
        { "PAUSA",  "Continuar",   "Saltar" },          // 5 Spanish
    };
    if (language < 0 || language >= 6) language = 1; // English fallback
    return table[language];
}

// Paints the Skip/Continue menu in device (pixel) coordinates over a w*h area.
// 't' is the animation clock in seconds, used to drive the hand bob and the glow
// orbit so the selected entry matches the live KH2 pause menu. 'language' selects the
// localized labels (firmware Language order; see cutsceneMenuStrings).
static void paintCutsceneSkipMenu(QPainter& p, int w, int h, int selection, double t, int language)
{
    const CutsceneMenuStrings strings = cutsceneMenuStrings(language);

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

    const QString title = QString::fromUtf8(strings.title);
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
    const QString labels[2] = { QString::fromUtf8(strings.cont), QString::fromUtf8(strings.skip) };
    int btnW = (int)(w * 0.26);
    int btnH = (int)qMax(28.0, h * 0.075);
    int spacing = (int)(btnH * 0.35);
    int firstY = (int)(h * 0.50);

    QFont btnFont("KHMenu");
    btnFont.setPixelSize((int)qMax(15.0, h * 0.040));
    // Some localized labels are wider than the English ones (e.g. German "Überspringen").
    // Shrink the font until the widest label fits the button's flat area - the rounded caps
    // eat ~btnH of width - so labels stay inside the button rather than being clipped.
    {
        const qreal avail = btnW - btnH;
        QFontMetrics bfm(btnFont);
        int widest = 0;
        for (const QString& l : labels) widest = qMax(widest, bfm.horizontalAdvance(l));
        if (widest > avail && avail > 0) {
            btnFont.setPixelSize(qMax(10, (int)(btnFont.pixelSize() * avail / widest)));
        }
    }
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

    // Warm up the subtitle font path once at startup so the first subtitle that paints during a
    // cutscene doesn't trigger synchronous font registration + glyph rasterization on the GUI thread
    // (which caused a visible hitch right as subtitles first appeared). Painting into a throwaway
    // off-screen pixmap forces ensureCutsceneMenuAssets() and Qt's font engine to do their one-time
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

void CutsceneVideoView::setMenuLanguage(int language)
{
    m_menuLanguage = language;
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

    // Subtitles render in device/pixel coordinates, beneath the pause menu (so both can show).
    // drawForeground runs once per video frame; we render the cue into a cached pixmap and only
    // rebuild it when the text or viewport size changes, so steady-state cost is a single blit.
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

    if (!m_menuVisible) {
        return;
    }

    // Draw the menu in device/pixel coordinates over the whole viewport.
    const double t = m_animClock.isValid() ? m_animClock.elapsed() / 1000.0 : 0.0;
    painter->save();
    painter->resetTransform();
    paintCutsceneSkipMenu(*painter, viewport()->width(), viewport()->height(), m_menuSelection, t, m_menuLanguage);
    painter->restore();
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
