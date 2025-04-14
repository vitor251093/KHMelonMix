#include "AudioPlayer.h"

#include "AudioSource.h"

#include <QAudioSink>
#include <QAudioFormat>
#include <QAudioSink>

namespace melonMix {

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_audioSource(new AudioSourceWav(this))
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

void AudioPlayer::setVolume(qreal value)
{
    if (m_audioOutput) {
        m_audioOutput->setVolume(value);
    }
}

void AudioPlayer::play()
{
    if (m_audioOutput) {
        m_audioSource->onStarted();
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

void AudioPlayer::onFadeOutCompleted()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        m_audioSource->onStopped();
        m_playing = false;
    }
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
