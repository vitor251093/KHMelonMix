#include "OboeCallback.h"
#include "../types.h"
#include "../SPU.h"
#include "FrontendUtil.h"

OboeCallback::OboeCallback(int volume) : _volume(volume) {
}

oboe::DataCallbackResult
OboeCallback::onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {
    int len = numFrames;

    // resample incoming audio to match the output sample rate

    int len_in = Frontend::AudioOut_GetNumSamples(len);
    s16 buf_in[1024 * 2];
    int num_in;

    // TODO: audio sync
    num_in = SPU::ReadOutput(buf_in, len_in);

    if (num_in < 1)
    {
        memset(audioData, 0, len * sizeof(s16) * 2);
        return oboe::DataCallbackResult::Continue;
    }

    int margin = 6;
    if (num_in < len_in - margin)
    {
        int last = num_in-1;

        for (int i = num_in; i < len_in - margin; i++)
            ((u32*) buf_in)[i] = ((u32*) buf_in)[last];

        num_in = len_in - margin;
    }

    Frontend::AudioOut_Resample(buf_in, num_in, (s16*) audioData, len, _volume);
    return oboe::DataCallbackResult::Continue;
}
