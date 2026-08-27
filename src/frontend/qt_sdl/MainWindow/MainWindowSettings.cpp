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

#include <cstring>

#include <SDL2/SDL.h>

#include "types.h"
#include "Platform.h"

#include "MainWindowSettings.h"
#include "SettingsView.h"
#include "ui_MainWindowSettings.h"

#include "EmuInstance.h"
#include "AudioSource.h"
#include "AudioPlayer.h"

using namespace melonDS;

namespace {

// Some HD cutscene MP4s ship with their first 256 bytes XORed against a fixed, repeating
// 16-byte key (a KH HD Remix asset-scrambling quirk), so they start with neither a "ftyp"
// nor a "moov" box and QMediaPlayer refuses to load them as InvalidMedia. XOR is its own
// inverse, so re-applying the same key recovers the real header.
constexpr unsigned char kMp4HeaderKey[16] = {
    0x3f, 0x1e, 0xc1, 0x93, 0x26, 0x42, 0x58, 0xb8,
    0xaa, 0x9d, 0x53, 0x75, 0xbf, 0x62, 0x9a, 0x97
};
constexpr qint64 kMp4HeaderFixLen = 256;

bool isKnownIsoBoxType(const char* type)
{
    static const char* const known[] = {
        "ftyp", "moov", "free", "skip", "wide", "mdat", "pnot", "uuid", "meta", "moof", "mfra", "styp"
    };
    for (const char* k : known) {
        if (memcmp(type, k, 4) == 0) {
            return true;
        }
    }
    return false;
}

// Peeks at the box type right after the first box's size field (bytes 4..8) and checks
// whether un-XORing it with kMp4HeaderKey turns it into a recognized ISO base media box
// type. Files that are already fine (or aren't MP4 at all) are left untouched.
bool mp4NeedsHeaderFix(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    char buf[8];
    if (f.read(buf, sizeof(buf)) != (qint64)sizeof(buf)) {
        return false;
    }
    if (isKnownIsoBoxType(buf + 4)) {
        return false;
    }
    char fixed[4];
    for (int i = 0; i < 4; i++) {
        fixed[i] = buf[4 + i] ^ (char)kMp4HeaderKey[(4 + i) % 16];
    }
    return isKnownIsoBoxType(fixed);
}

// Wraps a local file and un-XORs the scrambled header range on the fly while reading, so
// QMediaPlayer sees a valid MP4 without the file ever being rewritten on disk. Everything
// past kMp4HeaderFixLen is passed through unchanged, and reads are served straight off
// disk (no whole-file buffering), so this is cheap even for the largest cutscenes.
class ScrambledMp4Device : public QIODevice
{
public:
    explicit ScrambledMp4Device(const QString& path, QObject* parent = nullptr)
        : QIODevice(parent), m_file(path) {}

    bool open(QIODevice::OpenMode mode) override
    {
        if (!m_file.open(QIODevice::ReadOnly)) {
            return false;
        }
        return QIODevice::open(mode | QIODevice::ReadOnly);
    }

    void close() override
    {
        m_file.close();
        QIODevice::close();
    }

    bool isSequential() const override { return false; }
    qint64 size() const override { return m_file.size(); }
    qint64 pos() const override { return m_file.pos(); }
    bool atEnd() const override { return m_file.atEnd(); }
    qint64 bytesAvailable() const override { return m_file.bytesAvailable(); }

    bool seek(qint64 pos) override
    {
        if (!m_file.seek(pos)) {
            return false;
        }
        return QIODevice::seek(pos);
    }

protected:
    qint64 readData(char* data, qint64 maxSize) override
    {
        const qint64 startPos = m_file.pos();
        const qint64 n = m_file.read(data, maxSize);
        if (n <= 0) {
            return n;
        }
        const qint64 fixEnd = qMin(startPos + n, kMp4HeaderFixLen);
        for (qint64 p = startPos; p < fixEnd; p++) {
            data[p - startPos] ^= kMp4HeaderKey[p % 16];
        }
        return n;
    }

    qint64 writeData(const char*, qint64) override { return -1; }

private:
    QFile m_file;
};

} // namespace

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
    createSettingsView();
}

void MainWindowSettings::createSettingsView()
{
    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    settingsView = new SettingsView(this);
    centralWidget->addWidget(settingsView);
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

    for (auto* player : bgmPlayers) {
        if (player->getBgmId() == bgmId && player->isPlaying()) {
            return;
        }
    }

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

    if (ramVolume == 0x40) {
        // Volume is decreased when paused or during cutscenes
        volume *= 0.7;
    }

    return (volume / 100.0);
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
    printf("BGM players remaining after cleanup: %d\n", (int)bgmPlayers.size());
}

