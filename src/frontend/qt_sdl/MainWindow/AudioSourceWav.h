#ifndef AUDIOSOURCEWAV_H
#define AUDIOSOURCEWAV_H

#include "AudioSource.h"

#include <QAudioSink>
#include <QIODevice>

namespace melonMix {

class AudioSourceWav : public AudioSource
{
    Q_OBJECT
public:
    AudioSourceWav(QObject *parent);

    bool load(const QString &fileName) override;
    bool isFileLoaded() const override;
    qint64 getCurrentPlayingPos(qint64 audioSinkPlayTime) const override;

    void onStarted(qint64 resumePosition, qreal volume, int fadeInMs) override;
    void onStopped() override;

    qint64 readData(char *data, qint64 maxSize) override;

private:
    bool readWavHeader();

    QFile m_file;

    qint64 m_loopStartByte = 0;
    qint64 m_loopEndByte = 0;

    qint64 m_currentPos = 0;
    qint64 m_resumedPos = 0;
    qint64 m_loopsPlayed = 0;
};

} // namespace melonMix

#endif // AUDIOSOURCEWAV_H
