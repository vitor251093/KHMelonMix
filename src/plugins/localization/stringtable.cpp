#include "stringtable.h"

#include "utils.h"
#include "types.h"
#include "strings.h"

#include <assert.h>

namespace ndsloc {

StringTableFile::StringTableFile(const uint8_t* inputPtr, uint32_t inputSize)
    : m_buffer(inputPtr)
    , m_bufferSize(inputSize)
    , m_format(detect(inputPtr, inputSize))
{
}

void readEntry(const uint8_t* inputPtr, uint32_t at, stringtable::Entry& entry)
{
    entry.id = utils::readUInt16(inputPtr, at);
    entry.titleLength = utils::readUInt16(inputPtr, at + 2);
    entry.contentLength = utils::readUInt16(inputPtr, at + 4);
    entry.extraLength = utils::readUInt16(inputPtr, at + 6);
    entry.start = utils::readUInt32(inputPtr, at + 8);
    entry.endTitle = utils::readUInt32(inputPtr, at + 12);
    entry.endContent = utils::readUInt32(inputPtr, at + 16);
}

void readPagedHeader(const uint8_t* inputPtr, uint32_t at, stringtable::PagedHeader& header)
{
    header.size = utils::readUInt32(inputPtr, at);
    header.titleOffset = utils::readUInt32(inputPtr, at + 4);
    header.bodyOffset = utils::readUInt32(inputPtr, at + 8);
    header.id = utils::readUInt16(inputPtr, at + 12);
    header.extraCount = utils::readUInt16(inputPtr, at + 14);
}

bool walkRecords(const uint8_t* inputPtr, uint32_t inputSize, std::vector<stringtable::Record>& out)
{
    out.clear();

    if (inputSize < stringtable::kShortHeaderSize)
        return false;

    if (utils::readUInt32(inputPtr, 0) != stringtable::kShortHeaderSize)
        return false;

    const uint32_t count = utils::readUInt32(inputPtr, 4);
    if (count == 0 || count > inputSize / stringtable::kMinRecordSize)
        return false;

    uint32_t pos = stringtable::kShortHeaderSize;

    while (pos < inputSize)
    {
        if (inputSize - pos < 4)
            return false;

        const uint32_t size = utils::readUInt24(inputPtr, pos);

        if (size == 0)
        {
            if (pos + 4 != inputSize)
                return false;

            pos += 4;
            break;
        }

        if (size < stringtable::kMinRecordSize || (size & 1) != 0 || size > inputSize - pos)
            return false;

        out.push_back({ pos, size });
        pos += size;

        if (out.size() > count)
            return false;
    }

    return pos == inputSize && out.size() == count;
}

bool pagedRecordLooksValid(const uint8_t* inputPtr, const stringtable::Record& record)
{
    if (record.size < stringtable::kPagedHeaderSize + 4)
        return false;

    stringtable::PagedHeader header;
    readPagedHeader(inputPtr, record.offset, header);

    if (header.size != record.size)
        return false;

    if (header.extraCount > (record.size - stringtable::kPagedHeaderSize) / 4)
        return false;

    if (header.titleOffset != stringtable::kPagedHeaderSize + header.extraCount * 4)
        return false;

    if (header.bodyOffset <= header.titleOffset || header.bodyOffset >= record.size)
        return false;

    if ((header.bodyOffset & 1) != 0)
        return false;

    uint32_t pos = header.bodyOffset;
    uint32_t pages = 0;

    while (pos < record.size)
    {
        if (record.size - pos < 4)
            return false;

        const uint32_t size = utils::readUInt32(inputPtr, record.offset + pos);

        if (size < stringtable::kMinRecordSize || (size & 1) != 0 || size > record.size - pos)
            return false;

        pos += size;
        ++pages;
    }

    return pos == record.size && pages > 0;
}


bool looksLikeStringDB(const uint8_t* inputPtr, uint32_t inputSize)
{
    if (inputSize < 8)
        return false;

    const uint32_t count = utils::readUInt32(inputPtr, 0);

    if (count == 0 || (count & 1) != 0)
        return false;

    if (count > (inputSize - 4) / 4)
        return false;

    const uint32_t dataAt = 4 + count * 4;
    if (dataAt >= inputSize)
        return false;

    uint32_t previous = 0;
    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t end = utils::readUInt32(inputPtr, 4 + i * 4);

        if (end < previous || (end & 1) != 0)
            return false;

        if (end > inputSize - dataAt)
            return false;

        previous = end;
    }

