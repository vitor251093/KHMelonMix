
#ifndef TEXREPLACE_H
#define TEXREPLACE_H

#include "OpenGLSupport.h"

namespace Texreplace
{

unsigned char* LoadTextureFromFile(const char* path, int* width, int* height, int* channels);
void ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);

}

#endif
