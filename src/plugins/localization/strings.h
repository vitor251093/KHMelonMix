#pragma once

#include <vector>
#include <string>

#include "types.h"

namespace ndsloc {
enum EFileFormat : uint8_t;

namespace strings {

void appendWideStrings(const std::string& filename, const std::vector<U16String>& strings, std::vector<String>& out);

std::ofstream startFile(const std::string& outPathNoExt, ExportFormat format);
void writeStrings(ExportFormat format, std::ofstream& os, uint32_t fileOffset, const std::string& filename, const std::vector<String>& strings);
void writeStrings(ExportFormat format, std::ofstream& os, uint32_t fileOffset, const std::string& filename, const std::vector<U16String>& strings);

struct CsvLine
{
    int offset = 0;
    std::string text;
};

struct CsvNdsSubfile
{
    std::string filename;

    std::vector<CsvLine> lines;
};

struct CsvNdsFile
{
    std::string filename;

    std::vector<CsvLine> lines;
    std::vector<CsvNdsSubfile> subfiles;
};


std::vector<CsvNdsFile> readCsvFile(const std::string& inPath);

} // namespace strings
} // namespace ndsloc
