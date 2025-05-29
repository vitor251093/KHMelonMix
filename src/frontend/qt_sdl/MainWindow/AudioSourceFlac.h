#ifndef AUDIOSOURCEFLAC_H
#define AUDIOSOURCEFLAC_H

#ifdef AUDIO_FLAC_ENABLED

#include "AudioSource.h"

#include <QAudioSink>
#include <QString>
#include <FLAC/stream_decoder.h>

namespace melonMix {

class AudioSourceFlac : public AudioSource
{
    Q_OBJECT

public:
    AudioSourceFlac(QObject *parent);
    ~AudioSourceFlac();

    bool load(const QString &fileName) override;
    bool isFileLoaded() const override;
    qint64 getCurrentPlayingPos(qint64 audioSinkPlayTime) const override;

    void onStarted(qint64 resumePosition, qreal volume, int fadeInMs) override;
    void onStopped() override;

    bool open(QIODevice::OpenMode mode) override;
    void close() override;

protected:
    bool seekInternal(qint64 samplePos);
    qint64 readData(char *data, qint64 maxSize) override;

private:
    // FLAC callbacks
    static FLAC__StreamDecoderWriteStatus writeCallback(
        const FLAC__StreamDecoder *decoder, 
        const FLAC__Frame *frame, 
        const FLAC__int32 * const buffer[], 
        void *clientData);
        
    static void metadataCallback(
        const FLAC__StreamDecoder *decoder, 
        const FLAC__StreamMetadata *metadata, 
        void *clientData);
        
    static void errorCallback(
        const FLAC__StreamDecoder *decoder, 
        FLAC__StreamDecoderErrorStatus status, 
        void *clientData);

    bool initializeDecoder();
    void cleanupDecoder();
    bool decodeUntilFrame(qint64 targetSample);
    void fillBuffer();
    FLAC__StreamDecoder* createNewDecoder();

    FLAC__StreamDecoder* m_decoder = nullptr;
    QString m_fileName;
    
    bool m_fileLoaded = false;
    qint64 m_totalSamples = 0;

    qint64 m_loopStartSample = 0;
    qint64 m_loopEndSample = 0;

    qint64 m_currentSample = 0;
    qint64 m_resumedPos = 0;
    qint64 m_loopsPlayed = 0;

    QByteArray m_buffer;
    qint64 m_bufferPosition = 0;
    qint64 m_bufferSize = 0;

    qint64 m_lastDecodedSample = 0;
    qint64 m_totalReadSamples = 0;
};

} // namespace melonMix

#endif // AUDIO_FLAC_ENABLED

#endif // AUDIOSOURCEFLAC_H
