#pragma once

#include <string>
#include <vector>

#include "strings.h"

namespace ndsloc {

namespace strings {
struct CsvNdsFile;
}

enum Language : uint8_t;

namespace p2 {

static const uint32_t kSectorTableAt = 0x10;
static const uint32_t kSectorSize = 0x200;
static const uint32_t kNameLength = 8;

static const uint8_t kTypePlain = 0x00;
static const uint8_t kTypeNamed = 0x80;

static const uint8_t kFlagCompressed = 0x80;

static const uint8_t kLZ10 = 0x10;
static const uint8_t kLZ11 = 0x11;

bool isP2File(const uint8_t* buffer);

} // namespace p2

struct P2SubFile
{
    uint16_t chunkId = 0;
    std::string fileName;
    uint32_t fileOffset = 0;
    uint16_t fileSize = 0;
    uint16_t maxSize = 0;
    uint8_t someFlag = 0;

    uint8_t* inputPtr = nullptr;

    std::string getFilename() const
    {
        if (!fileName.empty())
            return fileName;
        else
            return std::to_string(chunkId) + ".Z";
    }

    bool isCompressed() const
    {
        return (someFlag & p2::kFlagCompressed) != 0;
    }
};

class P2File
{
public:
    P2File(const std::string& filePath);
    P2File(uint8_t* inputPtr, uint32_t inputSize);

    const std::vector<P2SubFile>& readAndGetFileTable();

    void setLanguage(Language language) { m_language = language; }

    bool extractStrings(std::vector<String>& out);

    void saveToDisk(const std::string& outPath);

    void updateEntrySizeInTable(uint16_t subfileId, uint32_t newSize);
private:
    bool sizeTableLooksValid(uint32_t sizesAt) const;
    bool readFileTable();

    bool extractSubFile(const P2SubFile& subfile, uint32_t depth, std::vector<String>& out);
    bool extractPayload(const P2SubFile& subfile, uint8_t* payload, uint32_t payloadSize, uint32_t payloadOffset, uint32_t depth, std::vector<String>& out);

    std::vector<uint8_t> m_inputBuffer;
    uint8_t* m_inputPtr = nullptr;
    uint32_t m_inputSize = 0;
    Language m_language;
    static constexpr uint32_t m_maxDepth = 4;

    std::vector<P2SubFile> m_subfiles;
};


} // namespace ndsloc
