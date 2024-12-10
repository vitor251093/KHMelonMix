#ifndef GPU3D_TEXCACHE
#define GPU3D_TEXCACHE

#include "types.h"
#include "GPU.h"
#include "GPU_Texreplace.h"

#include <assert.h>
#include <unordered_map>
#include <vector>

#include <filesystem>
#include <fstream>

#include "plugins/PluginManager.h"

#define XXH_STATIC_LINKING_ONLY
#include "xxhash/xxhash.h"

namespace melonDS
{

enum
{
    outputFmt_RGB6A5,
    outputFmt_RGBA8,
    outputFmt_BGRA8
};

template <int outputFmt>
void ConvertBitmapTexture(u32 width, u32 height, u32* output, u32 addr, GPU& gpu);
template <int outputFmt>
void ConvertCompressedTexture(u32 width, u32 height, u32* output, u32 addr, u32 addrAux, u32 palAddr, GPU& gpu);
template <int outputFmt, int X, int Y>
void ConvertAXIYTexture(u32 width, u32 height, u32* output, u32 addr, u32 palAddr, GPU& gpu);
template <int outputFmt, int colorBits>
void ConvertNColorsTexture(u32 width, u32 height, u32* output, u32 addr, u32 palAddr, bool color0Transparent, GPU& gpu);

template <typename TexLoaderT, typename TexHandleT>
class Texcache
{
public:
    Texcache(const TexLoaderT& texloader)
        : TexLoader(texloader) // probably better if this would be a move constructor???
    {}

    Plugins::Plugin* GamePlugin;

    u64 MaskedHash(u8* vram, u32 vramSize, u32 addr, u32 size)
    {
        u64 hash = 0;

        while (size > 0)
        {
            u32 pieceSize;
            if (addr + size > vramSize)
                // wraps around, only do the part inside
                pieceSize = vramSize - addr;
            else
                // fits completely inside
                pieceSize = size;

            hash = XXH64(&vram[addr], pieceSize, hash);

            addr += pieceSize;
            addr &= (vramSize - 1);
            assert(size >= pieceSize);
            size -= pieceSize;
        }

        return hash;
    }

    bool CheckInvalid(u32 start, u32 size, u64 oldHash, u64* dirty, u8* vram, u32 vramSize)
    {
        u32 startBit = start / VRAMDirtyGranularity;
        u32 bitsCount = ((start + size + VRAMDirtyGranularity - 1) / VRAMDirtyGranularity) - startBit;
    
        u32 startEntry = startBit >> 6;
        u64 entriesCount = ((startBit + bitsCount + 0x3F) >> 6) - startEntry;
        for (u32 j = startEntry; j < startEntry + entriesCount; j++)
        {
            if (GetRangedBitMask(j, startBit, bitsCount) & dirty[j & ((vramSize / VRAMDirtyGranularity)-1)])
            {
                if (MaskedHash(vram, vramSize, start, size) != oldHash)
                    return true;
            }
        }

        return false;
    }

    bool Update(GPU& gpu)
    {
        auto textureDirty = gpu.VRAMDirty_Texture.DeriveState(gpu.VRAMMap_Texture, gpu);
        auto texPalDirty = gpu.VRAMDirty_TexPal.DeriveState(gpu.VRAMMap_TexPal, gpu);

        bool textureChanged = gpu.MakeVRAMFlat_TextureCoherent(textureDirty);
        bool texPalChanged = gpu.MakeVRAMFlat_TexPalCoherent(texPalDirty);

        if (textureChanged || texPalChanged)
        {
            //printf("check invalidation %d\n", TexCache.size());
            for (auto it = Cache.begin(); it != Cache.end();)
            {
                TexCacheEntry& entry = it->second;
                if (textureChanged)
                {
                    for (u32 i = 0; i < 2; i++)
                    {
                        if (CheckInvalid(entry.TextureRAMStart[i], entry.TextureRAMSize[i],
                                entry.TextureHash[i],
                                textureDirty.Data,
                                gpu.VRAMFlat_Texture, sizeof(gpu.VRAMFlat_Texture)))
                            goto invalidate;
                    }
                }

                if (texPalChanged && entry.TexPalSize > 0)
                {
                    if (CheckInvalid(entry.TexPalStart, entry.TexPalSize,
                            entry.TexPalHash,
                            texPalDirty.Data,
                            gpu.VRAMFlat_TexPal, sizeof(gpu.VRAMFlat_TexPal)))
                        goto invalidate;
                }

                it++;
                continue;
            invalidate:
                FreeTextures[entry.WidthLog2][entry.HeightLog2].push_back(entry.Texture);

                //printf("invalidating texture %d\n", entry.ImageDescriptor);

                it = Cache.erase(it);
            }

            return true;
        }

        return false;
    }

