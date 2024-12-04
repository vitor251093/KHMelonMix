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


using namespace melonDS;

MainWindowSettings::MainWindowSettings(EmuInstance* inst, QWidget* parent) :
    QMainWindow(parent),
    emuInstance(inst),
    localCfg(inst->getLocalConfig()),
    ui(new Ui::MainWindowSettings)
{
    ui->setupUi(this);

    settingsWidget = findChild<QWidget*>("settingsWidget");
    settingWidgetOptions = findChild<QStackedWidget*>("settingWidgetOptions");
    showingSettings = false;
}

MainWindowSettings::~MainWindowSettings()
{
    delete ui;
}

void MainWindowSettings::initWidgets()
{
    createBgmPlayer();
    createVideoPlayer();
}

void MainWindowSettings::createBgmPlayer()
{
    bgmPlayerAudioOutput = new QAudioOutput(this);
    bgmPlayer = new QMediaPlayer(this);

    bgmPlayer->setAudioOutput(bgmPlayerAudioOutput);
}

void MainWindowSettings::asyncStartBgmMusic(QString bgmMusicFilePath)
{
    QMetaObject::invokeMethod(this, "startBgmMusic", Qt::QueuedConnection, Q_ARG(QString, bgmMusicFilePath));
}

void MainWindowSettings::startBgmMusic(QString bgmMusicFilePath)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bgmPlayer->setMedia(QUrl::fromLocalFile(bgmMusicFilePath));
#else
    bgmPlayer->setSource(QUrl::fromLocalFile(bgmMusicFilePath));
#endif

    int volume = localCfg.GetInt("Audio.Volume");
    bgmPlayerAudioOutput->setVolume(volume / 256.0);

    bgmPlayer->play();
}

void MainWindowSettings::asyncStopBgmMusic()
{
    QMetaObject::invokeMethod(this, "stopBgmMusic", Qt::QueuedConnection);
}

void MainWindowSettings::stopBgmMusic()
{
    bool isCutscenePlaying = bgmPlayer->playbackState() == QMediaPlayer::PlaybackState::PlayingState;
    if (isCutscenePlaying) {
        bgmPlayer->stop();
    }
}

void MainWindowSettings::createVideoPlayer()
{
    playerWidget = new QVideoWidget(this);
    playerWidget->setCursor(Qt::BlankCursor);

    playerAudioOutput = new QAudioOutput(this);
    player = new QMediaPlayer(this);

    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    centralWidget->addWidget(playerWidget);

    connect(player, &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
        emuInstance->plugin->log((std::string("======= MediaStatus: ") + std::to_string(status)).c_str());

        if (status == QMediaPlayer::BufferingMedia || status == QMediaPlayer::BufferedMedia) {
            emuInstance->plugin->onReplacementCutsceneStarted();
        }
        if (status == QMediaPlayer::EndOfMedia) {
            asyncStopVideo();
        }
        if (status == QMediaPlayer::InvalidMedia) {
            emuInstance->plugin->log(("======= Error: " + player->errorString().toStdString()).c_str());
        }
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [=](QMediaPlayer::Error error) {
        emuInstance->plugin->log(("======= Error: " + player->errorString().toStdString()).c_str());
    });
#else
    connect(player, &QMediaPlayer::errorOccurred, [=](QMediaPlayer::Error error, const QString &errorString) {
        emuInstance->plugin->log(("======= Error: " + player->errorString().toStdString()).c_str());
    });
#endif

    player->setVideoOutput(playerWidget);
    player->setAudioOutput(playerAudioOutput);
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
    centralWidget->setCurrentWidget(playerWidget);

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