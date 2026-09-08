#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace NDS {
struct NDSEntry;
class NDSFileSystem;
}

namespace ndsloc {

namespace strings {
struct CsvNdsFile;
struct CsvLine;
}

namespace patcher {

uint32_t createPatch(const std::string& romPath, const std::vector<strings::CsvNdsFile>& modFiles);
uint32_t createPatch(uint8_t* rom, uint32_t romSize, const std::vector<strings::CsvNdsFile>& modFiles);

} // namespace patcher
} // namespace ndsloc