void MainWindowSettings::createVideoPlayer()
{
    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    playerView = new CutsceneVideoView(this);
    centralWidget->addWidget(playerView);

    connect(player.get(), &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
        printf("======= MediaStatus: %d\n", status);

        if (status == QMediaPlayer::EndOfMedia) {
            asyncStopVideo();
        }
        if (status == QMediaPlayer::InvalidMedia) {
            emuInstance->plugin->errorLog("======= Error: %d", status);
        }
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [=](QMediaPlayer::Error error) {
        emuInstance->plugin->errorLog("======= Error: %s", player->errorString().toStdString().c_str());
        cancelVideo(player->errorString().toStdString());
    });
#else
    connect(player.get(), &QMediaPlayer::errorOccurred, [=](QMediaPlayer::Error error, const QString &errorString) {
        emuInstance->plugin->errorLog("======= Error: %s", player->errorString().toStdString().c_str());
        cancelVideo(player->errorString().toStdString());
    });
#endif

    // Drive subtitle cue switching off the playback clock. positionChanged fires roughly
    // per frame during playback; the view only repaints when the active cue actually changes.
    connect(player.get(), &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        if (playerView) {
            playerView->setPlaybackPosition(pos);
        }
    });

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

void MainWindowSettings::asyncStartVideo(QString videoFilePath, QString subtitlesFilePath, int menuLanguage)
{
    QMetaObject::invokeMethod(this, "startVideo", Qt::QueuedConnection,
        Q_ARG(QString, videoFilePath), Q_ARG(QString, subtitlesFilePath), Q_ARG(int, menuLanguage));
}

void MainWindowSettings::startVideo(QString videoFilePath, QString subtitlesFilePath, int menuLanguage)
{
    playerSourceDevice.reset();
    if (mp4NeedsHeaderFix(videoFilePath)) {
        playerSourceDevice.reset(new ScrambledMp4Device(videoFilePath, this));
        if (!playerSourceDevice->open(QIODevice::ReadOnly)) {
            playerSourceDevice.reset();
        }
    }

    const QUrl videoUrl = QUrl::fromLocalFile(videoFilePath);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (playerSourceDevice) {
        player->setMedia(videoUrl, playerSourceDevice.get());
    } else {
        player->setMedia(videoUrl);
    }
#else
    if (playerSourceDevice) {
        player->setSourceDevice(playerSourceDevice.get(), videoUrl);
    } else {
        player->setSource(videoUrl);
    }
#endif

    // Load subtitles for this cutscene (an empty path clears any previous cues).
    playerView->loadSubtitles(subtitlesFilePath);
    // Localize the pause menu to match the cutscene's language.
    playerView->setMenuLanguage(menuLanguage);

    QStackedWidget* centralWidget = (QStackedWidget*)this->centralWidget();
    centralWidget->setCurrentWidget(playerView);

    int volume = localCfg.GetInt("Audio.Volume");
    playerAudioOutput->setVolume(volume / 256.0);

    player->play();
}

void MainWindowSettings::cancelVideo(std::string error)
{
    // Stop regardless of whether the video is playing or paused, so a cutscene
    // skipped from the (paused) menu doesn't leave its audio playing.
    if (player->playbackState() != QMediaPlayer::PlaybackState::StoppedState) {
        player->stop();
    }
    playerSourceDevice.reset();

    hideCutscenePauseMenu();

    // Drop any subtitle cues so a finished cutscene doesn't leave stale text behind.
    playerView->loadSubtitles("");

    showGame();

    emuInstance->plugin->resumeIngamePrerenderedCutsceneAfterReplacementCutsceneFailedToPlay(error);
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
    playerSourceDevice.reset();

    hideCutscenePauseMenu();

    // Drop any subtitle cues so a finished cutscene doesn't leave stale text behind.
    playerView->loadSubtitles("");

    showGame();

    emuInstance->plugin->skipIngamePrerenderedCutsceneAfterReplacementCutsceneFinishesNaturally();
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

bool MainWindowSettings::isVideoPlaying() const
{
    return player && player->playbackState() == QMediaPlayer::PlaybackState::PlayingState;
}

bool MainWindowSettings::isVideoPaused() const
{
    return player && player->playbackState() == QMediaPlayer::PlaybackState::PausedState;
}

void MainWindowSettings::asyncShowCutscenePauseMenu(int selection)
{
    QMetaObject::invokeMethod(this, "showCutscenePauseMenu", Qt::QueuedConnection, Q_ARG(int, selection));
}

void MainWindowSettings::showCutscenePauseMenu(int selection)
{
    if (!playerView) {
        return;
    }
    playerView->setMenuSizeModifier(emuInstance->plugin->getHudScale()/8.0);
    playerView->setMenuSelection(selection);
    playerView->setMenuVisible(true);
}

void MainWindowSettings::asyncUpdateCutscenePauseMenu(int selection)
{
    QMetaObject::invokeMethod(this, "updateCutscenePauseMenu", Qt::QueuedConnection, Q_ARG(int, selection));
}

void MainWindowSettings::updateCutscenePauseMenu(int selection)
{
    if (!playerView) {
        return;
    }
    playerView->setMenuSelection(selection);
}

void MainWindowSettings::asyncHideCutscenePauseMenu()
{
    QMetaObject::invokeMethod(this, "hideCutscenePauseMenu", Qt::QueuedConnection);
}

void MainWindowSettings::hideCutscenePauseMenu()
{
    if (playerView) {
        playerView->setMenuVisible(false);
    }
}

void MainWindowSettings::asyncPlayMenuSound(int kind)
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
}