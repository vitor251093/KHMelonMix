
#ifndef TEXREPLACE_H
#define TEXREPLACE_H

#include "types.h"

namespace Texreplace
{

using namespace melonDS;

unsigned char* LoadRGB6TextureFromFile(const char* path, int* width, int* height, int* channels);
unsigned char* LoadRGB8TextureFromFile(const char* path, int* width, int* height, int* channels);
void ExportRGB6TextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);
void ExportRGB8TextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);

}

#endif
