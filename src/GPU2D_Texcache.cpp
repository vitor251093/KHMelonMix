#include "GPU2D_Texcache.h"

#include "types.h"

#include <assert.h>
#include <vector>

#include <filesystem>
#include <fstream>

#define XXH_STATIC_LINKING_ONLY
#include "xxhash/xxhash.h"

namespace melonDS
{

void Texcache2D::ApplyTextureToMemory(TexArrayEntry* entry, s32 orig_xoff, u32 xend, s32 orig_xpos, s32 ypos,
                                      bool window, u32 spritemode, u32 pixelattr, u16* attrib, u8* objWindow, u32* objLine)
{
    // TODO: KH Still a bit broken
    unsigned char* imageData = entry->Data;

    bool isBitmapSprite = spritemode == 3;
    if (isBitmapSprite)
    {
        // bitmap sprite

        u32 alpha = attrib[2] >> 12;
        if (!alpha) return;
        alpha++;
    }

    s32 xoff = orig_xoff;
    s32 xpos = orig_xpos;

    for (; xoff < xend;)
    {
        int channels = 4;
        unsigned char* pixel = imageData + ((ypos + entry->Y) * entry->FullWidth + entry->X + ((xpos - orig_xpos) % entry->Width)) * (channels);
        u16 color = (pixel[0] >> 3) | ((pixel[1] >> 3) << 0x5) | ((pixel[2] >> 3) << 0xA);
        bool visible = pixel[3] == 0xFF;
        if (visible) color |= 0x8000;

        if (isBitmapSprite ? (color & 0x8000) : color)
        {
            if (window) objWindow[xpos] = 1;
            else {
                for (int j = 0; j < MODIFIER_2D_TEXTURE_SCALE; j++)
                    objLine[xpos*MODIFIER_2D_TEXTURE_SCALE + j] = color | pixelattr;
            }
        }
        else if (!window)
        {
            if (objLine[xpos] == 0) {
                for (int j = 0; j < MODIFIER_2D_TEXTURE_SCALE; j++)
                    objLine[xpos*MODIFIER_2D_TEXTURE_SCALE + j] = pixelattr & 0x180000;
            }
        }

        xoff++;
        xpos++;
    }
}

void Texcache2D::RetrieveTextureRowFromMemory(GPU& gpu, unsigned char*& imageData, TexArrayEntry* entry, s32 orig_xoff, u32 xend, s32 orig_xpos,
                                              s32 ypos, GPU2D::Unit* CurUnit, bool window, u32 spritemode, u16* attrib, u32* objLine)
{
    int channels = 4;
    
    s32 xoff = orig_xoff;
    s32 xpos = orig_xpos;

    for (; xoff < xend;)
    {
        u32 og_pixel = objLine[xpos*MODIFIER_2D_TEXTURE_SCALE];

        u16 alpha = 255;

        if (spritemode == 3 || (!window && ((attrib[0] & 0x2000) ? (CurUnit->DispCnt & 0x80000000) : true)))
        {
            u32 tmpAlpha = (attrib[2] & 0xF000) >> 12;
            alpha = tmpAlpha == 0 ? 0 : ((tmpAlpha + 1) << 4) - 1;
        }

        u16 color = 0;
        if (og_pixel & 0x8000) {
            color = og_pixel & 0x7FFF;
        }
        else if (og_pixel & 0x1000) {
            u16* pal = (u16*)&gpu.Palette[CurUnit->Num ? 0x600 : 0x200];
            color = pal[og_pixel & 0xFF];
        }
        else {
            u16* extpal = CurUnit->GetOBJExtPal();
            color = extpal[og_pixel & 0xFFF];
        }
        if (spritemode == 3 && (color & 0x8000) == 0) {
            alpha = 0;
        }
        if (og_pixel == 0x80000) {
            alpha = 0;
        }

        u8 r = ((color & 0x001F) << 1) << 2;
        u8 g = ((color & 0x03E0) >> 4) << 2;
        u8 b = ((color & 0x7C00) >> 9) << 2;

        unsigned char* pixel = imageData + ((ypos + entry->Y) * entry->Width + entry->X + ((xpos - orig_xpos) % entry->Width)) * (channels);
        
        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;
        pixel[3] = alpha;

        xoff++;
        xpos++;
    }
}

void Texcache2D::GetTexture(GPU& gpu, u32& width, u32& height, s32 orig_xoff, u32 xend, s32 orig_xpos, s32 ypos, GPU2D::Unit* CurUnit,
                            bool window, u32 spritemode, u32 pixelattr, u16* attrib, u8* objWindow, u32* objLine)
{
    u64 key = ((u64)attrib[0] << 32) | ((u64)attrib[1] << 16) | (u64)attrib[2];

    std::ostringstream oss;
    oss << "2d-";
    oss << attrib[0];
    oss << "-";
    oss << attrib[1];
    oss << "-";
    oss << attrib[2];
    std::string uniqueIdentifier = oss.str();

    auto it = Cache.find(key);

    TexArrayEntry* entry;
    if (it != Cache.end()) {
        entry = &it->second;
    }
    else {
        TexArrayEntry newEntry = {false, 0, 0, width, height, (s32)width, (s32)height};
        Cache.emplace(key, newEntry);
        entry = &Cache[key];
    }

    if (entry->Loaded)
    {
        if (entry->Data != nullptr) {
            ApplyTextureToMemory(entry, orig_xoff, xend, orig_xpos, ypos, window, spritemode, pixelattr, attrib, objWindow, objLine);
        }
        return;
    }

    int channels = 4;
    int oldWidth = width;
    int oldHeight = height;
    unsigned char* imageData = nullptr;
    bool textureReplacementEnabled = true;
    if (!textureReplacementEnabled) {
        entry->Loaded = true;
        return;
    }

    Plugins::TextureEntry* textureConfig = GamePlugin->textureFileConfig(uniqueIdentifier);
    std::string fullPath = GamePlugin->textureFilePath(uniqueIdentifier);
    std::string fullPath2 = GamePlugin->tmpTextureFilePath(uniqueIdentifier);
    if (textureConfig != nullptr) {
        entry->X = textureConfig->posX;
        entry->Y = textureConfig->posY;
        entry->Width = textureConfig->sizeX == 0 ? width : textureConfig->sizeX;
        entry->Height = textureConfig->sizeY == 0 ? height : textureConfig->sizeY;
        entry->FullWidth = entry->X + entry->Width;
        entry->FullHeight = entry->Y + entry->Height;
    }

    const char* path = fullPath.c_str();
    const char* path2 = fullPath2.c_str();

    int r_channels;
    if (strlen(path) > 0) // load complete 2D image
    {
        imageData = Texreplace::LoadRGB8TextureFromFile(path, &entry->FullWidth, &entry->FullHeight, &r_channels);

        if (imageData != nullptr)
        {
            entry->Data = imageData;
            ApplyTextureToMemory(entry, orig_xoff, xend, orig_xpos, ypos, window, spritemode, pixelattr, attrib, objWindow, objLine);
            entry->Loaded = true;
            return;
        }
    }

    if (entry->Data == nullptr)
    {
        entry->Data = (unsigned char*)malloc(entry->Height * entry->Width * channels * sizeof(unsigned char));
    }
    imageData = entry->Data;

    RetrieveTextureRowFromMemory(gpu, imageData, entry, orig_xoff, xend, orig_xpos, ypos, CurUnit, window, spritemode, attrib, objLine);

    entry->Loaded = ypos + 1 == entry->Height;
    if (entry->Loaded && GamePlugin->shouldExportTextures()) {
        printf("Saving texture %s\n", path2);
        Texreplace::ExportRGB8TextureAsFile(imageData, path2, entry->Width, entry->Height, channels);
    }
}

}