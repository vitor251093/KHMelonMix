#ifndef MELONDS_OBOECALLBACK_H
#define MELONDS_OBOECALLBACK_H

#include <oboe/Oboe.h>

class OboeCallback : public oboe::AudioStreamCallback {
private:
    int _volume;

public:
    OboeCallback(int volume);
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) override;
};


#endif //MELONDS_OBOECALLBACK_H
