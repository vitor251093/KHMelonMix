#include "cakp.h"

#include "utils.h"
#include "types.h"
#include "lang.h"

#include <algorithm>
#include <cstring>
#include <assert.h>

namespace ndsloc {

namespace cakp {

bool isCAKP(const uint8_t* buffer)
{
    return (std::memcmp(buffer, "CAKP", 4) == 0);
}

bool isTextOpcode(uint8_t group, uint8_t opcode)
{
    return group == 0x03 && opcode == 0x01;  // show_subtitle
}

} // namespace cakp

CAKPFile::CAKPFile(const uint8_t* inputPtr, uint32_t inputSize, std::string sectionName)
    : m_buffer(inputPtr)
    , m_bufferSize(inputSize)
    , m_language(LANG_EN)
    , m_sectionName(std::move(sectionName))
{
}

bool CAKPFile::inRange(uint32_t offset, uint32_t length) const
{
    return offset <= m_bufferSize && length <= m_bufferSize - offset;
}

bool CAKPFile::readDirectory(uint32_t offset, Directory& dir)
{
    if (offset == kUnused)
        return false;

    dir.count = utils::readUInt32(m_buffer, offset);

    if (dir.count > m_bufferSize / 8)
    {
        assert(false);
        return false;
    }

    dir.offsetsAt = offset + 4;
    dir.sizesAt = offset + 4 + dir.count * 4;

    return inRange(dir.offsetsAt, dir.count * 8);
}

bool CAKPFile::directoryEntry(const Directory& dir, uint32_t index, uint32_t& offset, uint32_t& size)
{
    if (index >= dir.count)
        return false;

    offset = utils::readUInt32(m_buffer, dir.offsetsAt + index * 4);
    size = utils::readUInt32(m_buffer, dir.sizesAt + index * 4);

    return true;
}

void CAKPFile::addString(uint32_t offset, uint32_t limit, bool allowDuplicates, std::vector<String>& out)
{
    if (offset >= limit || limit > m_bufferSize) {
        return;
    }
    if (!allowDuplicates && std::find(m_seen.begin(), m_seen.end(), offset) != m_seen.end()) {
        return;
    }

    const uint8_t* const begin = m_buffer + offset;
    const uint8_t* const end = m_buffer + limit;
    const uint8_t* p = begin;
    while (p < end && *p != 0) {
        ++p;
    }
    if (p == end) {
        return;
    }

    m_seen.push_back(offset);

    std::string text(reinterpret_cast<const char*>(begin), static_cast<size_t>(p - begin));
    out.emplace_back(offset, std::move(text), std::string(m_sectionName), false);
}

void CAKPFile::collectMessageRecord(uint32_t recordAt, uint32_t poolAt, uint32_t sectionEnd, std::vector<String>& out)
{
    if (!inRange(recordAt, kMessageSlots * 4) || recordAt + kMessageSlots * 4 > sectionEnd)
        return;

    for (uint32_t slot = 0; slot < kLanguageCount; ++slot)
    {
        if (slot == m_language)
        {
            uint32_t textRel = utils::readUInt32(m_buffer, recordAt + slot * 4);

            if (textRel == kUnused) // 0xFFFFFFFF means "no text"
                continue;

            addString(poolAt + textRel, sectionEnd, false, out);
        }
    }
}

bool CAKPFile::readLanguageCondition(uint32_t conditionAt, uint32_t sectionEnd, uint16_t& language) const
{
    if (!inRange(conditionAt, kConditionSize) || conditionAt + kConditionSize > sectionEnd)
        return false;

    if (utils::readUInt16(m_buffer, conditionAt + kConditionVarIdAt) != kLanguageVarId)
        return false;

    language = utils::readUInt16(m_buffer, conditionAt + kConditionValueAt);
    return true;
}

bool CAKPFile::collectSection(uint32_t sectionAt, uint32_t sectionSize, std::vector<String>& out)
{
    if (!inRange(sectionAt, sectionSize) || sectionSize < 8)
        return false;

    const uint32_t sectionEnd = sectionAt + sectionSize;

    uint32_t poolRel = utils::readUInt32(m_buffer, sectionAt);

    if (poolRel < 4 || poolRel > sectionSize)
        return false;

    const uint32_t poolAt = sectionAt + poolRel;

    const bool sectionLanguageMatches = (m_sectionLanguage >= 0) && (m_sectionLanguage == static_cast<int>(m_language));
    uint32_t languageBlockEnd = 0;
    bool inWantedLanguageBlock = sectionLanguageMatches;

    uint32_t ip = sectionAt + 4;
    while (ip + sizeof(cakp::Instruction) <= poolAt)
    {
        const cakp::Instruction* instr = reinterpret_cast<const cakp::Instruction*>(m_buffer + ip);

        if (instr->sizeInDwords == 0)
            return false;

        const uint32_t next = ip + static_cast<uint32_t>(instr->sizeInDwords) * 4;
        if (next > poolAt)
            return false;

        if (languageBlockEnd != 0 && ip - sectionAt >= languageBlockEnd)
        {
            languageBlockEnd = 0;
            inWantedLanguageBlock = sectionLanguageMatches;
        }

        const uint32_t argCount = instr->sizeInDwords - 1;

        const bool typedArgs = !(instr->group == 0x00 && instr->opcode == 0x05);
        if (typedArgs)
        {
            for (uint32_t i = 0; i + 1 < argCount; i += 2)
            {
                uint32_t type = utils::readUInt32(m_buffer, ip + 4 + i * 4);
                uint32_t value = utils::readUInt32(m_buffer, ip + 8 + i * 4);

                if (type == cakp::ARG_STR)
                {
                    if (inWantedLanguageBlock && cakp::isTextOpcode(instr->group, instr->opcode))
                        addString(poolAt + value, sectionEnd, false, out);
                }
                else if (type == cakp::ARG_MSG)
                {
                    collectMessageRecord(poolAt + value, poolAt, sectionEnd, out);
                }
            }
        }
        else if (argCount >= 2)
        {
            const uint32_t conditionAt = poolAt + utils::readUInt32(m_buffer, ip + 4);
            const uint32_t skipTarget = utils::readUInt32(m_buffer, ip + 8);

            uint16_t conditionLanguage = 0;
            if (readLanguageCondition(conditionAt, sectionEnd, conditionLanguage))
            {
                languageBlockEnd = skipTarget;
                inWantedLanguageBlock = (conditionLanguage == m_language);
            }
        }

        ip = next;
    }
    return true;
}

bool CAKPFile::extractStrings(std::vector<String>& out)
{
    out.clear();
    m_seen.clear();

    assert(cakp::isCAKP(m_buffer));

    const cakp::FileHeader* header = reinterpret_cast<const cakp::FileHeader*>(m_buffer);

    Directory nameDir;
    Directory sectionDir;
    if (!readDirectory(header->fileSlots[0], nameDir) ||
        !readDirectory(header->fileSlots[1], sectionDir))
        return false;

    std::vector<std::string> sectionNames;
    readSectionNames(nameDir, sectionNames);

    bool ok = true;
    for (uint32_t i = 0; i < sectionDir.count; ++i) {
        uint32_t sectionAt = 0;
        uint32_t sectionSize = 0;
        if (!directoryEntry(sectionDir, i, sectionAt, sectionSize))
        {
            ok = false;
            break;
        }

        m_sectionLanguage = -1;
        if (i < sectionNames.size())
        {
            Language sectionLanguage = LANG_JP;
            if (languageFromSectionName(sectionNames[i], sectionLanguage))
                m_sectionLanguage = static_cast<int>(sectionLanguage);
        }

        if (!collectSection(sectionAt, sectionSize, out)) {
            ok = false;
        }
    }

    m_sectionLanguage = -1;

    std::sort(out.begin(), out.end(), [](const String& a, const String& b) { return a.offset < b.offset; });
    return ok;
}

bool CAKPFile::readSectionNames(const Directory& nameDir, std::vector<std::string>& out)
{
    out.clear();

    uint32_t nameTableAt = 0;
    uint32_t nameTableSize = 0;
    if (!directoryEntry(nameDir, 0, nameTableAt, nameTableSize) || !inRange(nameTableAt, nameTableSize) || nameTableSize < 2)
        return false;

    const uint16_t nameCount = utils::readUInt16(m_buffer, nameTableAt);
    const uint32_t nameTableEnd = nameTableAt + nameTableSize;

    if (2 + static_cast<uint32_t>(nameCount) * 2 > nameTableSize)
        return false;

    out.reserve(nameCount);

    for (uint16_t i = 0; i < nameCount; ++i)
    {
        const uint16_t rel = utils::readUInt16(m_buffer, nameTableAt + 2 + i * 2);
        const uint32_t at = nameTableAt + rel;

        if (at >= nameTableEnd)
        {
            out.emplace_back();
            continue;
        }

        const uint8_t* const begin = m_buffer + at;
        const uint8_t* p = begin;
        while (p < m_buffer + nameTableEnd && *p != 0)
            ++p;

        out.emplace_back(reinterpret_cast<const char*>(begin), static_cast<size_t>(p - begin));
    }

    return true;
}

bool CAKPFile::extractSectionStrings(std::vector<String>& out)
{
    out.clear();
    m_seen.clear();
    m_sectionLanguage = -1;
    return collectSection(0, m_bufferSize, out);
}

} // namespace ndsloc
