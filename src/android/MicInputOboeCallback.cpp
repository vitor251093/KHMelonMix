#include "MicInputOboeCallback.h"
#include "types.h"

MicInputOboeCallback::MicInputOboeCallback(int bufferSize) : bufferSize(bufferSize) {
    this->buffer = new s16[bufferSize];
}

oboe::DataCallbackResult
MicInputOboeCallback::onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {
    s16* input = (s16*) audioData;

    for (int i = 0; i < numFrames; ++i) {
        int intVal = (int) input[i];
        int newVal = intVal * 10;
        if (newVal >= 32768)
            newVal = 32767;
        else if (newVal <= -32767)
            newVal = -32766;

        input[i] = (s16) newVal;
    }

    if (this->bufferOffset + numFrames > this->bufferSize) {
        int firstCopyAmount = this->bufferSize - this->bufferOffset;
        int secondCopyAmount = numFrames - firstCopyAmount;

        memcpy(&this->buffer[this->bufferOffset], input, firstCopyAmount * sizeof(s16));
        memcpy(this->buffer, &input[firstCopyAmount], secondCopyAmount * sizeof(s16));
        this->bufferOffset = secondCopyAmount;
    } else {
        memcpy(&this->buffer[this->bufferOffset], input, numFrames * sizeof(s16));
        this->bufferOffset += numFrames;
    }
    return oboe::DataCallbackResult::Continue;
}

MicInputOboeCallback::~MicInputOboeCallback() {
    delete[] this->buffer;
}
