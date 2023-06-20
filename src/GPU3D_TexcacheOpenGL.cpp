#include "GPU3D_TexcacheOpenGL.h"

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

void TexcacheOpenGLLoader::UploadTexture(GLuint handle, u32 width, u32 height, u32 layer, void* data)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0, 0, 0, layer,
        width, height, 1,
        GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, data);
}

void TexcacheOpenGLLoader::DeleteTexture(GLuint handle)
{
    glDeleteTextures(1, &handle);
}

unsigned char* TexcacheOpenGLLoader::LoadTextureFromFile(const char* path, int* width, int* height, int* channels)
{
    return stbi_load(path, width, height, channels, 0);
}

void TexcacheOpenGLLoader::ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels)
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
            alpha = (alpha + 1 >> 3) - 1;
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = alpha;
        }
    }
}
}