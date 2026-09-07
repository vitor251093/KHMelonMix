#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace ndsloc {

enum Language : uint8_t
{
    LANG_JP = 0,
    LANG_EN = 1,
    LANG_FR = 2,
    LANG_DE = 3,
    LANG_IT = 4,
    LANG_ES = 5,
    LANG_COUNT = 6
};

bool languageFromSectionName(const std::string& name, Language& language);
std::string getLanguageFileName(Language language);
std::string getLanguageName(Language language);
bool shouldIgnoreFile(const std::string& filename, Language language);

} // namespace ndsloc
