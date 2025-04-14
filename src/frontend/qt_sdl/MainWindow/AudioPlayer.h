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
    AudioPlayer(QObject* parent);

    bool loadFile(const QString &fileName);

    void play();
    void stop(int fadeOutMs = 0);

    void pause();
    void resume();

    void setVolume(qreal value);

    bool isPlaying() const { return m_playing; }

private slots:
    void onFadeOutCompleted();

private:
    QScopedPointer<AudioSourceWav> m_audioSource;
    QScopedPointer<QAudioSink> m_audioOutput;

    bool m_playing = false;
};

} // namespace melonMix

#endif // AUDIOPLAYER_H