#pragma once

#include "..\types.h"

namespace melonDS {
    class NDS;
}

namespace Plugins {
namespace AudioUtils {

using namespace melonDS;

class SSEQMuter {
public:
    SSEQMuter(melonDS::NDS* in_nds, u16 bgmId, const u32 sseqTableAddress, const u32 sseqId);

    void muteSongSequence();
    void muteSongSequenceV2();

private:
    void parseSequenceHeader();
    void readTrackEvent(uint32_t& curOffset, uint32_t endOffset);
    uint8_t readByte(const uint32_t offset) const;
    uint16_t readShort(const uint32_t offset) const;
    uint32_t readVarLen(uint32_t &offset, const uint32_t endOffset) const;

private:
    melonDS::NDS* nds = nullptr;
    const u16 m_bgmId = 0;
    const u32 m_sseqTableAddress = 0;
    const u32 m_sseqId = 0;

    u32 m_songSize = 0;
    u32 m_songAddress = 0;

    bool m_bSequenceIsValid = false;
};

} // namespace AudioUtils
} // namespace Plugins
