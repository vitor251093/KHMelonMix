#include "AndroidARCodeFile.h"

AndroidARCodeFile::AndroidARCodeFile() : ARCodeFile("nothing") {
}

void AndroidARCodeFile::updateCodeList(ARCodeList list) {
    this->Categories.clear();

    // Put all codes into a default category
    ARCodeCat defaultCategory = {
            .Name = "Default"
    };

    for (ARCodeList::iterator it = list.begin(); it != list.end(); it++)
    {
        ARCode& code = *it;
        defaultCategory.Codes.push_back(code);
    }

    this->Categories.push_back(defaultCategory);
}
