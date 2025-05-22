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

#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <SDL2/SDL.h>

#include "types.h"
#include "Platform.h"

#include "MainWindowSettings.h"
#include "ui_MainWindowSettings.h"

#include "EmuInstance.h"
#include "AudioSource.h"
#include "AudioPlayer.h"

using namespace melonDS;

MainWindowSettings::MainWindowSettings(EmuInstance* inst, QWidget* parent) :
    QMainWindow(parent),
    emuInstance(inst),
    localCfg(inst->getLocalConfig()),
    ui(new Ui::MainWindowSettings),
    playerWidget(new QVideoWidget(this)),
    playerAudioOutput(new QAudioOutput(this)),
    player(new QMediaPlayer(this))
{
    ui->setupUi(this);

    settingsWidget = findChild<QWidget*>("settingsWidget");
    settingWidgetOptions = findChild<QStackedWidget*>("settingWidgetOptions");
    showingSettings = false;
}

MainWindowSettings::~MainWindowSettings()
{
}

void MainWindowSettings::initWidgets()
{
    createVideoPlayer();
}

void MainWindowSettings::asyncStartBgmMusic(quint16 bgmId, quint8 volume, bool bResumePos, quint32 delayAtStart, QString bgmMusicFilePath)
{
    if (delayAtStart == 0) {
        QMetaObject::invokeMethod(this, "startBgmMusic", Qt::QueuedConnection,
            Q_ARG(quint16, bgmId), Q_ARG(quint8, volume),
            Q_ARG(bool, bResumePos), Q_ARG(QString, bgmMusicFilePath));
    } else {
        QTimer* timer = new QTimer(this);

        // The delay is expressed as offset from the beginning of the currently playing movie
        qint32 delayFromMovieStart = qMax(0, delayAtStart - player->position());
        timer->setInterval(delayFromMovieStart);
        timer->setSingleShot(true);

        printf("Delay to start replacement song %d (%dms)\n", bgmId, delayFromMovieStart);

        QObject::connect(timer, &QTimer::timeout, [this, bgmId, volume, bResumePos, bgmMusicFilePath](){
            QMetaObject::invokeMethod(this, "startBgmMusic", Qt::QueuedConnection,
                Q_ARG(quint16, bgmId), Q_ARG(quint8, volume),
                Q_ARG(bool, bResumePos), Q_ARG(QString, bgmMusicFilePath));
        });
        timer->start();
        delayedBgmStart.reset(timer);
    }
}

void MainWindowSettings::startBgmMusic(quint16 bgmId, quint8 volume, bool bResumePos, QString bgmMusicFilePath)
{
    if (bgmMusicFilePath.isEmpty())
        return;

    melonMix::AudioPlayer* bgmPlayer = new melonMix::AudioPlayer(this, bgmId);
    bgmPlayer->loadFile(bgmMusicFilePath);

    quint64 startPosition = 0;
    if (bResumePos && bgmId == bgmToResumeId) {
        startPosition = bgmToResumePosition;
    }

    static constexpr int kFadeInDurationMs = 600;
    int fadeIn = (startPosition > 0) ? kFadeInDurationMs : 0;
    qreal initialVolume = getBgmMusicVolume(volume);
    bgmPlayer->play(startPosition, initialVolume, fadeIn);
    if (bResumePos) {
        printf("Starting replacement song %d (Resumed with fadein at pos %lld) volume: %.3f\n", bgmId, startPosition, initialVolume);
    } else {
        printf("Starting replacement song %d volume: %.3f\n", bgmId, initialVolume);
    }

    bgmPlayers.append(bgmPlayer);
}

void MainWindowSettings::startBgmMusicDelayed(quint16 bgmId, quint8 volume, bool bResumePos, QString bgmMusicFilePath)
{
    startBgmMusic(bgmId, volume, bResumePos, bgmMusicFilePath);
    delayedBgmStart.reset(nullptr);
}

void MainWindowSettings::asyncUpdateBgmMusicVolume(quint8 ramVolume)
{
    QMetaObject::invokeMethod(this, "updateBgmMusicVolume", Qt::QueuedConnection, Q_ARG(quint8, ramVolume));
}

void MainWindowSettings::updateBgmMusicVolume(quint8 ramVolume)
{
    qreal volume = getBgmMusicVolume(ramVolume);

    for(auto* player : bgmPlayers) {
        player->setVolume(volume, 1000); // 1 sec transition
    }
}

qreal MainWindowSettings::getBgmMusicVolume(quint8 ramVolume)
{
    int volume = localCfg.GetInt("Audio.BGMVolume");
    if (volume == 0) {
        volume = (localCfg.GetInt("Audio.Volume") * 100) / 256;
        localCfg.SetInt("Audio.BGMVolume", volume);
    }

    if (ramVolume == 0x40) {
        // Volume is decreased when paused or during cutscenes
        volume *= 0.7;
    }

    return (volume / 256.0);
}

void MainWindowSettings::asyncStopBgmMusic(quint16 bgmId, bool bStoreResumePos, quint32 fadeOutDuration)
{
    QMetaObject::invokeMethod(this, "stopBgmMusic", Qt::QueuedConnection, Q_ARG(quint16, bgmId), Q_ARG(bool, bStoreResumePos), Q_ARG(quint32, fadeOutDuration));
}

