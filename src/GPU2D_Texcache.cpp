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

void Texcache2D::ApplyTextureToMemory(TexArrayEntry entry, u32& width, u32& height, s32 orig_xoff, u32 xend, s32 orig_xpos, s32 ypos,
                                      bool window, u32 spritemode, u32 pixelattr, u16* attrib, u8* objWindow, u32* objLine, Plugins::TextureEntry* textureConfig)
{
    // TODO: KH Still a bit broken
    unsigned char* imageData = entry.Data;

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
        u16 posX = textureConfig == nullptr ? 0 : textureConfig->posX;
        u16 posY = textureConfig == nullptr ? 0 : textureConfig->posY;
        int channels = 4;
        unsigned char* pixel = imageData + ((ypos + posY) * entry.FullWidth + posX + ((xpos - orig_xpos) % width)) * (channels);
        u16 color = (pixel[0] >> 3) | ((pixel[1] >> 3) << 0x5) | ((pixel[2] >> 3) << 0xA);
        bool visible = pixel[3] == 0xFF;
        if (visible) color |= 0x8000;

        if (isBitmapSprite ? (color & 0x8000) : color)
        {
            if (window) objWindow[xpos] = 1;
            else        objLine[xpos] = color | pixelattr;
        }
        else if (!window)
        {
            if (objLine[xpos] == 0)
                objLine[xpos] = pixelattr & 0x180000;
        }

        xoff++;
        xpos++;
    }
}

void Texcache2D::RetrieveTextureRowFromMemory(GPU& gpu, unsigned char*& imageData, u32& width, u32& height, s32 orig_xoff, u32 xend, s32 orig_xpos,
                                              s32 ypos, GPU2D::Unit* CurUnit, bool window, u32 spritemode, u16* attrib, u32* objLine)
{
    int channels = 4;
    
    s32 xoff = orig_xoff;
    s32 xpos = orig_xpos;

    int y = ypos;

    for (; xoff < xend;)
    {
        u32 og_pixel = objLine[xpos];

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

        unsigned char* pixel = imageData + (y * width + ((xpos - orig_xpos) % width)) * (channels);
        
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
    bool existingEntry = it != Cache.end();

    if (existingEntry && it->second.Loaded)
    {
        if (it->second.Data != nullptr) {
            // width = it->second.Width;
            // height = it->second.Height;

            Plugins::TextureEntry* textureConfig = GamePlugin->textureFileConfig(uniqueIdentifier);
            ApplyTextureToMemory(it->second, width, height, orig_xoff, xend, orig_xpos, ypos, window, spritemode, pixelattr, attrib, objWindow, objLine, textureConfig);
        }
        return;
    }

    TexArrayEntry entry = {false};
    if (existingEntry)
    {
        entry = it->second;
    }

    int channels = 4;
    int oldWidth = width;
    int oldHeight = height;
    bool loaded = false;
    unsigned char* imageData = nullptr;
    bool textureReplacementEnabled = true;
    if (textureReplacementEnabled) {
        Plugins::TextureEntry* textureConfig = GamePlugin->textureFileConfig(uniqueIdentifier);
        std::string fullPath = GamePlugin->textureFilePath(uniqueIdentifier);
        std::string fullPath2 = GamePlugin->tmpTextureFilePath(uniqueIdentifier, false);

        const char* path = fullPath.c_str();
        const char* path2 = fullPath2.c_str();

        int r_channels;
        if (strlen(path) > 0) // load complete 2D image
        {
            imageData = Texreplace::LoadTextureFromFile(path, &entry.FullWidth, &entry.FullHeight, &r_channels);
        }
        if (imageData == nullptr && strlen(path2) > 0) // load complete 2D image
        {
            imageData = Texreplace::LoadTextureFromFile(path2, &entry.FullWidth, &entry.FullHeight, &r_channels);
        }
        if (imageData == nullptr)
        {
            imageData = entry.Data;
            if (imageData == nullptr)
            {
                imageData = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char));
            }

            RetrieveTextureRowFromMemory(gpu, imageData, width, height, orig_xoff, xend, orig_xpos, ypos, CurUnit, window, spritemode, attrib, objLine);

            if (ypos + 1 == height) {
                if (GamePlugin->shouldExportTextures()) {
                    printf("Saving texture %s\n", path2);
                    Texreplace::ExportTextureAsFile(imageData, path2, width, height, channels);
                }
                loaded = true;
            }
            else {
                loaded = false;
            }
        }
        else
        {
            entry.Data = imageData;
            ApplyTextureToMemory(entry, width, height, orig_xoff, xend, orig_xpos, ypos, window, spritemode, pixelattr, attrib, objWindow, objLine, textureConfig);
            loaded = true;
        }
    }
    else {
        loaded = true;
    }

    entry.Loaded = loaded;
    entry.Width = width;
    entry.Height = height;

    if (!existingEntry)
    {
        Cache.emplace(std::make_pair(key, entry));
    }
}

}