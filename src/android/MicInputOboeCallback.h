#ifndef MELONDS_ANDROID_MICINPUTOBOECALLBACK_H
#define MELONDS_ANDROID_MICINPUTOBOECALLBACK_H

#include <oboe/Oboe.h>
#include "types.h"

class MicInputOboeCallback : public oboe::AudioStreamCallback {
private:
    int bufferSize;
    int bufferOffset = 0;

public:
    s16* buffer;

    MicInputOboeCallback(int bufferSize);
    ~MicInputOboeCallback();
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) override;
};

#endif //MELONDS_ANDROID_MICINPUTOBOECALLBACK_H
