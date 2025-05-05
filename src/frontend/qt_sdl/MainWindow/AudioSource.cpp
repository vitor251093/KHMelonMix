#include "AudioSource.h"
#include "AudioWavParser.h"

namespace melonMix {

AudioSourceWav::AudioSourceWav(QObject *parent)
    : QIODevice(parent)
{
}

bool AudioSourceWav::load(const QString &fileName)
{
    m_file.setFileName(fileName);
    if (!m_file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    if (!readWavHeader()) {
        m_file.close();
        return false;
    }

    bool bFoundMarkers = false;

    AudioWavParser cueReader;
    if (cueReader.readCuePoints(fileName.toStdString())) {
        auto loopStartSample = cueReader.getLoopMarkerSample(AudioWavParser::EMarker::Begin);
        auto loopEndSample = cueReader.getLoopMarkerSample(AudioWavParser::EMarker::End);
        if (loopStartSample >= 0 && loopEndSample >= 0 && loopEndSample > loopStartSample) {
            m_loopStartByte = m_dataStart + (loopStartSample * m_bytesPerFrame);
            m_loopEndByte = m_dataStart + (loopEndSample * m_bytesPerFrame);
            bFoundMarkers = true;
        }
    }

    if (!bFoundMarkers) {
        m_loopStartByte = m_dataStart;
        m_loopEndByte = m_dataStart + m_dataSize;
    }

    open(QIODevice::ReadOnly);
    return true;
}

void AudioSourceWav::onStarted(qint64 resumePosition, qreal volume, int fadeInMs)
{
    m_samplesRemaining = 0;
    m_totalTransitionSamples = 0;
    m_status = EStatus::Playing;
    volume = std::clamp(volume, 0.0, 1.0);

    m_currentPos = m_dataStart + resumePosition;

    if (fadeInMs > 0) {
        m_currentVolume = 0.0;
        startFadeIn(volume, fadeInMs);
    } else {
        m_targetVolume = m_currentVolume = volume;
    }
}

void AudioSourceWav::onStopped()
{
    m_currentPos = m_dataStart;
    m_status = EStatus::Stopped;

    m_currentVolume = 0.0;
    m_samplesRemaining = 0;
    m_totalTransitionSamples = 0;
}

qint64 AudioSourceWav::readData(char *data, qint64 maxSize)
{
    if (!m_file.isOpen()) {
        return 0;
    }

    qint64 currentPos = m_file.pos();

    if (currentPos > 0 && currentPos != m_currentPos) {
        m_file.seek(m_currentPos);
    }

    qint64 bytesToRead = maxSize;

    qint64 bytesRead = 0;

    if (currentPos <= m_loopEndByte && currentPos + bytesToRead > m_loopEndByte) {
        qint64 bytesBeforeLoop = m_loopEndByte - currentPos;

        bytesRead = m_file.read(data, bytesBeforeLoop);
        
        if (bytesRead != bytesBeforeLoop) {
            printf("Error or end of file\n");
            return bytesRead;
        }

        printf("Loop bgm to start sample: %d\n", m_loopStartByte);

        m_file.seek(m_loopStartByte);
        currentPos = m_file.pos();

        qint64 bytesAfterLoop = maxSize - bytesBeforeLoop;
        auto additionalRead = m_file.read(data + bytesBeforeLoop, bytesAfterLoop);
        bytesRead += additionalRead;
    } else {
        bytesRead = m_file.read(data, maxSize);

        if (bytesRead < maxSize) {
            m_file.seek(m_loopStartByte);
            bytesRead += m_file.read(data + bytesRead, maxSize - bytesRead);
        }
    }

    m_currentPos = m_file.pos();

    applyVolume(data, bytesRead);

    if (m_samplesRemaining <= 0 && m_status == EStatus::Stopping) {
        QMetaObject::invokeMethod(parent(), "onFadeOutCompleted", Qt::QueuedConnection);
        m_status = EStatus::Stopped;
    }

    return bytesRead;
}

void AudioSourceWav::startFadeIn(qreal volume, int durationMs)
{
    setVolume(volume, durationMs);
}

void AudioSourceWav::startFadeOut(int durationMs)
{
    setVolume(0.0, durationMs);
    m_status = EStatus::Stopping;
}

void AudioSourceWav::applyVolume(char* buffer, int64_t numBytes)
{
    float targetForCalculation = (m_targetVolume <= 0.0f) ? 0.000001f : m_targetVolume;

    char* samplePtr = buffer;

    int channel = 0;
    int remainingBytes = numBytes;
    while (remainingBytes > 0)
    {
        float gain = m_currentVolume;
        if (m_samplesRemaining > 0) {
            float position = ((m_totalTransitionSamples - m_samplesRemaining) / (float)m_totalTransitionSamples);

            if (m_targetVolume <= 0.0f) {
                gain = m_currentVolume * std::exp(-6.0f * position);
            } else {
                float dbCurrent = 20.0f * std::log10(m_currentVolume);
                float dbTarget = 20.0f * std::log10(targetForCalculation);
                float dbCurrent_i = dbCurrent + position * (dbTarget - dbCurrent);
                gain = std::pow(10.0f, dbCurrent_i / 20.0f);
            }
        }

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

void AudioSourceWav::setVolume(double newVolume, int durationMs)
{
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

qint64 AudioSourceWav::bytesAvailable() const
{
    return m_file.bytesAvailable() + QIODevice::bytesAvailable() + m_sampleRate; 
}

QAudioFormat::SampleFormat AudioSourceWav::bitsPerSampleToSampleFormat(quint16 bitsPerSample)
{
    switch(bitsPerSample)
    {
        case 8: return QAudioFormat::UInt8;
        case 16: return QAudioFormat::Int16;
        case 24: return QAudioFormat::Int32;
        case 32: return QAudioFormat::Int32;
        default: return QAudioFormat::Unknown; // Unhandled
    }
}

bool AudioSourceWav::readWavHeader()
{
    char riffHeader[12];
    if (m_file.read(riffHeader, 12) != 12) {
        return false;
    }

    if (memcmp(riffHeader, "RIFF", 4) != 0 || memcmp(riffHeader + 8, "WAVE", 4) != 0) {
        return false;
    }

    bool foundFormat = false;
    bool foundData = false;
    
    while (!foundData) {
        char chunkHeader[8];
        if (m_file.read(chunkHeader, 8) != 8) return false;
        
        char* chunkId = chunkHeader;
        qint32 chunkSize = *reinterpret_cast<qint32*>(chunkHeader + 4);
        
        if (memcmp(chunkId, "fmt ", 4) == 0) {
            QByteArray fmtData = m_file.read(chunkSize);
            if (fmtData.size() != chunkSize) return false;

            quint16 audioFormat = *reinterpret_cast<const quint16*>(fmtData.constData());
            m_numChannels = *reinterpret_cast<const quint16*>(fmtData.constData() + 2);
            m_sampleRate = *reinterpret_cast<const quint32*>(fmtData.constData() + 4);
            m_bitsPerSample = *reinterpret_cast<const quint16*>(fmtData.constData() + 14);

            m_bytesPerFrame = m_numChannels * (m_bitsPerSample / 8);

            m_format.setSampleRate(m_sampleRate);
            m_format.setChannelCount(m_numChannels);
            
            auto sampleFormat = bitsPerSampleToSampleFormat(m_bitsPerSample);
            m_format.setSampleFormat(sampleFormat);

            foundFormat = true;
        } else if (memcmp(chunkId, "data", 4) == 0) {
            m_dataStart = m_file.pos();
            m_currentPos = m_dataStart;
            m_dataSize = chunkSize;
            foundData = true;
        } else {
            m_file.skip(chunkSize);
        }

        if (chunkSize % 2 != 0) {
            m_file.skip(1);
        }
    }
    
    return foundFormat && foundData;
}

} // namespace melonMix