    return dataAt + previous == inputSize;
}

bool looksLikeStringDBLong(const uint8_t* inputPtr, uint32_t inputSize)
{
    if (inputSize < 4 + stringtable::kEntrySize)
        return false;

    const uint32_t count = utils::readUInt32(inputPtr, 0);

    if (count == 0 || count > (inputSize - 4) / stringtable::kEntrySize)
        return false;

    const uint32_t dataAt = 4 + count * stringtable::kEntrySize;
    if (dataAt >= inputSize || ((inputSize - dataAt) & 1) != 0)
        return false;

    const uint32_t units = (inputSize - dataAt) / 2;

    uint32_t expectedStart = 0;
    for (uint32_t i = 0; i < count; ++i)
    {
        stringtable::Entry entry;
        readEntry(inputPtr, 4 + i * stringtable::kEntrySize, entry);

        if (entry.start != expectedStart)
            return false;

        if (entry.start > entry.endTitle || entry.endTitle > entry.endContent)
            return false;

        if (entry.endTitle - entry.start != entry.titleLength)
            return false;

        if (entry.endContent - entry.endTitle != entry.contentLength)
            return false;

        if (entry.extraLength == 0 || entry.endContent > units - entry.extraLength)
            return false;

        expectedStart = entry.endContent + entry.extraLength;
    }

    return expectedStart == units;
}

bool looksLikeStringDBShort(const uint8_t* inputPtr, uint32_t inputSize)
{
    std::vector<stringtable::Record> records;
    return walkRecords(inputPtr, inputSize, records);
}

bool looksLikeStringDBPaged(const uint8_t* inputPtr, uint32_t inputSize)
{
    std::vector<stringtable::Record> records;

    if (!walkRecords(inputPtr, inputSize, records))
        return false;

    for (const stringtable::Record& record : records)
    {
        if (!pagedRecordLooksValid(inputPtr, record))
            return false;
    }

    return true;
}

stringtable::Format StringTableFile::detect(const uint8_t* inputPtr, uint32_t inputSize)
{
    using namespace stringtable;

    if (inputPtr == nullptr)
        return Format::Unknown;

    if (looksLikeStringDB(inputPtr, inputSize))
        return Format::StringDB;

    if (looksLikeStringDBLong(inputPtr, inputSize))
        return Format::StringDB_Long;

    if (looksLikeStringDBPaged(inputPtr, inputSize))
        return Format::StringDB_Paged;

    if (looksLikeStringDBShort(inputPtr, inputSize))
        return Format::StringDB_Short;

    return Format::Unknown;
}

void StringTableFile::addString(uint32_t at, uint32_t lengthInBytes, std::vector<U16String>& out) const
{
    if (at > m_bufferSize || lengthInBytes > m_bufferSize - at)
        return;

    std::u16string text;
    text.reserve(lengthInBytes / 2);

    for (uint32_t i = 0; i + 1 < lengthInBytes; i += 2)
    {
        const char16_t unit = static_cast<char16_t>(utils::readUInt16(m_buffer, at + i));

        if (unit == 0)
            break; // NULL terminator

        text.push_back(unit);
    }

    out.emplace_back(at, std::move(text));
}

bool StringTableFile::extractStringDB(std::vector<U16String>& out) const
{
    const uint32_t count = utils::readUInt32(m_buffer, 0);
    const uint32_t dataAt = 4 + count * 4;

    out.reserve(count);

    uint32_t start = 0;
    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t end = utils::readUInt32(m_buffer, 4 + i * 4);

        addString(dataAt + start, end - start, out);
        start = end;
    }

    return true;
}

