#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

class QAudioSink;

namespace melonMix {

class AudioSourceWav;

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

private slots:
    void onFadeOutCompleted();

private:
    QScopedPointer<AudioSourceWav> m_audioSource;
    QScopedPointer<QAudioSink> m_audioOutput;

    quint16 m_bgmId = 0;
    bool m_playing = false;
};

} // namespace melonMix

#endif // AUDIOPLAYER_H