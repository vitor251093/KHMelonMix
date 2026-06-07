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
#include <QTimer>
#include <QFile>
#include <QGraphicsVideoItem>

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
    mediaDevices(new QMediaDevices()),
    playerAudioOutput(new QAudioOutput(this)),
    player(new QMediaPlayer(this))
{
    ui->setupUi(this);

    settingsWidget = findChild<QWidget*>("settingsWidget");
    settingWidgetOptions = findChild<QStackedWidget*>("settingWidgetOptions");
    showingSettings = false;

    connect(mediaDevices.get(), &QMediaDevices::audioOutputsChanged, this, &MainWindowSettings::onAudioOutputsChanged);
}

MainWindowSettings::~MainWindowSettings()
{
    disconnect(mediaDevices.get(), &QMediaDevices::audioOutputsChanged, this, &MainWindowSettings::onAudioOutputsChanged);
    if (m_sfxAudioDevice) {
        SDL_CloseAudioDevice(m_sfxAudioDevice);
        m_sfxAudioDevice = 0;
    }
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

void MainWindowSettings::onAudioOutputsChanged() {
    auto output = QMediaDevices::defaultAudioOutput();
    if (currentOutputDevice != output) {
        for(auto* bgmPlayer : bgmPlayers) {
            bgmPlayer->restartAudioSink(output);
        }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (player->state() != QMediaPlayer::State::StoppedState) {
#elif QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
        if (player->playbackState() != QMediaPlayer::PlaybackState::StoppedState) {
#else
        if (player->isPlaying()) {
#endif
            player->setAudioOutput(nullptr);
            playerAudioOutput.reset(new QAudioOutput(output, this));
            player->setAudioOutput(playerAudioOutput.get());
        }

        currentOutputDevice = output;
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
    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    playerView = new CutsceneVideoView(this);
    centralWidget->addWidget(playerView);

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

    player->setVideoOutput(playerView->videoItem());
    player->setAudioOutput(playerAudioOutput.get());

    createMenuSounds();
}

// Reads the raw PCM sample bytes from a 16-bit/44.1kHz/stereo WAV resource, parsing the
// RIFF chunks (so a non-canonical header / metadata chunk is handled gracefully).
static QByteArray loadWavPcm(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray all = f.readAll();
    f.close();

    auto u32 = [&](int o) {
        return (quint32)((quint8)all[o] | ((quint8)all[o + 1] << 8) |
                         ((quint8)all[o + 2] << 16) | ((quint32)(quint8)all[o + 3] << 24));
    };

    if (all.size() < 12 || all.left(4) != "RIFF" || all.mid(8, 4) != "WAVE") {
        return {};
    }

    int pos = 12;
    while (pos + 8 <= all.size()) {
        const QByteArray id = all.mid(pos, 4);
        const quint32 sz = u32(pos + 4);
        const int body = pos + 8;
        if (id == "data") {
            return all.mid(body, sz);
        }
        pos = body + sz + (sz & 1); // chunks are word-aligned
    }
    return {};
}

void MainWindowSettings::createMenuSounds()
{
    static const char* sources[5] = {
        nullptr,                       // 0: unused
        ":/ds/sfx_menu_enter.wav",     // 1: enter
        ":/ds/sfx_menu_move.wav",      // 2: move (up/down)
        ":/ds/sfx_menu_continue.wav",  // 3: continue
        ":/ds/sfx_menu_select.wav",    // 4: select
    };
    for (int i = 1; i < 5; i++) {
        m_sfxPcm[i] = loadWavPcm(QString::fromLatin1(sources[i]));
    }

    // Dedicated SDL audio device for menu SFX (the WAVs are s16le / 44.1kHz / stereo).
    SDL_AudioSpec want, got;
    memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16LSB;
    want.channels = 2;
    want.samples = 1024;
    want.callback = nullptr; // use the SDL_QueueAudio API instead of a callback
    m_sfxAudioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);
    if (m_sfxAudioDevice) {
        SDL_PauseAudioDevice(m_sfxAudioDevice, 0); // unpause: play queued audio
    }
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
    centralWidget->setCurrentWidget(playerView);

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
    // Stop regardless of whether the video is playing or paused, so a cutscene
    // skipped from the (paused) menu doesn't leave its audio playing.
    if (player->playbackState() != QMediaPlayer::PlaybackState::StoppedState) {
        player->stop();
    }

    hideCutsceneSkipMenu();

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

void MainWindowSettings::asyncShowCutsceneSkipMenu(int selection)
{
    QMetaObject::invokeMethod(this, "showCutsceneSkipMenu", Qt::QueuedConnection, Q_ARG(int, selection));
}

void MainWindowSettings::showCutsceneSkipMenu(int selection)
{
    if (!playerView) {
        return;
    }
    playerView->setMenuSelection(selection);
    playerView->setMenuVisible(true);
}

void MainWindowSettings::asyncUpdateCutsceneSkipMenu(int selection)
{
    QMetaObject::invokeMethod(this, "updateCutsceneSkipMenu", Qt::QueuedConnection, Q_ARG(int, selection));
}

void MainWindowSettings::updateCutsceneSkipMenu(int selection)
{
    if (!playerView) {
        return;
    }
    playerView->setMenuSelection(selection);
}

void MainWindowSettings::asyncHideCutsceneSkipMenu()
{
    QMetaObject::invokeMethod(this, "hideCutsceneSkipMenu", Qt::QueuedConnection);
}

void MainWindowSettings::hideCutsceneSkipMenu()
{
    if (playerView) {
        playerView->setMenuVisible(false);
    }
}

void MainWindowSettings::asyncPlayCutsceneMenuSound(int kind)
{
    QMetaObject::invokeMethod(this, "playCutsceneMenuSound", Qt::QueuedConnection, Q_ARG(int, kind));
}

void MainWindowSettings::playCutsceneMenuSound(int kind)
{
    if (kind < 1 || kind > 4 || m_sfxPcm[kind].isEmpty() || !m_sfxAudioDevice) {
        return;
    }

    // Scale to the configured output volume (SDL mix volume is 0..128).
    int vol = qBound(0, localCfg.GetInt("Audio.Volume") / 2, SDL_MIX_MAXVOLUME);
    const QByteArray& pcm = m_sfxPcm[kind];
    QByteArray scaled(pcm.size(), '\0');
    SDL_MixAudioFormat(reinterpret_cast<Uint8*>(scaled.data()),
                       reinterpret_cast<const Uint8*>(pcm.constData()),
                       AUDIO_S16LSB, (Uint32)pcm.size(), vol);

    // Clear any still-queued clip so a new press restarts immediately (responsive nav).
    SDL_ClearQueuedAudio(m_sfxAudioDevice);
    SDL_QueueAudio(m_sfxAudioDevice, scaled.constData(), (Uint32)scaled.size());
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