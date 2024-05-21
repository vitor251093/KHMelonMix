#include "GPU_Texreplace.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Texreplace
{

unsigned char* LoadTextureFromFile(const char* path, int* width, int* height, int* channels)
{
    return stbi_load(path, width, height, channels, 0);
}

void ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = ((unsigned char*)data) + (y * width + x) * (channels);
            unsigned char r = pixel[0];
            unsigned char g = pixel[1];
            unsigned char b = pixel[2];
            unsigned char alpha = pixel[3];
            r = (r << 2);
            g = (g << 2);
            b = (b << 2);
            alpha = (alpha + 1 << 3) - 1;
            if (alpha <= 0x7) alpha = 0;
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = alpha;
        }
    }

    stbi_write_png(path, width, height, channels, data, width * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = ((unsigned char*)data) + (y * width + x) * (channels);
            unsigned char r = pixel[0];
            unsigned char g = pixel[1];
            unsigned char b = pixel[2];
            unsigned char alpha = pixel[3];
            r = (r >> 2);
            g = (g >> 2);
            b = (b >> 2);
            alpha = (alpha >> 3);
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = alpha;
        }
    }
}
}