#include "MelonAudioStreamErrorCallback.h"

MelonAudioStreamErrorCallback::MelonAudioStreamErrorCallback(void (* callback)()) : _callback(callback) {
}

void MelonAudioStreamErrorCallback::onErrorAfterClose(oboe::AudioStream* stream, oboe::Result result) {
    if (result == oboe::Result::ErrorDisconnected && _callback != nullptr) {
        _callback();
    }
}
