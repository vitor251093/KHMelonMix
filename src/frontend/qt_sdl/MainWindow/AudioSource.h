#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QIODevice>
#include <QFile>
#include <QAudioFormat>

namespace melonMix {

class AudioSource : public QIODevice
{
    Q_OBJECT
public:
    AudioSource(QObject *parent);

    // QIODevice methods
    qint64 size() const override { return std::numeric_limits<qint64>::max(); }
    bool seek(qint64 pos) override { return true; }
    bool isSequential() const override { return false; }
    bool atEnd() const override { return false; }
    qint64 writeData(const char*, qint64) override { return 0; }

    // Pure virtual methods to override in child classes
    virtual bool load(const QString &fileName) = 0;
    virtual bool isFileLoaded() const = 0;
    virtual qint64 getCurrentPlayingPos(qint64 audioSinkPlayTime) const = 0;

    // AudioSource methods
    virtual void onStarted(qint64 resumePosition, qreal volume, int fadeInMs);
    virtual void onStopped();

    quint32 getSampleRate() const { return m_sampleRate; }
    quint32 getNumChannels() const { return m_numChannels; }
    quint32 getBitsPerSample() const { return m_bitsPerSample; }

    void startFadeIn(qreal volume, int durationMs);
    void startFadeOut(int durationMs);
    void setVolume(double newVolume, int durationMs);

    static QAudioFormat::SampleFormat bitsPerSampleToSampleFormat(quint16 bitsPerSample);

protected:
    qint64 byte_pos_to_sample_pos(qint64 byte_pos) const;
    qint64 sample_pos_to_byte_pos(qint64 sample_pos) const;
    qint64 pos_ms_to_byte_pos(qint64 pos_ms) const;
    qint64 byte_pos_to_pos_ms(qint64 byte_pos) const;

    void applyVolume(char* buffer, int64_t numSamples);

    QAudioFormat m_format;

    qint64 m_dataStart = 0;
    qint64 m_dataSize = 0;
    
    quint32 m_sampleRate = 0;
    quint16 m_numChannels = 0;
    quint16 m_bitsPerSample = 0;
    quint16 m_bytesPerFrame = 0;

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
