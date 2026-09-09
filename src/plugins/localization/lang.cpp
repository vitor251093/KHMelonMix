#include "lang.h"

#include "utils.h"

#include <map>

namespace ndsloc {

std::string getLanguageFileName(Language language)
{
    static const std::string strings[] =
    {
        "jp",
        "en",
        "fr",
        "de",
        "it",
        "es"
    };

    return strings[language];
}

std::string getLanguageName(Language language)
{
    static const std::string strings[] =
    {
        "JP",
        "EN",
        "FR",
        "DE",
        "IT",
        "ES"
    };

    return strings[language];
}

bool shouldIgnoreFile(const std::string& filename, Language language)
{
    std::string str = filename;

    auto idx = filename.find_first_of('.');
    if (idx != std::string::npos)
    {
        str = filename.substr(0, idx);
    }

    if ((utils::ends_with(str,"en") || str.find("/en/") != std::string::npos) && language != Language::LANG_EN)
        return true;
    else if ((utils::ends_with(str,"fr") || str.find("/fr/") != std::string::npos) && language != Language::LANG_FR)
        return true;
    else if ((utils::ends_with(str,"ja") || str.find("/ja/") != std::string::npos) && language != Language::LANG_JP)
        return true;
    else if ((utils::ends_with(str,"de") || str.find("/de/") != std::string::npos) && language != Language::LANG_DE)
        return true;
    else if ((utils::ends_with(str,"es") || str.find("/es/") != std::string::npos) && language != Language::LANG_ES)
        return true;
    else if ((utils::ends_with(str,"it") || str.find("/it/") != std::string::npos) && language != Language::LANG_IT)
        return true;

    return false;
}

bool languageFromSectionName(const std::string& name, Language& language)
{
    struct LangCode
    {
        const char* code;
        Language language;
    };

    static const LangCode kCodes[] =
    {
        { "_jp", LANG_JP },
        { "_en", LANG_EN },
        { "_fr", LANG_FR },
        { "_de", LANG_DE },
        { "_it", LANG_IT },
        { "_es", LANG_ES }
    };

    if (name.size() < 3)
        return false;

    const std::string suffix = name.substr(name.size() - 3);

    for (const auto& entry : kCodes)
    {
        if (suffix == entry.code)
        {
            language = entry.language;
            return true;
        }
    }

    return false;
}

} // namespace ndsloc
