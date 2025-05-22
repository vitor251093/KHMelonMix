#include "AudioSourceWav.h"
#include "AudioWavParser.h"

namespace melonMix {

AudioSourceWav::AudioSourceWav(QObject *parent)
    : AudioSource(parent) {
}

bool AudioSourceWav::load(const QString &fileName) {
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

bool AudioSourceWav::isFileLoaded() const {
    return m_file.isOpen();
}

qint64 AudioSourceWav::getCurrentPlayingPos(qint64 audioSinkPlayTime) const {
    qint64 playTime = audioSinkPlayTime + m_resumedPos;
    if (m_loopEndByte > m_loopStartByte) {
        qint64 oneLoopDuration = byte_pos_to_pos_ms(m_loopEndByte - m_loopStartByte);
        playTime -= (m_loopsPlayed * oneLoopDuration);
    }
    return playTime;
}

void AudioSourceWav::onStarted(qint64 resumePosition, qreal volume, int fadeInMs) {
    m_currentPos = m_dataStart + pos_ms_to_byte_pos(resumePosition);
    m_resumedPos = resumePosition;
    m_loopsPlayed = 0;
    AudioSource::onStarted(resumePosition, volume, fadeInMs);
}

void AudioSourceWav::onStopped() {
    m_currentPos = m_dataStart;
    m_resumedPos = 0;
    m_loopsPlayed = 0;
    AudioSource::onStopped();
}

qint64 AudioSourceWav::readData(char *data, qint64 maxSize) {
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
        m_loopsPlayed++;

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

    return bytesRead;
}

bool AudioSourceWav::readWavHeader() {
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
            if (sampleFormat != QAudioFormat::Unknown) {
                m_format.setSampleFormat(sampleFormat);
                foundFormat = true;
            }
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
