#include "GPU_Texreplace.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Texreplace
{

#define CHANNELS_RGB  3
#define CHANNELS_RGBA 4

unsigned char* LoadTextureFromFile(const char* path, int* width, int* height, int* channels)
{
    unsigned char* imageData = stbi_load(path, width, height, channels, 0);
    if (imageData == nullptr) {
        return nullptr;
    }

    // Convert RGB to RGBA (DS GPU expects 4-channel textures)
    if (*channels == CHANNELS_RGB) {
        unsigned char* newImageData = (unsigned char*)malloc(
            (*height) * (*width) * CHANNELS_RGBA * sizeof(unsigned char));
        for (int y = 0; y < (*height); ++y) {
            for (int x = 0; x < (*width); ++x) {
                unsigned char* oldPixel = imageData + (y * (*width) + x) * CHANNELS_RGB;
                unsigned char* newPixel = newImageData + (y * (*width) + x) * CHANNELS_RGBA;
                newPixel[0] = oldPixel[0];
                newPixel[1] = oldPixel[1];
                newPixel[2] = oldPixel[2];
                newPixel[3] = 255;
            }
        }
        stbi_image_free(imageData);
        imageData = newImageData;
        *channels = CHANNELS_RGBA;
    }
    for (int y = 0; y < (*height); ++y) {
        for (int x = 0; x < (*width); ++x) {
            unsigned char* pixel = imageData + (y * (*width) + x) * CHANNELS_RGBA;
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

    return imageData;
}

void ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels)
{
    unsigned char* newImageData = (unsigned char*)malloc((height) * (width) * (channels) * sizeof(unsigned char[4]));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = ((unsigned char*)data) + (y * width + x) * (channels);
            unsigned char* newPixel = ((unsigned char*)newImageData) + (y * width + x) * (channels);
            unsigned char r = pixel[0];
            unsigned char g = pixel[1];
            unsigned char b = pixel[2];
            unsigned char alpha = pixel[3];
            r = (r << 2);
            g = (g << 2);
            b = (b << 2);
            alpha = (alpha + 1 << 3) - 1;
            if (alpha <= 0x7) alpha = 0;
            newPixel[0] = r;
            newPixel[1] = g;
            newPixel[2] = b;
            newPixel[3] = alpha;
        }
    }

    stbi_write_png(path, width, height, channels, newImageData, width * channels);
    free(newImageData);
}
}