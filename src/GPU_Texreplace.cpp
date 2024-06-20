#include "GPU_Texreplace.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Texreplace
{

unsigned char* LoadTextureFromFile(const char* path, bool asDsTexture, int* width, int* height, int* channels)
{
    unsigned char* imageData = stbi_load(path, width, height, channels, 0);
    if (imageData == nullptr) {
        return nullptr;
    }

    if (*channels == 3) {
        unsigned char* newImageData = (unsigned char*)malloc((*height) * (*width) * (*channels) * sizeof(unsigned char[4]));
        for (int y = 0; y < (*height); ++y) {
            for (int x = 0; x < (*width); ++x) {
                unsigned char* old_pixel = imageData + (y * (*width) + x) * 3;
                unsigned char* new_pixel = newImageData + (y * (*width) + x) * 4;
                new_pixel[0] = old_pixel[0];
                new_pixel[1] = old_pixel[1];
                new_pixel[2] = old_pixel[2];
                new_pixel[3] = 255;
            }
        }
        imageData = newImageData;
        *channels = 4;
    }
    if (asDsTexture) {
        for (int y = 0; y < (*height); ++y) {
            for (int x = 0; x < (*width); ++x) {
                unsigned char* pixel = imageData + (y * (*width) + x) * 4;
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

    return imageData;
}

void ExportTextureAsFile(unsigned char* data, bool isDsTexture, const char* path, u32 width, u32 height, u32 channels)
{
    unsigned char* newImageData = (unsigned char*)malloc((height) * (width) * (channels) * sizeof(unsigned char[4]));

    if (isDsTexture) {
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

        data = newImageData;
    }

    stbi_write_png(path, width, height, channels, data, width * channels);
}
}