bool StringTableFile::extractStringDBLong(std::vector<U16String>& out) const
{
    const uint32_t count = utils::readUInt32(m_buffer, 0);
    const uint32_t dataAt = 4 + count * stringtable::kEntrySize;

    out.reserve(count * 3);

    for (uint32_t i = 0; i < count; ++i)
    {
        stringtable::Entry entry;
        readEntry(m_buffer, 4 + i * stringtable::kEntrySize, entry);

        addString(dataAt + entry.start * 2, entry.titleLength * 2, out);
        addString(dataAt + entry.endTitle * 2, entry.contentLength * 2, out);
        addString(dataAt + entry.endContent * 2, entry.extraLength * 2, out);
    }

    return true;
}

bool StringTableFile::extractStringDBShort(std::vector<U16String>& out) const
{
    std::vector<stringtable::Record> records;
    if (!walkRecords(m_buffer, m_bufferSize, records))
        return false;

    out.reserve(records.size());

    for (const stringtable::Record& record : records)
        addString(record.offset + 4, record.size - 4, out);

    return true;
}

bool StringTableFile::extractStringDBPaged(std::vector<U16String>& out) const
{
    std::vector<stringtable::Record> records;
    if (!walkRecords(m_buffer, m_bufferSize, records))
        return false;

    out.reserve(records.size() * 2);

    for (const stringtable::Record& record : records)
    {
        stringtable::PagedHeader header;
        readPagedHeader(m_buffer, record.offset, header);

        addString(record.offset + header.titleOffset,
            header.bodyOffset - header.titleOffset, out);

        uint32_t pos = header.bodyOffset;
        while (pos < record.size)
        {
            const uint32_t size = utils::readUInt32(m_buffer, record.offset + pos);

            addString(record.offset + pos + 4, size - 4, out);
            pos += size;
        }
    }

    return true;
}

bool StringTableFile::readRecords(std::vector<stringtable::Record>& out) const
{
    out.clear();

    if (m_format != stringtable::Format::StringDB_Short && m_format != stringtable::Format::StringDB_Paged)
        return false;

    return walkRecords(m_buffer, m_bufferSize, out);
}

bool StringTableFile::extractStrings(std::vector<U16String>& out) const
{
    using namespace stringtable;

    out.clear();

    switch (m_format)
    {
    case Format::StringDB:
        return extractStringDB(out);
    case Format::StringDB_Long:
        return extractStringDBLong(out);
    case Format::StringDB_Short:
        return extractStringDBShort(out);
    case Format::StringDB_Paged:
        return extractStringDBPaged(out);
    default:
        return false;
    }
}

bool StringTableFile::looksValid(const uint8_t* inputPtr, uint32_t inputSize)
{
    return detect(inputPtr, inputSize) != stringtable::Format::Unknown;
}

uint32_t StringTableFile::exportStrings(std::ofstream& os, const std::vector<uint8_t>& in_data, ndsloc::ExportFormat format, const std::string& filename, const std::string& ext, uint32_t offset)
{
    std::vector<U16String> out_strings;

    const uint8_t* data = in_data.data();
    const uint32_t dataSize = static_cast<uint32_t>(in_data.size());

    StringTableFile table(data, dataSize);

    if (table.getFormat() != stringtable::Format::Unknown)
    {
        table.extractStrings(out_strings);
    }
    else
    {
        // Unrecognized format (probably not string table)
    }

    if (utils::ends_with(ext, "s.z") && table.getFormat() != stringtable::Format::StringDB_Short)
        assert(false);

    strings::writeStrings(format, os, offset, filename, out_strings);

    return out_strings.size();
}

std::string utf16ToUtf8(const std::u16string& in)
{
    std::string out;
    out.reserve(in.size());

    for (size_t i = 0; i < in.size(); ++i)
    {
        uint32_t cp = in[i];

        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < in.size())
        {
            const uint32_t low = in[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF)
            {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            }
        }

        if (cp < 0x80)
        {
            out.push_back(static_cast<char>(cp));
        }
        else if (cp < 0x800)
        {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else if (cp < 0x10000)
        {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else
        {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    return out;
}

} // namespace ndsloc
