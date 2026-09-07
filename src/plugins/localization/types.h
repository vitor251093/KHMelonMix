#pragma once

#include <stdint.h>
#include <string>

namespace ndsloc {

enum ExportFormat : uint8_t { Csv, Ini };

struct String
{
    String(uint32_t in_off, std::string&& in_text, std::string&& in_section, bool in_wide)
        : offset(in_off)
        , text(std::move(in_text))
        , section(std::move(in_section))
        , bIsWide(in_wide)
    {
    }

    uint32_t offset = 0;
    std::string text;

    std::string section;
    bool bIsWide = false;
};

struct U16String
{
    U16String(uint32_t in_off, std::u16string&& in_text)
        : offset(in_off)
        , text(std::move(in_text))
    { }

    uint32_t offset = 0;
    std::u16string text;
};

} // namespace ndsloc
