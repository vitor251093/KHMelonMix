#include "strings.h"

#include "utils.h"

#include <algorithm>
#include <string>
#include <iomanip>
#include <fstream>
#include <assert.h>

#include "stringtable.h"

namespace ndsloc {
namespace strings {

void appendWideStrings(const std::string& filename, const std::vector<U16String>& strings, std::vector<String>& out)
{
    for (auto& str : strings)
    {
        out.emplace_back(str.offset, utf16ToUtf8(str.text), std::string(filename), true);
    }
}

void writeIniString(std::ofstream& os, int offset, const std::string& text)
{
    os << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << offset << "=";
    os << text << "\n";
}

void writeIniString(std::ofstream& os, int offset, const std::u16string& text)
{
    writeIniString(os, offset, utf16ToUtf8(text));
}

static std::string csvEscape(const std::string& in)
{
    if (in.find_first_of(",\"\r\n") == std::string::npos)
        return in;

    std::string out;
    out.reserve(in.size() + 2);
    out.push_back('"');
    for (char c : in) {
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

std::ofstream startFile(const std::string& outPathNoExt, ExportFormat format)
{
    std::string fullPath = outPathNoExt;
    fullPath += (format == ExportFormat::Csv) ? ".csv" : ".ini";

    std::ofstream os(fullPath, std::ios::binary);

    if (format == ExportFormat::Csv)
    {
        os << "\xEF\xBB\xBF";
        os << "file,subfile,offset,text\n";
    }

    return std::move(os);
}

void writeCsvLine(std::ofstream& os, const std::string& filename, const std::string& subfilename, int offset, const std::string& text)
{
    os << filename;
    os << ',' << subfilename;
    os << ",0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << offset;
    os << ',' << csvEscape(text);
    os << '\n';
}

void writeCsvLine(std::ofstream& os, const std::string& filename, const std::string& subfilename, int offset, const std::u16string& text)
{
    writeCsvLine(os, filename, subfilename, offset, utf16ToUtf8(text));
}

bool shouldIgnoreString(std::string str)
{
    if (str.empty()
        || str.find("DELETED") != std::string::npos)
        return true;

    return false;
}

const std::string& getSection(const String& entry) { return entry.section; }

const std::string& getSection(const U16String&) { static const std::string empty; return empty; }

std::string getText(const String& entry) { return entry.text; }
std::string getText(const U16String& entry) { return utf16ToUtf8(entry.text); }

template <typename T>
void writeStringsImpl(ExportFormat format, std::ofstream& os, uint32_t fileOffset, const std::string& filename, const std::vector<T>& strings)
{
    for (const auto& entry : strings)
    {
        std::string text = getText(entry);
        if (shouldIgnoreString(text))
            continue;

        utils::replace_in_string(text, "\n", "\\n");
        utils::replace_in_string(text, "\r", "\\r");

        if (format == ExportFormat::Csv)
            strings::writeCsvLine(os, filename, getSection(entry), entry.offset, text);
        else
            strings::writeIniString(os, fileOffset + entry.offset, text);
    }
}

void writeStrings(ExportFormat format, std::ofstream& os, uint32_t fileOffset, const std::string& filename, const std::vector<String>& strings)
{
    writeStringsImpl(format, os, fileOffset, filename, strings);
}
 
void writeStrings(ExportFormat format, std::ofstream& os, uint32_t fileOffset, const std::string& filename, const std::vector<U16String>& strings)
{
    writeStringsImpl(format, os, fileOffset, filename, strings);
}

void consumeBom(std::istream& is)
{
    if (is.peek() != 0xEF)
        return;

    char bom[3] = {};
    const std::streampos start = is.tellg();
    if (is.read(bom, 3) && bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF')
        return;

    is.clear();
    is.seekg(start);
}

bool readCsvRecord(std::istream& is, std::vector<std::string>& fields)
{
    fields.clear();

    std::string field;
    bool inQuotes = false;
    bool quotedField = false;
    bool gotAnything = false;

    char c = 0;
    while (is.get(c))
    {
        gotAnything = true;

        if (inQuotes)
        {
            if (c == '"')
            {
                if (is.peek() == '"')
                {
                    is.get(c);
                    field += '"';
                }
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                field += c;
            }
            continue;
        }

        if (c == '"' && field.empty() && !quotedField)
        {
            inQuotes = true;
            quotedField = true;
        }
        else if (c == ',')
        {
            fields.push_back(field);
            field.clear();
            quotedField = false;
        }
        else if (c == '\r')
        {
            // part of a CRLF line ending, ignore
        }
        else if (c == '\n')
        {
            fields.push_back(field);
            return true;
        }
        else
        {
            field += c;
        }
    }

    if (!gotAnything)
        return false;

    fields.push_back(field);
    return true;
}

std::string trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t");
    if (first == std::string::npos)
        return {};
    const auto last = s.find_last_not_of(" \t");
    return s.substr(first, last - first + 1);
}

int parseHex(const std::string& value, const char* columnName, size_t lineNumber)
{
    std::string s = trim(value);
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        s.erase(0, 2);

    assert(!s.empty());

    size_t consumed = 0;
    const unsigned long parsed = std::stoul(s, &consumed, 16);
    assert(consumed == s.size());
    return static_cast<int>(parsed);
}

std::string unescapeText(const std::string& text)
{
    std::string out;
    out.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\\' && i + 1 < text.size())
        {
            const char next = text[i + 1];
            if (next == 'n') { out += '\n'; ++i; continue; }
            if (next == 'r') { out += '\r'; ++i; continue; }
        }
        out += text[i];
    }

    return out;
}

std::vector<CsvNdsFile> readCsvFile(const std::string& inPath)
{
    std::ifstream is(inPath, std::ios::binary);
    if (!is)
    {
        assert(false);
        return {};
    }

    consumeBom(is);

    std::vector<CsvNdsFile> files;
    int currentFileId = -1;

    std::vector<std::string> fields;
    size_t lineNumber = 0;

    while (readCsvRecord(is, fields))
    {
        ++lineNumber;

        if (lineNumber == 1) // header
            continue;

        if (fields.size() == 1 && fields[0].empty()) // blank line
            continue;

        if (fields.size() < 4)
        {
            assert(false); // Invalid line (not enough columns)
            continue;
        }

        const auto& filename = fields[0];
        CsvNdsFile* currentFile = nullptr;

        if (currentFileId != -1)
        {
            assert(currentFileId < files.size());
            auto& file = files[currentFileId];
            if (filename == file.filename)
            {
                currentFile = &file;
            }
            else
            {
                currentFileId = -1;
            }
        }

        if (!currentFile)
        {
            auto found = std::find_if(files.begin(), files.end(), [filename](const auto& e) { return e.filename == filename; });
            if (found != files.end())
            {
                currentFile = &(*found);
                currentFileId = std::distance(files.begin(), found);
            }
            else
            {
                CsvNdsFile newFile;
                newFile.filename = filename;
                files.push_back(std::move(newFile));
                currentFile = &files.back();
                currentFileId = files.size() - 1;
            }
        }

        assert(currentFile);

        CsvLine entry;
        entry.offset = parseHex(fields[2], "offset", lineNumber);
        entry.text = unescapeText(fields[3]);

        std::string subfilename = fields[1];
        if (!subfilename.empty())
        {
            auto foundSub = std::find_if(currentFile->subfiles.begin(), currentFile->subfiles.end(), [subfilename](const auto& e) { return e.filename == subfilename; });
            if (foundSub != currentFile->subfiles.end())
            {
                foundSub->lines.push_back(std::move(entry));
            }
            else
            {
                CsvNdsSubfile newSubFile;
                newSubFile.filename = subfilename;
                newSubFile.lines.push_back(std::move(entry));
                currentFile->subfiles.push_back(std::move(newSubFile));
            }
        }
        else
        {
            currentFile->lines.push_back(std::move(entry));
        }
    }

    return std::move(files);
}


} // namespace strings
} // namespace ndsloc
