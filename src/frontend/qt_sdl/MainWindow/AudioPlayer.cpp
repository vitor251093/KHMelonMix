#include "AudioPlayer.h"

#include "AudioSourceWav.h"
#include "AudioSourceFlac.h"

#include <QAudioSink>
#include <QAudioFormat>
#include <QAudioSink>
#include <QFileInfo>
#include <QMediaDevices>

namespace melonMix {

AudioPlayer::AudioPlayer(QObject *parent, quint16 bgmId)
    : QObject(parent)
    , m_bgmId(bgmId)
{
}

bool AudioPlayer::loadFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        return false;
    }

    m_audioSource.reset();

    if (fileInfo.suffix().toLower() == "wav") {
        m_audioSource.reset(new AudioSourceWav(this));
    }
#ifdef AUDIO_FLAC_ENABLED
    else if (fileInfo.suffix().toLower() == "flac") {
        m_audioSource.reset(new AudioSourceFlac(this));
    }
#endif

    if (m_audioSource && m_audioSource->load(fileName)) {
        createAudioSink(QMediaDevices::defaultAudioOutput());
        return true;
    } else {
        return false;
    }
}

void AudioPlayer::createAudioSink(const QAudioDevice& audioOutput) {
    QAudioFormat format;
    format.setSampleRate(m_audioSource->getSampleRate());
    format.setChannelCount(m_audioSource->getNumChannels());

    auto sampleFormat = AudioSourceWav::bitsPerSampleToSampleFormat(m_audioSource->getBitsPerSample());
    format.setSampleFormat(sampleFormat);

    m_audioOutput.reset(new QAudioSink(audioOutput, format, this));
}

void AudioPlayer::restartAudioSink(const QAudioDevice& audioOutput) {
    if (m_playing && m_audioSource) {
        m_audioOutput->stop();
        createAudioSink(audioOutput);
        m_audioOutput->start(m_audioSource.get());
    }
}

void AudioPlayer::setVolume(qreal value, int durationInMs)
{
    if (m_audioOutput) {
        m_audioSource->setVolume(value, durationInMs);
    }
}

void AudioPlayer::play(qint64 resumePosition, qreal volume, int fadeInMs)
{
    if (m_audioOutput) {
        m_audioSource->onStarted(resumePosition, volume, fadeInMs);
        m_audioOutput->start(m_audioSource.get());
        m_playing = true;
    }
}

void AudioPlayer::stop(int fadeOutMs)
{
    if (!m_audioOutput) {
        return;
    }
    if (fadeOutMs == 0) {
        onFadeOutCompleted();
    } else {
        m_audioSource->startFadeOut(fadeOutMs);
    }
}

qint64 AudioPlayer::getCurrentPlayingPos() const
{
    qint64 playTimeMs = m_audioOutput->elapsedUSecs() / 1000;
    return m_audioSource->getCurrentPlayingPos(playTimeMs);
}

void AudioPlayer::onFadeOutCompleted()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        m_audioSource->onStopped();
        m_playing = false;
    }

    QMetaObject::invokeMethod(parent(), "onBgmFadeOutCompleted", Qt::QueuedConnection, Q_ARG(AudioPlayer*, this));
}

void AudioPlayer::pause()
{
    if (m_audioOutput) {
        m_audioOutput->suspend();
    }
}

void AudioPlayer::resume()
{
    if (m_audioOutput) {
        m_audioOutput->resume();
    }
}

} // namespace melonMix
