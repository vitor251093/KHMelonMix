#include "FileUtils.h"
#include <cstring>
#include <cstdlib>

namespace MelonDSAndroid {
    char* joinPaths(const char* path1, const char* path2)
    {
        int path1Len = strlen(path1);
        int path2Len = strlen(path2);
        int path2StartOffset = 0;

        if (path1[path1Len - 1] == '/')
        {
            path1Len--;
        }
        if (path2[path2Len - 1] == '/')
        {
            path2Len--;
            path2StartOffset = 1;
        }

        // Add 1 extra char for the slash separator and another one for the null terminator
        char* finalPath = (char*) malloc((path1Len + 1 + path2Len + 1) * sizeof(char));

        // Doesn't matter if the extra slash is copied. It will be overwritten
        strcpy(finalPath, path1);
        finalPath[path1Len] = '/';
        strcpy(&finalPath[path1Len + 1], &path2[path2StartOffset]);
        return finalPath;
    }
}