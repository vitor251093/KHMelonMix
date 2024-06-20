
#ifndef TEXREPLACE_H
#define TEXREPLACE_H

#include "types.h"

namespace Texreplace
{

using namespace melonDS;

unsigned char* LoadTextureFromFile(const char* path, bool asDsTexture, int* width, int* height, int* channels);
void ExportTextureAsFile(unsigned char* data, bool isDsTexture, const char* path, u32 width, u32 height, u32 channels);

}

#endif
