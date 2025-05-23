#include "AudioSourceFlac.h"
#include <QFile>
#include <QDebug>

#ifdef AUDIO_FLAC_ENABLED

namespace melonMix {

AudioSourceFlac::AudioSourceFlac(QObject *parent)
    : AudioSource(parent) {
}

AudioSourceFlac::~AudioSourceFlac() {
    close();
}

bool AudioSourceFlac::open(QIODevice::OpenMode mode) {
    if (!m_fileLoaded) {
        printf("Cannot open device: No FLAC file loaded\n");
        return false;
    }
    
    if ((mode & QIODevice::WriteOnly) != 0) {
        printf("AudioSourceFlac is read-only\n");
        return false;
    }

    m_currentSample = 0;
    m_loopsPlayed = 0;
    m_bufferPosition = 0;
    m_bufferSize = 0;
    m_lastDecodedSample = 0;

    if (!initializeDecoder()) {
        printf("Failed to initialize FLAC decoder\n");
        return false;
    }

    fillBuffer();
    
    return QIODevice::open(mode);
}

void AudioSourceFlac::close() {
    if (QIODevice::isOpen()) {
        cleanupDecoder();
        QIODevice::close();
    }
}

void AudioSourceFlac::onStarted(qint64 resumePosition, qreal volume, int fadeInMs) {
    m_loopsPlayed = 0;
    m_resumedPos = resumePosition;
    if (resumePosition > 0) {
        qint64 sample_pos = m_sampleRate * resumePosition / 1000;
        seekInternal(sample_pos);
    }
    AudioSource::onStarted(resumePosition, volume, fadeInMs);
}

void AudioSourceFlac::onStopped() {
    m_resumedPos = 0;
    m_loopsPlayed = 0;
    AudioSource::onStopped();
}

qint64 AudioSourceFlac::getCurrentPlayingPos(qint64 audioSinkPlayTime) const {
    qint64 playTime = audioSinkPlayTime + m_resumedPos;
    if (m_loopEndSample > m_loopStartSample) {
        qint64 oneLoopDuration = ((m_loopEndSample - m_loopStartSample) * 1000) / m_sampleRate;
        playTime -= (m_loopsPlayed * oneLoopDuration);
    }
    return playTime;
}

bool AudioSourceFlac::seekInternal(qint64 samplePos) {
    if (!m_fileLoaded || !QIODevice::isOpen())
        return false;

    if (samplePos > m_totalSamples)
        samplePos = m_totalSamples;

    m_bufferPosition = 0;
    m_bufferSize = 0;

    cleanupDecoder();
    if (!initializeDecoder()) {
        printf("Failed to reinitialize decoder for seek\n");
        return false;
    }
    
    if (!FLAC__stream_decoder_seek_absolute(m_decoder, samplePos)) {
        printf("Failed to seek in FLAC file\n");
        return false;
    }

    m_currentSample = samplePos;
    m_lastDecodedSample = samplePos;

    return true;
}

bool AudioSourceFlac::load(const QString &fileName) {
    close();

    m_fileLoaded = false;
    m_fileName = fileName;

    FLAC__StreamDecoder* tempDecoder = createNewDecoder();
    if (tempDecoder == nullptr)
        return false;

    FLAC__stream_decoder_delete(tempDecoder);

    m_loopStartSample = 0;
    m_loopEndSample = m_totalSamples;
    
    m_fileLoaded = true;
    m_currentSample = 0;
    m_lastDecodedSample = 0;

    open(QIODevice::ReadOnly);
    return true;
}

bool AudioSourceFlac::isFileLoaded() const {
    return (m_fileLoaded && m_decoder);
}

FLAC__StreamDecoderWriteStatus AudioSourceFlac::writeCallback(
    const FLAC__StreamDecoder *decoder, 
    const FLAC__Frame *frame, 
    const FLAC__int32 * const buffer[], 
    void *clientData)
{
    AudioSourceFlac* device = static_cast<AudioSourceFlac*>(clientData);
    if (!device || !frame)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    int bytesPerSample = device->m_bitsPerSample / 8;

    qint64 frameSize = frame->header.blocksize * frame->header.channels * bytesPerSample;

    if (device->m_buffer.size() < frameSize)
        device->m_buffer.resize(frameSize);

    device->m_bufferSize = frameSize;
    device->m_bufferPosition = 0;

    char *outputPtr = device->m_buffer.data();

    for (unsigned i = 0; i < frame->header.blocksize; i++) {
        for (unsigned j = 0; j < frame->header.channels; j++) {
            if (device->m_bitsPerSample == 8) {
                quint8 sample = (buffer[j][i] >> 24) + 128; // Convert to unsigned
                *outputPtr++ = sample;
            } else if (device->m_bitsPerSample == 16) {
                qint16 sample = qBound<qint16>((qint16)-32768, (qint16)buffer[j][i], (qint16)32767);
                memcpy(outputPtr, &sample, 2);
                outputPtr += 2;
            } else if (device->m_bitsPerSample == 24) {
                qint32 sample = qBound<qint32>((qint32)-8388608, (qint32)buffer[j][i], (qint32)8388607);
                *outputPtr++ = sample & 0xFF;
                *outputPtr++ = (sample >> 8) & 0xFF;
                *outputPtr++ = (sample >> 16) & 0xFF;
            } else if (device->m_bitsPerSample == 32) {
                qint32 sample = buffer[j][i];
                memcpy(outputPtr, &sample, 4);
                outputPtr += 4;
            }
        }
    }

    device->m_lastDecodedSample = frame->header.number.sample_number;
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void AudioSourceFlac::metadataCallback(
    const FLAC__StreamDecoder *decoder, 
    const FLAC__StreamMetadata *metadata, 
    void *clientData)
{
    AudioSourceFlac *device = static_cast<AudioSourceFlac*>(clientData);
    if (!device || !metadata)
        return;
    
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        const FLAC__StreamMetadata_StreamInfo &info = metadata->data.stream_info;
        device->m_totalSamples = info.total_samples;
        device->m_numChannels = info.channels;
        device->m_sampleRate = info.sample_rate;
        device->m_bitsPerSample = info.bits_per_sample;
    }
    else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        const FLAC__StreamMetadata_VorbisComment &vc = metadata->data.vorbis_comment;

        for (unsigned i = 0; i < vc.num_comments; i++) {
            const char *entry = reinterpret_cast<const char*>(vc.comments[i].entry);
            const unsigned length = vc.comments[i].length;
            QString comment = QString::fromUtf8(entry, length);

            if (comment.startsWith("LOOPSTART=", Qt::CaseInsensitive)) {
                bool success;
                qint64 loopStart = comment.mid(10).toLongLong(&success);
                if (success) {
                    device->m_loopStartSample = loopStart;
                }
            }
            else if (comment.startsWith("LOOPEND=", Qt::CaseInsensitive)) {
                bool success;
                qint64 loopEnd = comment.mid(8).toLongLong(&success);
                if (success) {
                    device->m_loopEndSample = loopEnd;
                }
            }
        }
    }
}

