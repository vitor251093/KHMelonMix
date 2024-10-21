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
    createVideoPlayer();
}

void MainWindowSettings::createVideoPlayer()
{
    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();

    playerWidgetArea = new QWidget();
    playerWidgetArea->setContentsMargins(0, 0, 0, 0);
    playerWidgetArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centralWidget->addWidget(playerWidgetArea);

    playerWidgetAreaLayout = new QGridLayout(playerWidgetArea);
    playerWidgetAreaLayout->setContentsMargins(0, 0, 0, 0);

    QGraphicsView* playerGraphicsView = new QGraphicsView;
    QGraphicsScene* playerGraphicsScene = new QGraphicsScene(playerGraphicsView);
    // playerGraphicsView->setSceneRect(centralWidget->geometry());
    playerGraphicsView->setScene(playerGraphicsScene);
    playerWidgetAreaLayout->addWidget(playerGraphicsView, 0, 0, 1, 1);

    QGraphicsVideoItem* playerGraphicsVideoItem = new QGraphicsVideoItem;
    playerGraphicsScene->addItem(playerGraphicsVideoItem);

    subtitleWidget = new QWidget(this);
    subtitleWidget->setContentsMargins(0, 0, 0, 0);
    subtitleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    subtitleWidgetLayout = new QGridLayout(subtitleWidget);
    subtitleWidgetLayout->setContentsMargins(0, 0, 0, 0);

    subtitleLabel = new QLabel();
    subtitleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet("background-color: transparent; color: white; font-size: 24px;");
    subtitleWidgetLayout->addWidget(subtitleLabel, 0, 0, 1, 1);

    playerAudioOutput = new QAudioOutput(this);
    player = new QMediaPlayer();

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

    player->setVideoOutput(playerGraphicsVideoItem);
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
    centralWidget->setCurrentWidget(playerGraphicsView);

    int playerWidth = centralWidget->geometry().width();
    int playerHeight = centralWidget->geometry().height();

    playerWidgetArea->setGeometry(0, 0, playerWidth, playerHeight);
    playerWidgetArea->setVisible(true);

    // playerGraphicsView->setSceneRect(centralWidget->geometry());
    // playerGraphicsVideoItem->setSize(QSizeF(playerWidth, playerHeight));
    subtitleWidget->setGeometry(0, (playerHeight*5)/6, playerWidth, playerHeight/6);
    subtitleWidget->show();

    subtitleLabel->setText("example subtitle");

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

    playerWidgetArea->setVisible(false);
    subtitleWidget->hide();

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
                centralWidget->setCurrentWidget(playerGraphicsView);
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