    int RightmostBit(u32 num) {
        int c = 0;
        if (num == 0) {
            return 0;
        }
        while ((num & 1) == 0) {
            num >>= 1;
            c++;
        }
        return c;
    }

    void GetTexture(GPU& gpu, u32 texParam, u32& width, u32& height, u32 palBase, TexHandleT& textureHandle, u32& layer, u32*& helper)
    {
        // remove sampling and texcoord gen params
        texParam &= ~0xC00F0000;

        u32 fmt = (texParam >> 26) & 0x7;
        u64 key = texParam;
        if (fmt != 7)
        {
            key |= (u64)palBase << 32;
            if (fmt == 5)
                key &= ~((u64)1 << 29);
        }
        //printf("%" PRIx64 " %" PRIx32 " %" PRIx32 "\n", key, texParam, palBase);

        assert(fmt != 0 && "no texture is not a texture format!");

        auto it = Cache.find(key);

        if (it != Cache.end())
        {
            textureHandle = it->second.Texture.TextureID;
            layer = it->second.Texture.Layer;
            width = it->second.Texture.Width;
            height = it->second.Texture.Height;
            helper = &it->second.LastVariant;
            return;
        }

        u32 widthLog2 = (texParam >> 20) & 0x7;
        u32 heightLog2 = (texParam >> 23) & 0x7;
        width = 8 << widthLog2;
        height = 8 << heightLog2;

        u32 addr = (texParam & 0xFFFF) * 8;

        TexCacheEntry entry = {0};

        entry.TextureRAMStart[0] = addr;
        entry.WidthLog2 = widthLog2;
        entry.HeightLog2 = heightLog2;

        // apparently a new texture
        if (fmt == 7)
        {
            entry.TextureRAMSize[0] = width*height*2;

            ConvertBitmapTexture<outputFmt_RGB6A5>(width, height, DecodingBuffer, addr, gpu);
        }
        else if (fmt == 5)
        {
            u32 slot1addr = 0x20000 + ((addr & 0x1FFFC) >> 1);
            if (addr >= 0x40000)
                slot1addr += 0x10000;

            entry.TextureRAMSize[0] = width*height/16*4;
            entry.TextureRAMStart[1] = slot1addr;
            entry.TextureRAMSize[1] = width*height/16*2;
            entry.TexPalStart = palBase*16;
            entry.TexPalSize = 0x10000;

            ConvertCompressedTexture<outputFmt_RGB6A5>(width, height, DecodingBuffer, addr, slot1addr, entry.TexPalStart, gpu);
        }
        else
        {
            u32 texSize, palAddr = palBase*16, numPalEntries;
            switch (fmt)
            {
            case 1: texSize = width*height; numPalEntries = 32; break;
            case 6: texSize = width*height; numPalEntries = 8; break;
            case 2: texSize = width*height/4; numPalEntries = 4; palAddr >>= 1; break;
            case 3: texSize = width*height/2; numPalEntries = 16; break;
            case 4: texSize = width*height; numPalEntries = 256; break;
            }

            palAddr &= 0x1FFFF;

            /*printf("creating texture | fmt: %d | %dx%d | %08x | %08x\n", fmt, width, height, addr, palAddr);
            svcSleepThread(1000*1000);*/

            entry.TextureRAMSize[0] = texSize;
            entry.TexPalStart = palAddr;
            entry.TexPalSize = numPalEntries*2;

            //assert(entry.TexPalStart+entry.TexPalSize <= 128*1024*1024);

            bool color0Transparent = texParam & (1 << 29);

            switch (fmt)
            {
            case 1: ConvertAXIYTexture<outputFmt_RGB6A5, 3, 5>(width, height, DecodingBuffer, addr, palAddr, gpu); break;
            case 6: ConvertAXIYTexture<outputFmt_RGB6A5, 5, 3>(width, height, DecodingBuffer, addr, palAddr, gpu); break;
            case 2: ConvertNColorsTexture<outputFmt_RGB6A5, 2>(width, height, DecodingBuffer, addr, palAddr, color0Transparent, gpu); break;
            case 3: ConvertNColorsTexture<outputFmt_RGB6A5, 4>(width, height, DecodingBuffer, addr, palAddr, color0Transparent, gpu); break;
            case 4: ConvertNColorsTexture<outputFmt_RGB6A5, 8>(width, height, DecodingBuffer, addr, palAddr, color0Transparent, gpu); break;
            }
        }

        for (int i = 0; i < 2; i++)
        {
            if (entry.TextureRAMSize[i])
                entry.TextureHash[i] = MaskedHash(gpu.VRAMFlat_Texture, sizeof(gpu.VRAMFlat_Texture),
                    entry.TextureRAMStart[i], entry.TextureRAMSize[i]);
        }
        if (entry.TexPalSize)
            entry.TexPalHash = MaskedHash(gpu.VRAMFlat_TexPal, sizeof(gpu.VRAMFlat_TexPal),
                entry.TexPalStart, entry.TexPalSize);

        int oldWidth = width;
        int oldHeight = height;
        unsigned char* imageData = (unsigned char*)DecodingBuffer;
        bool textureReplacementEnabled = true;
        if (textureReplacementEnabled) {
            std::ostringstream oss0;
            oss0 << "3d-";
            for (int i = 0; i < 2; i++)
            {
                if (entry.TextureRAMSize[i])
                    oss0 << static_cast<char32_t>(entry.TextureHash[i]);
            }
            std::string uniqueIdentifier1 = oss0.str();
            std::ostringstream oss;
            for (int i = 0; i < 2; i++)
            {
                if (entry.TextureRAMSize[i])
                    oss << static_cast<char32_t>(XXH3_64bits(&gpu.VRAMFlat_Texture[entry.TextureRAMStart[i]], entry.TextureRAMSize[i]));
            }
            std::string uniqueIdentifier2 = oss.str();
            oss << "-";
            oss << palBase;
            std::string uniqueIdentifier3 = oss.str();
            
            std::string filename1 = uniqueIdentifier1 + ".png";
            std::string filename2 = uniqueIdentifier3 + ".png";
            std::string filename3 = uniqueIdentifier2 + ".png";

            std::string assetsFolder = GamePlugin->assetsFolder();
            std::filesystem::path currentPath = std::filesystem::current_path();
            std::filesystem::path assetsFolderPath = currentPath / "assets" / assetsFolder;
            std::filesystem::path tmpFolderPath = assetsFolderPath / "textures_tmp";
            std::filesystem::path fullPath1 = assetsFolderPath / "textures" / filename1;
            std::filesystem::path fullPath2 = assetsFolderPath / "textures" / filename2;
            std::filesystem::path fullPath3 = assetsFolderPath / "textures" / filename3;
            std::filesystem::path fullPathTmp = tmpFolderPath / filename2;
#ifdef _WIN32
            const char* path1 = fullPath1.string().c_str();
            const char* path2 = fullPath2.string().c_str();
            const char* path3 = fullPath3.string().c_str();
            const char* pathTmp = fullPathTmp.string().c_str();
#else
            const char* path1 = fullPath1.c_str();
            const char* path2 = fullPath2.c_str();
            const char* path3 = fullPath3.c_str();
            const char* pathTmp = fullPathTmp.c_str();
#endif

            if (!std::filesystem::exists(assetsFolderPath)) {
                std::filesystem::create_directory(assetsFolderPath);
            }
            if (!std::filesystem::exists(tmpFolderPath)) {
                std::filesystem::create_directory(tmpFolderPath);
            }

            int channels = 4;
            int r_width, r_height, r_channels;
            imageData = Texreplace::LoadTextureFromFile(path1, &r_width, &r_height, &r_channels);
            if (imageData != nullptr) {
                printf("Loading texture %s\n", path1);
                width = r_width;
                height = r_height;
            }
            else {
                imageData = Texreplace::LoadTextureFromFile(path2, &r_width, &r_height, &r_channels);
                if (imageData != nullptr) {
                    printf("Loading texture %s\n", path2);
                    width = r_width;
                    height = r_height;
                }
                else {
                    imageData = Texreplace::LoadTextureFromFile(path3, &r_width, &r_height, &r_channels);
                    if (imageData != nullptr) {
                        printf("Loading texture %s\n", path3);
                        width = r_width;
                        height = r_height;
                    }
                    else {
                        imageData = (unsigned char*)DecodingBuffer;

                        if (GamePlugin->shouldExportTextures()) {
                            printf("Saving texture %s\n", pathTmp);
                            Texreplace::ExportTextureAsFile(imageData, pathTmp, width, height, channels);
                        }
                    }
                }
            }

            widthLog2 = RightmostBit(width) - 3;
            heightLog2 = RightmostBit(height) - 3;
            entry.WidthLog2 = widthLog2;
            entry.HeightLog2 = heightLog2;
        }

        auto& texArrays = TexArrays[widthLog2][heightLog2];
        auto& freeTextures = FreeTextures[widthLog2][heightLog2];

        if (freeTextures.size() == 0)
        {
            texArrays.resize(texArrays.size()+1);
            TexHandleT& array = texArrays[texArrays.size()-1];

            u32 layers = 1;

            // allocate new array texture
            //printf("allocating new layer set for %d %d %d %d\n", width, height, texArrays.size()-1, array.ImageDescriptor);
            array = TexLoader.GenerateTexture(width, height, layers);

            for (u32 i = 0; i < layers; i++)
            {
                freeTextures.push_back(TexArrayEntry{array, i, width, height});
            }
        }

        TexArrayEntry storagePlace = freeTextures[freeTextures.size()-1];
        freeTextures.pop_back();

        storagePlace.Width = oldWidth;
        storagePlace.Height = oldHeight;
        entry.Texture = storagePlace;

        TexLoader.UploadTexture(storagePlace.TextureID, width, height, storagePlace.Layer, imageData);
        //printf("using storage place %d %d | %d %d (%d)\n", width, height, storagePlace.TexArrayIdx, storagePlace.LayerIdx, array.ImageDescriptor);

        textureHandle = storagePlace.TextureID;
        layer = storagePlace.Layer;
        helper = &Cache.emplace(std::make_pair(key, entry)).first->second.LastVariant;
    }

    void Reset()
    {
        for (u32 i = 0; i < 8; i++)
        {
            for (u32 j = 0; j < 8; j++)
            {
                for (u32 k = 0; k < TexArrays[i][j].size(); k++)
                    TexLoader.DeleteTexture(TexArrays[i][j][k]);
                TexArrays[i][j].clear();
                FreeTextures[i][j].clear();
            }
        }
        Cache.clear();
    }
private:
    struct TexArrayEntry
    {
        TexHandleT TextureID;
        u32 Layer;
        u32 Width;
        u32 Height;
    };

    struct TexCacheEntry
    {
        u32 LastVariant; // very cheap way to make variant lookup faster

        u32 TextureRAMStart[2], TextureRAMSize[2];
        u32 TexPalStart, TexPalSize;
        u8 WidthLog2, HeightLog2;
        TexArrayEntry Texture;

        u64 TextureHash[2];
        u64 TexPalHash;
    };
    std::unordered_map<u64, TexCacheEntry> Cache;

    TexLoaderT TexLoader;

    std::vector<TexArrayEntry> FreeTextures[8][8];
    std::vector<TexHandleT> TexArrays[8][8];

    u32 DecodingBuffer[1024*1024];
};

}

#endif