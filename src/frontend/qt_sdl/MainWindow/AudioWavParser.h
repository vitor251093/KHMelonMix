#ifndef AUDIOWAVPARSER_H
#define AUDIOWAVPARSER_H

#include <string>
#include <vector>
#include "types.h"

namespace melonMix {

using namespace melonDS;

struct WavSampleLoop
{
    uint32_t id;
    uint32_t type;
    uint32_t start;
    uint32_t end;
    uint32_t fraction;
    uint32_t playCount;
};

struct WavSmplChunk
{
    uint32_t manufacturerId;
    uint32_t productId;
    uint32_t samplePeriod;
    uint32_t midiUnityNote;
    uint32_t midiPitchFraction;
    uint32_t smpteFormat;
    uint32_t smpteOffset;
    uint32_t numSampleLoops;
    uint32_t samplerData;
    std::vector<WavSampleLoop> loops;
};

class AudioWavParser
{
public:
    AudioWavParser() {}
    
    bool readCuePoints(const std::string& filePath);

    enum EMarker : u8 { Begin, End };
    s64 getLoopMarkerSample(EMarker marker) const;

private:
    bool parseFmtChunk(std::ifstream& file, uint32_t size);
    bool parseSmplChunk(std::ifstream& file, uint32_t size);

    bool m_hasSmplLoops;
    WavSmplChunk m_smplChunk;
};

} // namespace melonMix

#endif // AUDIOWAVPARSER_H