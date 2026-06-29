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

#ifndef MAINWINDOWSETTINGS_H
#define MAINWINDOWSETTINGS_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QByteArray>
#include <QPushButton>
#include <initializer_list>

#include "Config.h"
#include "CutsceneVideoView.h"

class EmuInstance;

namespace melonMix {
class AudioPlayer;
}

namespace Ui { class MainWindowSettings; }
class MainWindowSettings;

class MainWindowSettings : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowSettings(EmuInstance* inst, QWidget* parent);
    ~MainWindowSettings();

public slots:
    void asyncStartBgmMusic(quint16 bgmId, quint8 volume, bool bResumePos, quint32 delayAtStart, QString bgmMusicFilePath);
    void asyncStopBgmMusic(quint16 bgmId, bool bStoreResumePos, quint32 fadeOutDuration);
    void asyncPauseBgmMusic();
    void asyncUnpauseBgmMusic();
    void asyncUpdateBgmMusicVolume(quint8 ramVolume);
    void asyncStopAllBgm();

    void startBgmMusic(quint16 bgmId, quint8 volume, bool bResumePos, QString bgmMusicFilePath);
    void stopBgmMusic(quint16 bgmId, bool bStoreResumePos, quint32 fadeOutDuration);
    void pauseBgmMusic();
    void unpauseBgmMusic();
    void stopAllBgm();

    void updateBgmMusicVolume(quint8 ramVolume);

    void asyncStartVideo(QString videoFilePath, QString subtitlesFilePath, int menuLanguage);
    void asyncStopVideo();
    void asyncPauseVideo();
    void asyncUnpauseVideo();

    void startVideo(QString videoFilePath, QString subtitlesFilePath, int menuLanguage);
    void stopVideo();
    void pauseVideo();
    void unpauseVideo();

    void asyncShowCutsceneSkipMenu(int selection);
    void asyncUpdateCutsceneSkipMenu(int selection);
    void asyncHideCutsceneSkipMenu();

    void showCutsceneSkipMenu(int selection);
    void updateCutsceneSkipMenu(int selection);
    void hideCutsceneSkipMenu();

    // kind: 1 = enter, 2 = move (up/down), 3 = continue, 4 = select
    void asyncPlayCutsceneMenuSound(int kind);
    void playCutsceneMenuSound(int kind);

private slots:
    void onBgmFadeOutCompleted(melonMix::AudioPlayer* playerStopped);
    void startBgmMusicDelayed(quint16 bgmId, quint8 volume, bool bResumePos, QString bgmMusicFilePath);

protected:
    void onAudioOutputsChanged();
    void keyPressEvent(QKeyEvent* event) override;

    QWidget* settingsWidget;
    QStackedWidget* settingWidgetOptions;
    bool showingSettings;

    void initWidgets();

    virtual void showGame() = 0;

private:
    QScopedPointer<Ui::MainWindowSettings> ui;
    Config::Table& localCfg;
    EmuInstance* emuInstance;

    QScopedPointer<QMediaDevices> mediaDevices;
    QAudioDevice currentOutputDevice;

    CutsceneVideoView* playerView = nullptr;
    QScopedPointer<QAudioOutput> playerAudioOutput;
    QScopedPointer<QMediaPlayer> player;

    QList<melonMix::AudioPlayer*> bgmPlayers;
    QScopedPointer<QTimer> delayedBgmStart;
    quint16 bgmToResumeId = 0;
    quint64 bgmToResumePosition = 0;

    // Menu sound effects, indexed by sound kind 1..4 (slot 0 unused). We preload the
    // decoded PCM and play it through a dedicated SDL audio device using the queue API.
    // Qt Multimedia (QSoundEffect / QAudioSink) was unusable here: its Linux GStreamer
    // backend either degrades the sound on repeated playback or churns GObjects (one
    // "g_object_unref" critical per replay). SDL is already our audio backend and the
    // clear+queue API restarts the clip cleanly with no object churn.
    QByteArray m_sfxPcm[5];
    unsigned int m_sfxAudioDevice = 0; // SDL_AudioDeviceID (0 = not open)

    void createVideoPlayer();
    void createMenuSounds();
    qreal getBgmMusicVolume(quint8 ramVolume);
};

#endif // MAINWINDOWSETTINGS_H
