#include "AudioPlayer.h"

#include "AudioSource.h"

#include <QAudioSink>
#include <QAudioFormat>
#include <QAudioSink>

namespace melonMix {

AudioPlayer::AudioPlayer(QObject *parent, quint16 bgmId)
    : QObject(parent)
    , m_audioSource(new AudioSourceWav(this))
    , m_bgmId(bgmId)
{
}

bool AudioPlayer::loadFile(const QString &fileName)
{
    if (!m_audioSource->load(fileName)) {
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(m_audioSource->getSampleRate());
    format.setChannelCount(m_audioSource->getNumChannels());

    auto sampleFormat = AudioSourceWav::bitsPerSampleToSampleFormat(m_audioSource->getBitsPerSample());
    format.setSampleFormat(sampleFormat);

    m_audioOutput.reset(new QAudioSink(format, this));

    return true;
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

    m_audioSource->startFadeOut(fadeOutMs);
}

qint64 AudioPlayer::getCurrentPlayingPos() const
{
    return m_audioSource->getCurrentPlayingPos();
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
