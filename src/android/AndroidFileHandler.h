#ifndef MELONDS_ANDROID_ANDROIDFILEHANDLER_H
#define MELONDS_ANDROID_ANDROIDFILEHANDLER_H

#include <stdio.h>

namespace MelonDSAndroid
{
    class AndroidFileHandler {
    public:
        virtual FILE* open(const char* path, const char* mode) = 0;
        virtual ~AndroidFileHandler() {};
    };
}

#endif //MELONDS_ANDROID_ANDROIDFILEHANDLER_H
