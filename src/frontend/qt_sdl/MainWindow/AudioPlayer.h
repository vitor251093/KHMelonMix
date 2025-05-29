#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QAudioSink>

namespace melonMix {

class AudioSource;

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    AudioPlayer(QObject* parent, quint16 bgmId);

    bool loadFile(const QString &fileName);

    void play(qint64 resumePosition, qreal volume, int fadeInMs);
    void stop(int fadeOutMs);

    void pause();
    void resume();

    void setVolume(qreal value, int durationInMs);

    quint16 getBgmId() const { return m_bgmId; }
    bool isPlaying() const { return m_playing; }
    qint64 getCurrentPlayingPos() const;

    void restartAudioSink(const QAudioDevice& audioOutput);

private slots:
    void onFadeOutCompleted();

private:
    void createAudioSink(const QAudioDevice& audioOutput);

    QScopedPointer<AudioSource> m_audioSource;
    QScopedPointer<QAudioSink> m_audioOutput;

    quint16 m_bgmId = 0;
    bool m_playing = false;
};

} // namespace melonMix

#endif // AUDIOPLAYER_H