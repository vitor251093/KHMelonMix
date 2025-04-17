#include "AudioWavParser.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

namespace melonMix {

struct WavHeader
{
    char riff[4];
    u32 fileSize;
    char wave[4];
};

struct WavFmtChunk
{
    u16 audioFormat;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
};

struct ChunkHeader
{
    char id[4];
    u32 size;
};

bool AudioWavParser::readCuePoints(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        printf("Error: Could not open file: %s\n", filePath.c_str());
        return false;
    }

    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::strncmp(header.riff, "RIFF", 4) != 0 || std::strncmp(header.wave, "WAVE", 4) != 0) {
        printf("Error: Not a valid WAV file: %s\n", filePath.c_str());
        return false;
    }
    
    bool foundFmt = false;
    bool foundSmpl = false;

    while (!file.eof()) {
        ChunkHeader chunkHeader;
        file.read(reinterpret_cast<char*>(&chunkHeader), sizeof(chunkHeader));

        if (file.eof())
            break;

        if (std::strncmp(chunkHeader.id, "fmt ", 4) == 0) {
            foundFmt = parseFmtChunk(file, chunkHeader.size);
        } else if (std::strncmp(chunkHeader.id, "smpl", 4) == 0) {
            foundSmpl = parseSmplChunk(file, chunkHeader.size);
        } else {
            file.seekg(chunkHeader.size, std::ios::cur);
        }
        
        if (chunkHeader.size % 2 != 0) {
            file.seekg(1, std::ios::cur);
        }
    }

    if (!foundFmt) {
        std::cerr << "Error: Format chunk not found in WAV file" << std::endl;
        return false;
    }
    
    if (!foundSmpl) {
        std::cerr << "Warning: No cue points found in WAV file" << std::endl;
        return false;
    }

    return foundSmpl;
}

s64 AudioWavParser::getLoopMarkerSample(EMarker marker) const
{
    if (m_hasSmplLoops && !m_smplChunk.loops.empty()) {
        switch(marker)
        {
        case EMarker::Begin:
            return m_smplChunk.loops[0].start;
        case EMarker::End:
            return m_smplChunk.loops[0].end;
        }
    }

    return -1;
}

bool AudioWavParser::parseFmtChunk(std::ifstream& file, uint32_t size)
{
    WavFmtChunk fmtChunk;
    file.read(reinterpret_cast<char*>(&fmtChunk), sizeof(fmtChunk));

    if (size > sizeof(fmtChunk)) {
        file.seekg(size - sizeof(fmtChunk), std::ios::cur);
    }

    return true;
}

bool AudioWavParser::parseSmplChunk(std::ifstream& file, uint32_t size)
{
    file.read(reinterpret_cast<char*>(&m_smplChunk), sizeof(uint32_t) * 9);

    if (m_smplChunk.numSampleLoops > 0) {
        m_smplChunk.loops.resize(m_smplChunk.numSampleLoops);

        for (uint32_t i = 0; i < m_smplChunk.numSampleLoops; ++i) {
            file.read(reinterpret_cast<char*>(&m_smplChunk.loops[i]), sizeof(WavSampleLoop));
        }

        m_hasSmplLoops = true;
    }

    uint32_t bytesRead = sizeof(uint32_t) * 9 + m_smplChunk.numSampleLoops * sizeof(WavSampleLoop);
    if (size > bytesRead) {
        file.seekg(size - bytesRead, std::ios::cur);
    }
    
    return true;
}

} // namespace melonMix