
#ifndef TEXREPLACE_H
#define TEXREPLACE_H

#include "types.h"

namespace Texreplace
{

using namespace melonDS;

unsigned char* LoadRawTextureFromFile(const char* path, int* width, int* height, int* channels);
unsigned char* LoadTextureFromFile(const char* path, int* width, int* height, int* channels);
void ExportRawTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);
void ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);

}

#endif
