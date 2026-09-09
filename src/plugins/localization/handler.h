#pragma once

#include <string>

namespace Plugins {

class LocalizationHandler
{
public:
    virtual std::string getLocalizationFilePath(std::string language) = 0;
    virtual bool shouldExcludeLocalizationSubfile(std::string filename) { return false; }
};

} // namespace Plugins