void MainWindowSettings::stopBgmMusic(quint16 bgmId, bool bStoreResumePos, quint32 fadeOutDuration)
{
    if (delayedBgmStart)
        delayedBgmStart.reset();

    for(auto* player : bgmPlayers) {
        if (player->getBgmId() == bgmId && player->isPlaying()) {
            printf("Stopping replacement song %d with %dms fadeout\n", bgmId, fadeOutDuration);
            player->stop(fadeOutDuration);

            if (bStoreResumePos) {
                bgmToResumeId = bgmId;
                bgmToResumePosition = player->getCurrentPlayingPos();
            }
        }
    }
}

void MainWindowSettings::asyncPauseBgmMusic()
{
    QMetaObject::invokeMethod(this, "pauseBgmMusic", Qt::QueuedConnection);
}

void MainWindowSettings::pauseBgmMusic()
{
    printf("Pausing bgm music\n");

    for(auto* player : bgmPlayers) {
        player->pause();
    }
}

void MainWindowSettings::asyncUnpauseBgmMusic()
{
    QMetaObject::invokeMethod(this, "unpauseBgmMusic", Qt::QueuedConnection);
}

void MainWindowSettings::unpauseBgmMusic()
{
    printf("Resuming bgm music\n");

    for(auto* player : bgmPlayers) {
        player->resume();
    }
}

void MainWindowSettings::asyncStopAllBgm()
{
    QMetaObject::invokeMethod(this, "stopAllBgm", Qt::QueuedConnection);
}

void MainWindowSettings::stopAllBgm()
{
    printf("Stop all bgm\n");

    for(auto* player : bgmPlayers) {
        player->stop(0);
    }
}

void MainWindowSettings::onBgmFadeOutCompleted(melonMix::AudioPlayer* playerStopped)
{
    for(auto it = bgmPlayers.begin(); it != bgmPlayers.end();) {
        auto* player (*it);
        if (player == playerStopped) {
            delete player;
            it = bgmPlayers.erase(it);
        } else {
            it++;
        }
    }
}

void MainWindowSettings::createVideoPlayer()
{
    playerWidget->setCursor(Qt::BlankCursor);

    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    centralWidget->addWidget(playerWidget.get());

    connect(player.get(), &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
        printf("======= MediaStatus: %d\n", status);

        if (status == QMediaPlayer::BufferingMedia || status == QMediaPlayer::BufferedMedia) {
            emuInstance->plugin->onReplacementCutsceneStarted();
        }
        if (status == QMediaPlayer::EndOfMedia) {
            asyncStopVideo();
        }
        if (status == QMediaPlayer::InvalidMedia) {
            emuInstance->plugin->errorLog("======= Error: %s", player->errorString().toStdString().c_str());
        }
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [=](QMediaPlayer::Error error) {
        emuInstance->plugin->errorLog("======= Error: %s", player->errorString().toStdString().c_str());
    });
#else
    connect(player.get(), &QMediaPlayer::errorOccurred, [=](QMediaPlayer::Error error, const QString &errorString) {
        emuInstance->plugin->errorLog("======= Error: %s", player->errorString().toStdString().c_str());
    });
#endif

    player->setVideoOutput(playerWidget.get());
    player->setAudioOutput(playerAudioOutput.get());
}

void MainWindowSettings::asyncStartVideo(QString videoFilePath)
{
    QMetaObject::invokeMethod(this, "startVideo", Qt::QueuedConnection, Q_ARG(QString, videoFilePath));
}

void MainWindowSettings::startVideo(QString videoFilePath)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    player->setMedia(QUrl::fromLocalFile(videoFilePath));
#else
    player->setSource(QUrl::fromLocalFile(videoFilePath));
#endif

    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    centralWidget->setCurrentWidget(playerWidget.get());

    int volume = localCfg.GetInt("Audio.Volume");
    playerAudioOutput->setVolume(volume / 256.0);

    player->play();
}

void MainWindowSettings::asyncStopVideo()
{
    QMetaObject::invokeMethod(this, "stopVideo", Qt::QueuedConnection);
}

void MainWindowSettings::stopVideo()
{
    bool isCutscenePlaying = player->playbackState() == QMediaPlayer::PlaybackState::PlayingState;
    if (isCutscenePlaying) {
        player->stop();
    }

    showGame();

    emuInstance->plugin->onReplacementCutsceneEnd();
}

void MainWindowSettings::asyncPauseVideo()
{
    QMetaObject::invokeMethod(this, "pauseVideo", Qt::QueuedConnection);
}

void MainWindowSettings::pauseVideo()
{
    player->pause();
}

void MainWindowSettings::asyncUnpauseVideo()
{
    QMetaObject::invokeMethod(this, "unpauseVideo", Qt::QueuedConnection);
}

void MainWindowSettings::unpauseVideo()
{
    player->play();
}

void MainWindowSettings::keyPressEvent(QKeyEvent* event)
{
    /*if (event->key() == Qt::Key_Escape) {
        QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();

        if (showingSettings) {
            if (player->playbackState() == QMediaPlayer::PlaybackState::PausedState) {
                player->play();
                centralWidget->setCurrentWidget(playerWidget);
            }
            else {
                showGame();
            }
        }
        else {
            bool isCutscenePlaying = player->playbackState() == QMediaPlayer::PlaybackState::PlayingState;
            if (isCutscenePlaying) {
                // player->stop();
                // asyncStopVideo();
                showingSettings = !showingSettings;
            }
            else {
                centralWidget->setCurrentWidget(settingsWidget);
            }
        }
        showingSettings = !showingSettings;
    }*/
}