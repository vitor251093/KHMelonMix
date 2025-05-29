#include "AudioUtils.h"

#include "../NDS.h"
#include <cstdio>

namespace Plugins {
namespace AudioUtils {

SSEQMuter::SSEQMuter(melonDS::NDS* in_nds, u16 bgmId, u32 songAddress, u32 songSize)
    : nds(in_nds)
    , m_bgmId(bgmId)
    , m_songAddress(songAddress)
    , m_songSize(songSize)
{
    parseSequenceHeader();
}

void SSEQMuter::parseSequenceHeader() {
    m_bSequenceIsValid = false;

    if (m_bgmId == 0xFFFF) {
        return;
    }

    // KH games store a table of entries in RAM, containing addresses of where the SSEQ is loaded
    // The table's length is equal to the total number of tracks.
    // Only one SSEQ is loaded at all times! And its address can be found using the BgmId (from SONG_ID_ADDRESS)
    // Important: the EMidiState needs to be checked! When the status is "LoadSequence", the SSEQ is not loaded yet.
    // When "PrePlay" or "Playing", the SSEQ is in RAM and its address is available.
    // First u32 in a table row corresponds to the size of the loaded SSEQ, and the second u32 is the address in RAM.
    // Caution: in Days, there is no track 27! Meaning that every song starting from 28 needs to be "-1" to get the correct address
    // (otherwise you'll just get a nullptr entry in the table). See call to getSongIdInSongTable().
    if (m_songAddress == 0) {
        printf("Error: SSEQ %d is not loaded!!!\n", m_bgmId);
        return;
    }

    if (m_songSize == 0) {
        printf("Error: Invalid SSEQ size for %d\n", m_bgmId);
        return;
    }

    u32 sseqTag = nds->ARM9Read32(m_songAddress);
    u32 sseqMagic = nds->ARM9Read32(m_songAddress + 4);
    u32 sseqFileSize = nds->ARM9Read32(m_songAddress + 8);

    if (sseqTag != 0x51455353) { // SSEQ
        printf("Error: Invalid SSEQ: incorrect header tag\n");
        return;
    }

    if (sseqMagic != 0x0100feff) {
        printf("Error: Invalid SSEQ: incorrect magic number\n");
        return;
    }

    if (sseqFileSize != m_songSize) {
        printf("Error: Invalid SSEQ: incorrect file size\n");
        return;
    }

    u32 dataHdrTag = nds->ARM9Read32(m_songAddress + 0x10);
    if (dataHdrTag != 0x41544144) { // "DATA"
        printf("Error: Invalid SSEQ: incorrect DATA header tag\n");
        return;
    }

    m_bSequenceIsValid = true;
}

void SSEQMuter::muteSongSequence() {
    if (!m_bSequenceIsValid) {
        return;
    }

    u32 firstValue = nds->ARM9Read32(m_songAddress + 32);
    if (firstValue != 0) {
        constexpr const u32 headerSize = 32;
        u32 startErase = m_songAddress + headerSize;
        u32 endErase = m_songAddress + m_songSize;
        for (u32 addr = startErase; addr < endErase; addr+=4) {
            nds->ARM7Write32(addr, 0x00);
        }

        //printf("Music SSEQ: Muted bgm %d (erased %d bytes)\n", m_bgmId, endErase - startErase);
    }
}

void SSEQMuter::muteSongSequenceV2() {
    if (!m_bSequenceIsValid) {
        return;
    }

    std::vector<uint32_t> trackOffsets;

    u32 offset = m_songAddress + 0x1C;
    uint8_t b = nds->ARM9Read8(offset);

    // SSEQ parsing logic extracted from VGMTrans
    // https://github.com/vgmtrans/vgmtrans/blob/master/src/main/formats/NDS/NDSSeq.cpp
    if (b == 0xFE) {
        offset += 3;
        b = nds->ARM9Read8(offset);
        while (b == 0x93) {
            uint32_t trkOffset = nds->ARM9Read8(offset + 2) + (nds->ARM9Read8(offset + 3) << 8) +
                (nds->ARM9Read8(offset + 4) << 16) + m_songAddress + 0x1C;
            trackOffsets.push_back(trkOffset);
            offset += 5;
            b = nds->ARM9Read8(offset);
        }
    }

    // First track is not in the table, its offset is simply the address when the table ends
    trackOffsets.insert(trackOffsets.begin(), offset);

    if (!trackOffsets.empty()) {
        for (int i = 0; i < trackOffsets.size(); i++) {
            u32 startOffset = trackOffsets[i];
            u32 endOffset = (i + 1 < trackOffsets.size()) ? trackOffsets[i + 1] : m_songAddress + m_songSize;

            uint32_t curOffset = startOffset;
            while (curOffset < endOffset) {
                readTrackEvent(curOffset, endOffset);
            }
        }
    }
}

uint8_t SSEQMuter::readByte(const uint32_t offset) const {
    return nds->ARM9Read8(offset);
}

uint16_t SSEQMuter::readShort(const uint32_t offset) const {
    return nds->ARM9Read16(offset);
}

uint32_t SSEQMuter::readVarLen(uint32_t &offset, const uint32_t endOffset) const {
    uint32_t value = 0;

    while (offset < endOffset) {
        uint8_t c = readByte(offset++);
        value = (value << 7) + (c & 0x7F);
        // Check if continue bit is set
        if((c & 0x80) == 0) {
            break;
        }
    }
    return value;
}

void SSEQMuter::readTrackEvent(uint32_t& curOffset, uint32_t endOffset) {
    uint8_t status_byte = readByte(curOffset++);

    if (status_byte < 0x80) // NoteOn Event
    {
      nds->ARM9Write8(curOffset, 0x00); // Erasing velocity
      curOffset++;
      u32 dur = readVarLen(curOffset, endOffset);
    }
    else
    switch (status_byte) {
        case 0x80: { // Wait
            auto dur = readVarLen(curOffset, endOffset);
            break;
        }
        case 0x81: { // Program Change
            u8 newProg = readVarLen(curOffset, endOffset);
            break;
        }
        case 0x93: {
            curOffset += 4;
            break;
        }
        case 0xA0: { // Random
            curOffset += 5;
            break;
        }
        case 0x94:
        case 0x95:
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD: {
            curOffset += 3;
            break;
        }
        case 0xC1: { // Volume change
            nds->ARM9Write8(curOffset, 0x00);
            curOffset++;
            break;
        }
        case 0xC2: { // Main volume
            nds->ARM9Write8(curOffset, 0x00);
            curOffset++;
            break;
        }
        case 0xA2: // If
        case 0xC0: // Pan
        case 0xC3: // Transpose
        case 0xC4: // Pitchbend
        case 0xC5: // Pitchbend Range
        case 0xC6: // Priority
        case 0xC7: // Note wait
        case 0xC8: // Tie
        case 0xC9: // Portamento
        case 0xCA: // Mod Depth
        case 0xCB: // Mod Speed
        case 0xCC: // Mod Type
        case 0xCD: // Mod Range
        case 0xCE: // Portamento Switch
        case 0xCF: // Portamento Time
        case 0xD0: // Attack
        case 0xD1: // Decay
        case 0xD2: // Sustain
        case 0xD3: // Release
        case 0xD4: // Loop Start
        case 0xD5: // Volume 2
        case 0xD6: { // Print Variable
            // Replace any of those events with a "Set volume to 0"
            nds->ARM9Write8(curOffset - 1, 0xC1);
            nds->ARM9Write8(curOffset, 0x00);
            curOffset++;
            break;
        }
        case 0xA1: // Variable
        case 0xE0: // Modulation Delay
        case 0xE1: // Tempo
        case 0xE2: // Sweep Pitch
        case 0xE3:
        case 0xFE: { // Allocate tracks
            curOffset += 2;
            break;
        }
        case 0xFC: { // Loop End
            break;
        }
        case 0xFD: // Return
        case 0xFF: // Enf of Tracks
        default: {
            return;
        }
    }
}


} // namespace AudioUtils
} // namespace Plugins