void AudioSourceFlac::errorCallback(
    const FLAC__StreamDecoder *decoder, 
    FLAC__StreamDecoderErrorStatus status, 
    void *clientData) {
    AudioSourceFlac* device = static_cast<AudioSourceFlac*>(clientData);
    if (device) {
        printf("FLAC decoder error: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
    }
}

FLAC__StreamDecoder* AudioSourceFlac::createNewDecoder() {
    FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
    if (!decoder) {
        printf("Failed to create FLAC decoder\n");
        return nullptr;
    }

    FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

    FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_file(
            decoder,
            m_fileName.toUtf8().constData(),
            writeCallback,
            metadataCallback,
            errorCallback,
            this);

    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        printf("Failed to initialize FLAC decoder: %s\n", FLAC__StreamDecoderInitStatusString[initStatus]);
        FLAC__stream_decoder_delete(decoder);
        return nullptr;
    }

    if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
        printf("Failed to process FLAC metadata\n");
        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);
        return nullptr;
    }

    return decoder;
}

bool AudioSourceFlac::initializeDecoder() {
    if (m_decoder) {
        cleanupDecoder();
    }

    m_decoder = createNewDecoder();
    return (m_decoder != nullptr);
}

void AudioSourceFlac::cleanupDecoder() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder);
        m_decoder = nullptr;
    }
}

bool AudioSourceFlac::decodeUntilFrame(qint64 targetSample) {
    if (!m_decoder)
        return false;
        
    while (m_lastDecodedSample < targetSample) {
        if (!FLAC__stream_decoder_process_single(m_decoder)) {
            printf("Error processing FLAC frame\n");
            return false;
        }
        
        if (FLAC__stream_decoder_get_state(m_decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
            // End of stream reached
            return false;
        }
    }
    
    return true;
}

void AudioSourceFlac::fillBuffer() {
    if (!m_decoder)
        return;

    if (!FLAC__stream_decoder_process_single(m_decoder)) {
        printf("Error processing FLAC frame\n");
        return;
    }

    if (FLAC__stream_decoder_get_state(m_decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
        printf("End of FLAC stream reached\n");
    }
}

qint64 AudioSourceFlac::readData(char* data, qint64 maxSize) {
    if (!m_fileLoaded || !data || maxSize <= 0 || !m_decoder)
        return 0;

    qint64 totalBytesRead = 0;
    qint64 bytesPerSample = m_numChannels * (m_bitsPerSample / 8);

    while (totalBytesRead < maxSize) {
        if (m_currentSample >= m_loopEndSample) {
            printf("Looping back to sample:%d (from:%d)\n", m_loopStartSample, m_currentSample);
            m_loopsPlayed++;

            if (!seekInternal(m_loopStartSample)) {
                printf("Failed to seek to loop start\n");
                break;
            }

            if (m_bufferPosition >= m_bufferSize) {
                fillBuffer();
                if (m_bufferSize == 0) {
                    break;
                }
            }
        }

        if (m_bufferPosition >= m_bufferSize) {
            if (m_currentSample >= m_totalSamples) {
                break;
            }

            fillBuffer();
            if (m_bufferSize == 0) {
                break;
            }
        }

        qint64 bytesToCopy = qMin(m_bufferSize - m_bufferPosition, maxSize - totalBytesRead);

        qint64 samplesUntilLoopEnd = m_loopEndSample - m_currentSample;
        qint64 bytesUntilLoopEnd = samplesUntilLoopEnd * bytesPerSample;

        if (bytesUntilLoopEnd > 0 && bytesToCopy > bytesUntilLoopEnd) {
            bytesToCopy = bytesUntilLoopEnd;
        }

        if (bytesToCopy <= 0)
            break;

        memcpy(data + totalBytesRead, m_buffer.constData() + m_bufferPosition, bytesToCopy);

        m_bufferPosition += bytesToCopy;
        totalBytesRead += bytesToCopy;
        m_currentSample += bytesToCopy / bytesPerSample;
    }

    applyVolume(data, totalBytesRead);

    return totalBytesRead;
}

} // namespace melonMix

#endif // AUDIO_FLAC_ENABLED
