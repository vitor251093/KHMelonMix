#ifndef GPU3D_TEXCACHEOPENGL
#define GPU3D_TEXCACHEOPENGL

#include "GPU3D_Texcache.h"
#include "OpenGLSupport.h"

namespace GPU3D
{

template <typename, typename>
class Texcache;

class TexcacheOpenGLLoader
{
public:
    GLuint GenerateTexture(u32 width, u32 height, u32 layers);
    void UploadTexture(GLuint handle, u32 width, u32 height, u32 layer, void* data);
    void DeleteTexture(GLuint handle);

    unsigned char* LoadTextureFromFile(const char* path, int* width, int* height, int* channels);
    void ExportTextureAsFile(unsigned char* data, const char* path, u32 width, u32 height, u32 channels);
};

using TexcacheOpenGL = Texcache<TexcacheOpenGLLoader, GLuint>;

}

#endif