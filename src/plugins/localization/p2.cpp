#include "p2.h"

#include <filesystem>
#include <assert.h>
#include <cstring>
#include <fstream>

#include "utils.h"
#include "lang.h"
#include "lzss.h"
#include "strings.h"
#include "cakp.h"
#include "stringtable.h"
#include "rompatcher.h"

namespace ndsloc {

namespace p2 {

bool isP2File(const uint8_t* buffer)
{
    return (std::memcmp(buffer, "P2", 2) == 0);
}

} // namespace p2

P2File::P2File(const std::string& filePath)
    : m_language(LANG_EN)
{
    m_inputBuffer = utils::readBinaryFile(filePath);
    m_inputPtr = m_inputBuffer.data();
    m_inputSize = m_inputBuffer.size();
}

P2File::P2File(uint8_t* inputPtr, uint32_t inputSize)
    : m_inputPtr(inputPtr)
    , m_inputSize(inputSize)
    , m_language(LANG_EN)
{
}

bool P2File::sizeTableLooksValid(uint32_t sizesAt) const
{
    const uint32_t fileCount = static_cast<uint32_t>(m_subfiles.size());

    if (sizesAt + fileCount * 4 > m_inputSize)
        return false;

    for (uint32_t i = 0; i < fileCount; ++i)
    {
        const uint32_t offset = m_subfiles[i].fileOffset;
        const uint32_t size = utils::readUInt24(m_inputPtr, sizesAt + i * 4);
        const uint8_t flag = m_inputPtr[sizesAt + i * 4 + 3];

        if (offset > m_inputSize || size > m_inputSize - offset)
            return false;

        if (size > m_subfiles[i].maxSize)
            return false;

        if ((flag & p2::kFlagCompressed) != 0 &&
            m_inputPtr[offset] != p2::kLZ10 && m_inputPtr[offset] != p2::kLZ11)
            return false;
    }

    return true;
}

const std::vector<P2SubFile>& P2File::readAndGetFileTable()
{
    readFileTable();
    return m_subfiles;
}

bool P2File::readFileTable()
{
    m_subfiles.clear();

    uint8_t* dataPtr = m_inputPtr;
    const uint32_t inputSize = m_inputSize;

    if (!p2::isP2File(dataPtr) || inputSize < p2::kSectorTableAt)
        return false; // Not a P2 file

    const uint8_t fileCount = dataPtr[2];
    const uint8_t fileType = dataPtr[3];
    const uint32_t firstFileOffset = utils::readUInt24(dataPtr, 0x0C);

    if (fileCount == 0)
        return false;

    if (fileType != p2::kTypePlain && fileType != p2::kTypeNamed)
        return false; // Unknown P2 type

    uint32_t currentPos = p2::kSectorTableAt;
    if (currentPos + fileCount * 2 > inputSize)
        return false;

    m_subfiles.resize(fileCount);

    for (int i = 0; i < fileCount; i++)
    {
        uint16_t val = utils::readUInt16(dataPtr, currentPos);
        m_subfiles[i].fileOffset = firstFileOffset + val * p2::kSectorSize;
        m_subfiles[i].chunkId = static_cast<uint16_t>(i);
        currentPos += 2;
    }

    for (int i = 0; i < fileCount; i++)
    {
        auto& fileDesc = m_subfiles[i];

        if (fileDesc.fileOffset > inputSize)
            return false;

        if (i + 1 < fileCount)
        {
            auto& nextFileDesc = m_subfiles[i + 1];
            if (nextFileDesc.fileOffset > fileDesc.fileOffset)
                fileDesc.maxSize = nextFileDesc.fileOffset - fileDesc.fileOffset;
            else
                fileDesc.maxSize = 0;
        }
        else
        {
            fileDesc.maxSize = inputSize - fileDesc.fileOffset;
        }
    }

    currentPos = (currentPos + 3u) & ~3u;

    const uint32_t sizesAt = sizeTableLooksValid(currentPos) ? currentPos : 0;

    if (sizesAt != 0)
    {
        for (int i = 0; i < fileCount; i++)
        {
            auto& fileDesc = m_subfiles[i];

            fileDesc.fileSize = utils::readUInt24(dataPtr, sizesAt + i * 4);
            fileDesc.someFlag = dataPtr[sizesAt + i * 4 + 3];
        }

        currentPos = sizesAt + fileCount * 4;
    }
    else
    {
        // No usable size table: fall back to the sector span and sniff the
        // payload for a codec marker instead of assuming compression.
        for (int i = 0; i < fileCount; i++)
        {
            auto& fileDesc = m_subfiles[i];

            fileDesc.fileSize = fileDesc.maxSize;

            const uint8_t first = dataPtr[fileDesc.fileOffset];
            fileDesc.someFlag = (first == p2::kLZ10 || first == p2::kLZ11)
                ? p2::kFlagCompressed
                : 0;
        }

        currentPos += fileCount * 4;
    }

    // --- name table: 8 bytes per entry, NUL-padded ---
    if (fileType == p2::kTypeNamed && currentPos + fileCount * p2::kNameLength <= inputSize)
    {
        for (int i = 0; i < fileCount; i++)
        {
            auto& fileDesc = m_subfiles[i];

            const char* name = reinterpret_cast<const char*>(dataPtr + currentPos);
            fileDesc.fileName = std::string(name, strnlen(name, p2::kNameLength));
            currentPos += p2::kNameLength;
        }
    }

    for (int i = 0; i < fileCount; i++)
    {
        auto& fileDesc = m_subfiles[i];
        fileDesc.inputPtr = dataPtr + fileDesc.fileOffset;
    }

    return true;
}

void P2File::saveToDisk(const std::string& outPath)
{
    utils::saveBinaryFile(m_inputBuffer, outPath);
}

void appendStrings(const std::vector<String>& strings, std::vector<String>& out)
{
    for (auto& str : strings)
    {
        if (str.text == "\x01")
            continue;

        out.push_back(std::move(str));
    }
}

bool P2File::extractPayload(const P2SubFile& subfile, uint8_t* payload, uint32_t payloadSize, uint32_t payloadOffset, uint32_t depth, std::vector<String>& out)
{
    assert(subfile.fileOffset == payloadOffset);

    if (payload == nullptr || payloadSize < 8)
        return true; // stub entries

    std::vector<String> strings;

    if (cakp::isCAKP(payload))
    {
        CAKPFile cakp(payload, payloadSize, subfile.getFilename());
        cakp.setLanguage(m_language);
        const bool ok = cakp.extractStrings(strings);

        appendStrings(strings, out);
        return ok;
    }

    if (p2::isP2File(payload))
    {
        if (depth >= m_maxDepth)
            return true;

        P2File nested(payload, payloadSize);
        nested.setLanguage(m_language);
        //nested.setMaxDepth(m_maxDepth);

        std::vector<String> nestedOut;
        const bool ok = nested.extractStrings(nestedOut);
        appendStrings(nestedOut, out);
        return ok;
    }

    if (StringTableFile::looksValid(payload, payloadSize))
    {
        std::vector<U16String> wide;
        StringTableFile table(payload, payloadSize);
        const bool ok = table.extractStrings(wide);
        strings::appendWideStrings(subfile.getFilename(), wide, out);
        return ok;
    }

    CAKPFile section(payload, payloadSize, subfile.getFilename());
    section.setLanguage(m_language);
    section.extractSectionStrings(strings);
    appendStrings(strings, out);
    return true;
}

uint32_t getExpectedDecompressedSize(const uint8_t* dataPtr, uint32_t dataSize)
{
    if (dataSize < 4)
        return 0;

    const uint32_t packed = utils::readUInt24(dataPtr, 1);
    if (packed != 0)
        return packed;

    return dataSize >= 8 ? utils::readUInt32(dataPtr, 4) : 0;
}

bool P2File::extractSubFile(const P2SubFile& subfile, uint32_t depth, std::vector<String>& out)
{
    if (subfile.fileSize == 0)
        return true;

    if (subfile.isCompressed())
    {
        ndsloc::LZSSFile lzss(subfile.inputPtr, subfile.fileSize);
        lzss.decompress();

        auto decompressed = lzss.getConvertedData();
        auto* data = decompressed.data();
        auto dataSize = static_cast<uint32_t>(decompressed.size());

        const uint32_t expected = getExpectedDecompressedSize(subfile.inputPtr, subfile.maxSize);
        assert(expected == dataSize);

        return extractPayload(subfile, data, dataSize, subfile.fileOffset, depth, out);
    }
    else
    {
        return extractPayload(subfile, subfile.inputPtr, subfile.fileSize, subfile.fileOffset, depth, out);
    }
}

void P2File::updateEntrySizeInTable(uint16_t subfileId, uint32_t newSize)
{
    // TODO: update this function

    const uint8_t fileCount = m_subfiles.size();
    uint8_t fileType = m_inputPtr[3];

    if (fileType == 0x80)
    {
        P2SubFile& fileDesc = m_subfiles[subfileId];
        const uint32_t currentPos = 0x10 + (2 * fileCount) + (4 * subfileId);

        m_inputPtr[currentPos] = static_cast<uint8_t>(newSize & 0xFF);
        m_inputPtr[currentPos + 1] = static_cast<uint8_t>((newSize >> 8) & 0xFF);
        m_inputPtr[currentPos + 2] = static_cast<uint8_t>((newSize >> 16) & 0xFF);
    }
}

bool P2File::extractStrings(std::vector<String>& out)
{
    out.clear();

    if (!readFileTable())
        return false;

    bool ok = true;
    for (const P2SubFile& subfile : m_subfiles)
    {
        if (!extractSubFile(subfile, 0, out))
            ok = false;
    }

    return ok;
}

} // namespace ndsloc
