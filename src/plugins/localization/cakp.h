#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace ndsloc {

enum Language : uint8_t;
struct String;

namespace cakp {

#pragma pack(push, 1)
struct FileHeader
{
    char magic[4]; // "CAKP"
    uint32_t unknown04;
    uint32_t fileSlots[8];
};

struct Instruction
{
    uint8_t group;
    uint8_t opcode;
    uint8_t sizeInDwords; 
    uint8_t flags;
};

enum ArgType : uint8_t
{
    ARG_NULL = 0x00,
    ARG_INT = 0x01,
    ARG_STR = 0x02,
    ARG_VAR = 0x04,
    ARG_BYTE = 0x08,
    ARG_FIXED = 0x10,
    ARG_MSG = 0x40
};
#pragma pack(pop)

bool isCAKP(const uint8_t* buffer);

bool isTextOpcode(uint8_t group, uint8_t opcode);

} // namespace cakp

class CAKPFile
{
public:
    CAKPFile(const uint8_t* inputPtr, uint32_t inputSize, std::string sectionName);

    void setLanguage(Language language) { m_language = language; }

    // Extraction method 1: whole CAKP file
    bool extractStrings(std::vector<String>& out);
    // Extraction method 2: single bare section (without CAKP header)
    bool extractSectionStrings(std::vector<String>& out);

private:
    struct Directory
    {
        uint32_t count;
        uint32_t offsetsAt;
        uint32_t sizesAt;
    };

    bool inRange(uint32_t offset, uint32_t length) const;
    bool readDirectory(uint32_t offset, Directory& dir);
    bool directoryEntry(const Directory& dir, uint32_t index, uint32_t& offset, uint32_t& size);

    bool readSectionNames(const Directory& nameDir, std::vector<std::string>& out);

    bool collectSection(uint32_t sectionAt, uint32_t sectionSize, std::vector<String>& out);
    void collectMessageRecord(uint32_t recordAt, uint32_t poolAt, uint32_t sectionEnd, std::vector<String>& out);

    void addString(uint32_t offset, uint32_t limit, bool allowDuplicates, std::vector<String>& out);

    bool readLanguageCondition(uint32_t conditionAt, uint32_t sectionEnd, uint16_t& language) const;

    const uint8_t* m_buffer = nullptr;
    uint32_t m_bufferSize = 0;
    Language m_language;

    int m_sectionLanguage = -1;

    const std::string m_sectionName;

    std::vector<uint32_t> m_seen;

    static constexpr uint16_t kLanguageVarId = 0x2011;
    static constexpr uint32_t kConditionSize = 24;
    static constexpr uint32_t kConditionVarIdAt = 0x06;
    static constexpr uint32_t kConditionValueAt = 0x10;

    static constexpr uint32_t kUnused = 0xFFFFFFFFu;
    static constexpr uint32_t kMessageSlots = 7;
    static constexpr uint32_t kLanguageCount = 6;
};

} // namespace ndsloc
