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

#ifndef CUTSCENEVIDEOVIEW_H
#define CUTSCENEVIDEOVIEW_H

#include <QGraphicsView>
#include <QElapsedTimer>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QSize>

class QGraphicsScene;
class QGraphicsVideoItem;
class QTimer;

// Renders the HD cutscene video and, when paused, the Skip/Continue menu on top
// of it. The menu is composited into this same widget (rather than a separate
// window) so it moves, stacks and focuses with the main window on every platform
// (including Wayland, where apps can't position their own top-level windows).
// Mimics the in-game cutscene pause menu (darkened background, "PAUSE" title,
// Continue/Skip options with the selected one highlighted).
class CutsceneVideoView : public QGraphicsView
{
public:
    explicit CutsceneVideoView(QWidget* parent = nullptr);
    QGraphicsVideoItem* videoItem() const { return m_videoItem; }
    void setMenuVisible(bool visible);
    void setMenuSelection(int selection);
    // Language for the pause-menu labels, in DS firmware Language order (0=ja, 1=en, 2=fr,
    // 3=de, 4=it, 5=es). Out-of-range values fall back to English.
    void setMenuLanguage(int language);

    // Parses a SubRip (.srt) subtitle file (empty path clears subtitles). See loadSubtitles.
    void loadSubtitles(const QString& filePath);
    // Updates the active cue from the current playback position (milliseconds), repainting on change.
    void setPlaybackPosition(qint64 ms);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    struct SubtitleCue { qint64 startMs; qint64 endMs; QString text; };

    QGraphicsScene* m_scene = nullptr;
    QGraphicsVideoItem* m_videoItem = nullptr;
    bool m_menuVisible = false;
    int m_menuSelection = 0; // 0 = Continue, 1 = Skip
    int m_menuLanguage = 1;  // firmware Language order (0=ja, 1=en, 2=fr, 3=de, 4=it, 5=es)

    // Subtitle cues for the current cutscene, sorted by start time, and the active one (-1 = none).
    QVector<SubtitleCue> m_cues;
    int m_activeCue = -1;

    // The active cue is pre-rendered (text + outline) into a pixmap and cached, so the per-video-
    // frame repaint is just a blit rather than rebuilding/stroking a QPainterPath every frame.
    // Rebuilt only when the cue text or the viewport size changes.
    QPixmap m_subtitlePixmap;
    QString m_subtitleCacheText;
    QSize m_subtitleCacheSize;

    // Drives the hand-bob and glow-orbit animations while the menu is visible.
    QTimer* m_animTimer = nullptr;
    QElapsedTimer m_animClock;
};

#endif // CUTSCENEVIDEOVIEW_H
