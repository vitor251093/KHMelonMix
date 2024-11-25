#ifndef MELONDS_ANDROID_ANDROIDARCODEFILE_H
#define MELONDS_ANDROID_ANDROIDARCODEFILE_H

#include "../ARCodeFile.h"

class AndroidARCodeFile : public ARCodeFile {
public:
    AndroidARCodeFile();
    void updateCodeList(ARCodeList list);
};

#endif //MELONDS_ANDROID_ANDROIDARCODEFILE_H
