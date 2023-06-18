#include "GPU3D_TexcacheOpenGL.h"

#include <filesystem>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace GPU3D
{

GLuint TexcacheOpenGLLoader::GenerateTexture(u32 width, u32 height, u32 layers)
{
    GLuint texarray;
    glGenTextures(1, &texarray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8UI, width, height, layers);
    return texarray;
}

void TexcacheOpenGLLoader::UploadTexture(u32 addr, GLuint handle, u32 width, u32 height, u32 layer, void* data)
{
    printf("TexcacheOpenGLLoader::UploadTexture(%u, %u, %d, %d, %d, ?)\n", addr, handle, width, height, layer);
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::string filename = std::to_string(addr) + ".png";
    std::filesystem::path fullPath = currentPath / "textures" / filename;
    const char* path = fullPath.u8string().c_str();

    int channels = 4;
    int r_width, r_height, r_channels;
    unsigned char* imageData = stbi_load(path, &r_width, &r_height, &r_channels, 0);
    bool cachedImage = (imageData != nullptr);
    if (imageData == nullptr) 
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
                pixel[0] = r;
                pixel[1] = g;
                pixel[2] = b;
                pixel[3] = alpha;
            }
        }

        stbi_write_png(path, width, height, channels, data, width * channels);

        r_width = width;
        r_height = height;
        r_channels = channels;
        imageData = (unsigned char*)data;
    }
    
    if (r_channels == channels) {
        for (int y = 0; y < r_height; ++y) {
            for (int x = 0; x < r_width; ++x) {
                unsigned char* pixel = imageData + (y * r_width + x) * (channels);
                unsigned char r = pixel[0];
                unsigned char g = pixel[1];
                unsigned char b = pixel[2];
                unsigned char alpha = pixel[3];
                r = (r >> 2);
                g = (g >> 2);
                b = (b >> 2);
                alpha = (alpha + 1 >> 3) - 1;
                pixel[0] = r;
                pixel[1] = g;
                pixel[2] = b;
                pixel[3] = alpha;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0, 0, 0, layer,
        width, height, 1,
        GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, imageData);

    if (cachedImage) {
        stbi_image_free(imageData);
    }
}

void TexcacheOpenGLLoader::DeleteTexture(GLuint handle)
{
    glDeleteTextures(1, &handle);
}

}