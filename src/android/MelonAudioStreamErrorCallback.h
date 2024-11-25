#ifndef MELONDS_ANDROID_MELONAUDIOSTREAMERRORCALLBACK_H
#define MELONDS_ANDROID_MELONAUDIOSTREAMERRORCALLBACK_H

#include <oboe/Oboe.h>

class MelonAudioStreamErrorCallback : public oboe::AudioStreamErrorCallback {
private:
    void (*_callback)(void);

public:
    MelonAudioStreamErrorCallback(void (* callback)(void));
    void onErrorAfterClose(oboe::AudioStream* stream, oboe::Result result) override;
};

#endif //MELONDS_ANDROID_MELONAUDIOSTREAMERRORCALLBACK_H
