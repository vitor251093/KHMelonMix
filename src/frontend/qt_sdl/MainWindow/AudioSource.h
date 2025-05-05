#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioSink>
#include <QIODevice>
#include <QFile>
#include <QBuffer>

namespace melonMix {

class AudioSourceWav : public QIODevice
{
    Q_OBJECT
public:
    AudioSourceWav(QObject *parent = nullptr);

    void onStarted(qint64 resumePosition, qreal volume, int fadeInMs);
    void onStopped();

    bool load(const QString &fileName);


    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char*, qint64) override { return 0; }
    qint64 size() const override { return m_file.size(); }
    bool isSequential() const override { return false; }
    bool atEnd() const override { return false; }
    qint64 bytesAvailable() const override;


    quint32 getSampleRate() const { return m_sampleRate; }
    quint32 getNumChannels() const { return m_numChannels; }
    quint32 getBitsPerSample() const { return m_bitsPerSample; }

    qint64 getCurrentPlayingPos() const { return m_currentPos; }
    void startFadeIn(qreal volume, int durationMs);
    void startFadeOut(int durationMs);
    void setVolume(double newVolume, int durationMs);

    static QAudioFormat::SampleFormat bitsPerSampleToSampleFormat(quint16 bitsPerSample);

private:
    void applyVolume(char* buffer, int64_t numSamples);

    bool readWavHeader();

    QFile m_file;
    QAudioFormat m_format;
    
    qint64 m_dataStart = 0;
    qint64 m_dataSize = 0;
    
    quint32 m_sampleRate = 0;
    quint16 m_numChannels = 0;
    quint16 m_bitsPerSample = 0;
    quint16 m_bytesPerFrame = 0;

    qint64 m_loopStartByte = 0;
    qint64 m_loopEndByte = 0;

    qint64 m_currentPos = 0;

    enum class EStatus : quint8 { Playing, Stopping, Stopped };
    EStatus m_status = EStatus::Stopped;
    quint64 m_stoppingDelay = 0;

    double m_currentVolume = 1.0;
    double m_targetVolume = 1.0;
    double m_volumeIncrement = 0.0;
    int64_t m_samplesRemaining = 0;
    int64_t m_totalTransitionSamples = 0;
};

} // namespace melonMix

#endif // AUDIOSOURCE_H