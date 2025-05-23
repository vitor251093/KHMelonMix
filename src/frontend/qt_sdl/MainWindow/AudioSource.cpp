#include "AudioSource.h"

namespace melonMix {

AudioSource::AudioSource(QObject *parent)
    : QIODevice(parent)
{
}

qint64 AudioSource::byte_pos_to_sample_pos(qint64 byte_pos) const {
    return byte_pos / (m_numChannels * (m_bitsPerSample / 8));
}

qint64 AudioSource::sample_pos_to_byte_pos(qint64 sample_pos) const {
    return sample_pos * (m_numChannels * (m_bitsPerSample / 8));
}

qint64 AudioSource::pos_ms_to_byte_pos(qint64 pos_ms) const {
    qint64 sample_pos = m_sampleRate * pos_ms / 1000;
    return sample_pos_to_byte_pos(sample_pos);
}

qint64 AudioSource::byte_pos_to_pos_ms(qint64 byte_pos) const {
    qint64 sample_pos = byte_pos_to_sample_pos(byte_pos);
    return (sample_pos * 1000) / m_sampleRate;
}

void AudioSource::onStarted(qint64 resumePosition, qreal volume, int fadeInMs) {
    m_samplesRemaining = 0;
    m_totalTransitionSamples = 0;
    m_status = EStatus::Playing;
    m_stoppingDelay = 0;
    volume = std::clamp(volume, 0.0, 1.0);

    if (fadeInMs > 0) {
        m_currentVolume = 0.0;
        startFadeIn(volume, fadeInMs);
    } else {
        m_targetVolume = m_currentVolume = volume;
    }
}

void AudioSource::onStopped() {
    m_status = EStatus::Stopped;
    m_stoppingDelay = 0;

    m_currentVolume = 0.0;
    m_samplesRemaining = 0;
    m_totalTransitionSamples = 0;
}


void AudioSource::startFadeIn(qreal volume, int durationMs) {
    setVolume(volume, durationMs);
}

void AudioSource::startFadeOut(int durationMs) {
    setVolume(0.0, durationMs);
    m_status = EStatus::Stopping;
    m_stoppingDelay = m_sampleRate; // 1 sec delay before destroying source
}

void AudioSource::applyVolume(char* buffer, int64_t numSamples) {
    if (m_samplesRemaining <= 0 && m_status == EStatus::Stopping) {
        if (m_stoppingDelay > 0) {
            m_stoppingDelay -= numSamples;
        } else {
            QMetaObject::invokeMethod(parent(), "onFadeOutCompleted", Qt::QueuedConnection);
            m_status = EStatus::Stopped;
        }
    }

    float targetForCalculation = (m_targetVolume <= 0.0f) ? 0.000001f : m_targetVolume;

    char* samplePtr = buffer;

    int channel = 0;
    int remainingBytes = numSamples;
    while (remainingBytes > 0)
    {
        float gain = m_currentVolume;

        if (m_bitsPerSample == 16) {
            int16_t value = (static_cast<uint8_t>(samplePtr[0])) |
                            (static_cast<int8_t>(samplePtr[1]) << 8);

            value *= gain;

            samplePtr[0] = value & 0xFF;
            samplePtr[1] = (value >> 8) & 0xFF;
        } 
        else if (m_bitsPerSample == 24) {
            int32_t value = 0;
            value = (static_cast<uint8_t>(samplePtr[0])) |
                    (static_cast<uint8_t>(samplePtr[1]) << 8) |
                    (static_cast<int8_t>(samplePtr[2]) << 16);

            value = static_cast<int32_t>(value * gain);

            samplePtr[0] = value & 0xFF;
            samplePtr[1] = (value >> 8) & 0xFF;
            samplePtr[2] = (value >> 16) & 0xFF;
        }
        else if (m_bitsPerSample == 32) {
            int32_t* sample = reinterpret_cast<int32_t*>(samplePtr);
            *sample = static_cast<int32_t>(*sample * gain);
        } else if (m_bitsPerSample == 8) {
            uint8_t* sample = reinterpret_cast<uint8_t*>(samplePtr);
            *sample = 128 + static_cast<uint8_t>((*sample - 128) * gain);
        }

        int sampleSize = m_bitsPerSample / 8;
        samplePtr += sampleSize;
        remainingBytes -= sampleSize;

        channel++;
        if (channel == m_numChannels)
        {
            if (m_samplesRemaining > 0) {
                m_currentVolume += m_volumeIncrement;
                m_samplesRemaining--;
            }  else {
                m_currentVolume = m_targetVolume;
            }
            channel = 0;
        }
    }
}

void AudioSource::setVolume(double newVolume, int durationMs) {
    if (m_status != EStatus::Playing) {
        return;
    }

    newVolume = std::clamp(newVolume, 0.0, 1.0);
    m_targetVolume = newVolume;

    int64_t totalSamples = static_cast<int64_t>(durationMs * m_sampleRate / 1000);
    
    if (totalSamples <= 0 || m_currentVolume == m_targetVolume) {
        m_currentVolume = m_targetVolume;
        m_samplesRemaining = 0;
        m_totalTransitionSamples = 0;
        m_volumeIncrement = 0.0;
    } else {
        m_volumeIncrement = (m_targetVolume - m_currentVolume) / totalSamples;
        m_samplesRemaining = totalSamples;
        m_totalTransitionSamples = totalSamples;
    }
}

QAudioFormat::SampleFormat AudioSource::bitsPerSampleToSampleFormat(quint16 bitsPerSample) {
    switch(bitsPerSample)
    {
        case 8: return QAudioFormat::UInt8;
        case 16: return QAudioFormat::Int16;
        //case 24: return QAudioFormat::Int32; // Unhandled
        case 32: return QAudioFormat::Int32;
        default: return QAudioFormat::Unknown; // Unhandled
    }
}

} // namespace melonMix
