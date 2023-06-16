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

void TexcacheOpenGLLoader::UploadTexture(GLuint handle, u32 width, u32 height, u32 layer, void* data)
{
    printf("TexcacheOpenGLLoader::UploadTexture(%u, %d, %d, %d, %p)\n", handle, width, height, layer, data);
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::string filename = std::to_string(handle) + ".png";
    std::filesystem::path fullPath = currentPath / "textures" / filename;
    const char* path = fullPath.c_str();

    int r_width, r_height, r_channels;
    unsigned char* imageData = stbi_load(path, &r_width, &r_height, &r_channels, 0);
    if (imageData != nullptr) 
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 0, 0, layer,
            width, height, 1,
            GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, imageData);

        stbi_image_free(imageData);
    }
    else {
        int channels = 4;
        stbi_write_png(path, width, height, channels, data, width * channels);

        glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 0, 0, layer,
            width, height, 1,
            GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, data);
    }
}

void TexcacheOpenGLLoader::DeleteTexture(GLuint handle)
{
    glDeleteTextures(1, &handle);
}

}