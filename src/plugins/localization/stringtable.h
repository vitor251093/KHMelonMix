#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace ndsloc {

enum ExportFormat : uint8_t;
struct U16String;

namespace stringtable {

enum class Format : uint8_t
{
    Unknown,
    StringDB,
    StringDB_Long,
    StringDB_Short,
    StringDB_Paged
};

struct Entry
{
    uint16_t id = 0;
    uint16_t titleLength = 0;
    uint16_t contentLength = 0;
    uint16_t extraLength = 0;
    uint32_t start = 0;
    uint32_t endTitle = 0;
    uint32_t endContent = 0;
};

struct Record
{
    uint32_t offset = 0;
    uint32_t size = 0;
};

struct PagedHeader
{
    uint32_t size = 0;
    uint32_t titleOffset = 0;
    uint32_t bodyOffset = 0;
    uint16_t id = 0;
    uint16_t extraCount = 0;
};

static const uint32_t kEntrySize = 20;
static const uint32_t kShortHeaderSize = 8;
static const uint32_t kPagedHeaderSize = 0x10;
static const uint32_t kMinRecordSize = 6;

static const char16_t kHighlightEnd = 0x0001;
static const char16_t kHighlightBegin = 0x0002;
static const char16_t kLineBreak = 0x000A;

} // namespace stringtable

class StringTableFile
{
public:
    StringTableFile(const uint8_t* inputPtr, uint32_t inputSize);

    bool extractStrings(std::vector<U16String>& out) const;

    static bool looksValid(const uint8_t* inputPtr, uint32_t inputSize);
    static uint32_t exportStrings(std::ofstream& os, const std::vector<uint8_t>& data, ndsloc::ExportFormat format, const std::string& filename, const std::string& ext, uint32_t offset);

private:
    stringtable::Format getFormat() const { return m_format; }

    static stringtable::Format detect(const uint8_t* inputPtr, uint32_t inputSize);

    bool readRecords(std::vector<stringtable::Record>& out) const;

    bool extractStringDB(std::vector<U16String>& out) const;
    bool extractStringDBLong(std::vector<U16String>& out) const;
    bool extractStringDBShort(std::vector<U16String>& out) const;
    bool extractStringDBPaged(std::vector<U16String>& out) const;

    void addString(uint32_t at, uint32_t lengthInBytes, std::vector<U16String>& out) const;

    const uint8_t* m_buffer = nullptr;
    uint32_t m_bufferSize = 0;

    stringtable::Format m_format = stringtable::Format::Unknown;
};

std::string utf16ToUtf8(const std::u16string& in);

} // namespace ndsloc
