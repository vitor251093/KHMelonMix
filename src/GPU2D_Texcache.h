#ifndef GPU2D_TEXCACHE
#define GPU2D_TEXCACHE

#include "GPU_Texreplace.h"

#include "GPU2D.h"

#include <unordered_map>

#include "plugins/PluginManager.h"

namespace melonDS
{

class Texcache2D
{
public:
    Texcache2D() {}

    Plugins::Plugin* GamePlugin;

    void GetTexture(GPU& gpu, u32& width, u32& height, s32 orig_xoff, u32 xend, s32 orig_xpos, s32 ypos, GPU2D::Unit* CurUnit,
                    bool window, u32 spritemode, u32 pixelattr, u16* attrib, u8* objWindow, u32* objLine);

private:
    struct TexArrayEntry
    {
        bool Loaded;
        unsigned char* Data;
        u32 Width;
        u32 Height;
        s32 FullWidth;
        s32 FullHeight;
    };

    std::unordered_map<u64, TexArrayEntry> Cache;

    void ApplyTextureToMemory(TexArrayEntry entry, u32& width, u32& height, s32 orig_xoff, u32 xend, s32 orig_xpos, s32 ypos, bool window,
                              u32 spritemode, u32 pixelattr, u16* attrib, u8* objWindow, u32* objLine, Plugins::TextureEntry* textureConfig);
    void RetrieveTextureRowFromMemory(GPU& gpu, unsigned char*& imageData, u32& width, u32& height, s32 orig_xoff, u32 xend,
                             s32 orig_xpos, s32 ypos, GPU2D::Unit* CurUnit, bool window, u32 spritemode, u16* attrib, u32* objLine);

};

}

#endif
