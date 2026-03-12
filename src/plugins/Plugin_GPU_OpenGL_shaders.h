/*
    Copyright 2016-2024 VitorMM

    This file is part of Melox Mix, which is based on melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef PLUGIN_GPU_OPENGL_SHADERS_H
#define PLUGIN_GPU_OPENGL_SHADERS_H

namespace Plugins
{
// language=GLSL
const char* k2DCompositorFS_Plugin = R"(#version 140

#define SHAPES_DATA_ARRAY_SIZE 32
#define SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE 4

struct ShapeData2D {
    vec2 sourceScale;

    int effects;

    float opacity;

    ivec4 squareInitialCoords;
    vec4 squareFinalCoords;

    vec4 fadeBorderSize;

    vec4 squareCornersModifier;

    ivec4 colorToAlpha;
    ivec4 singleColorToAlpha[SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE];
};

layout(std140) uniform ShapeBlock2D {
    ShapeData2D shapes[SHAPES_DATA_ARRAY_SIZE];
};

uniform float currentAspectRatio;
uniform float forcedAspectRatio;

uniform int hudScale;
uniform bool showOriginalHud;
uniform int screenLayout; // 0 = top screen, 1 = bottom screen, 2 = both vertical, 3 = both horizontal
uniform int brightnessMode; // 0 = default, 1 = top screen, 2 = bottom screen, 3 = horizontal, 4 = no brightness

uniform int shapeCount;
uniform int screenIndex;


uniform sampler2D BGLayerTex[4];
uniform sampler2DArray OBJLayerTex;
uniform sampler2DArray Capture128Tex;
uniform sampler2DArray Capture256Tex;
uniform isampler2D MosaicTex;

struct sBGConfig
{
    ivec2 Size;
    int Type;
    int PalOffset;
    int TileOffset;
    int MapOffset;
    bool Clamp;
};

layout(std140) uniform ubBGConfig
{
    int uVRAMMask;
    sBGConfig uBGConfig[4];
};

struct sScanline
{
    ivec2 BGOffset[4];
    ivec4 BGRotscale[2];
    int BackColor;
    uint WinRegs;
    int WinMask;
    ivec4 WinPos;
    bvec4 BGMosaicEnable;
    ivec4 MosaicSize;
};

layout(std140) uniform ubScanlineConfig
{
    sScanline uScanline[192];
};

layout(std140) uniform ubCompositorConfig
{
    ivec4 uBGPrio;
    bool uEnableOBJ;
    bool uEnable3D;
    int uBlendCnt;
    int uBlendEffect;
    ivec3 uBlendCoef;
};

uniform int uScaleFactor;

smooth in vec4 fTexcoord;
// fTexcoord.x : 0.0 .. 256.0
// fTexcoord.y : 0.0 .. 192.0
// fTexcoord.z : 0.0 .. 256.0 * uScaleFactor
// fTexcoord.w : 0.0 .. 192.0 * uScaleFactor

out vec4 oColor;

int MosaicX = 0;

ivec3 ConvertColor(int col)
{
    ivec3 ret;
    ret.r = (col & 0x1F) << 1;
    ret.g = ((col & 0x3E0) >> 4) | (col >> 15);
    ret.b = (col & 0x7C00) >> 9;
    return ret;
}

vec4 BG0Fetch(vec2 coord)
{
    return texture(BGLayerTex[0], coord);
}

vec4 BG1Fetch(vec2 coord)
{
    return texture(BGLayerTex[1], coord);
}

vec4 BG2Fetch(vec2 coord)
{
    return texture(BGLayerTex[2], coord);
}

vec4 BG3Fetch(vec2 coord)
{
    return texture(BGLayerTex[3], coord);
}

vec4 BG0CalcAndFetch(vec2 coord, int line)
{
    ivec2 bgoffset = uScanline[line].BGOffset[0];
    vec2 bgpos = vec2(bgoffset.xy) + coord;

    if (uScanline[line].BGMosaicEnable[0])
    {
        bgpos = floor(bgpos) - vec2(MosaicX, 0);
    }

    return BG0Fetch(bgpos / vec2(uBGConfig[0].Size));
}

vec4 BG1CalcAndFetch(vec2 coord, int line)
{
    ivec2 bgoffset = uScanline[line].BGOffset[1];
    vec2 bgpos = vec2(bgoffset.xy) + coord;

    if (uScanline[line].BGMosaicEnable[1])
    {
        bgpos = floor(bgpos) - vec2(MosaicX, 0);
    }

    return BG1Fetch(bgpos / vec2(uBGConfig[1].Size));
}

vec4 BG2CalcAndFetch(vec2 coord, int line)
{
    ivec2 bgoffset = uScanline[line].BGOffset[2];
    vec2 bgpos;
    if (uBGConfig[2].Type >= 2)
    {
        // rotscale BG
        bgpos = vec2(bgoffset.xy) / 256;
        vec4 rotscale = vec4(uScanline[line].BGRotscale[0]) / 256;
        mat2 rsmatrix = mat2(rotscale.xy, rotscale.zw);
        bgpos = bgpos + (coord * rsmatrix);
    }
    else
    {
        // text-mode BG
        bgpos = vec2(bgoffset.xy) + coord;
    }

    if (uScanline[line].BGMosaicEnable[2])
    {
        bgpos = floor(bgpos) - vec2(MosaicX, 0);
    }

    if (uBGConfig[2].Type >= 7)
    {
        // hi-res capture
        bgpos.y += uBGConfig[2].MapOffset;
        vec3 capcoord = vec3(bgpos / vec2(uBGConfig[2].Size), uBGConfig[2].TileOffset);

        // due to the possible weirdness of display capture buffers,
        // we need to do custom wraparound handling
        if (uBGConfig[2].Clamp)
        {
            if (any(lessThan(capcoord.xy, vec2(0))) || any(greaterThanEqual(capcoord.xy, vec2(1))))
                return vec4(0);
        }

        if (uBGConfig[2].Type == 7)
            return texture(Capture128Tex, capcoord);
        else
            return texture(Capture256Tex, capcoord);
    }

    return BG2Fetch(bgpos / vec2(uBGConfig[2].Size));
}

vec4 BG3CalcAndFetch(vec2 coord, int line)
{
    ivec2 bgoffset = uScanline[line].BGOffset[3];
    vec2 bgpos;
    if (uBGConfig[3].Type >= 2)
    {
        // rotscale BG
        bgpos = vec2(bgoffset.xy) / 256;
        vec4 rotscale = vec4(uScanline[line].BGRotscale[1]) / 256;
        mat2 rsmatrix = mat2(rotscale.xy, rotscale.zw);
        bgpos = bgpos + (coord * rsmatrix);
    }
    else
    {
        // text-mode BG
        bgpos = vec2(bgoffset.xy) + coord;
    }

    if (uScanline[line].BGMosaicEnable[3])
    {
        bgpos = floor(bgpos) - vec2(MosaicX, 0);
    }

    if (uBGConfig[3].Type >= 7)
    {
        // hi-res capture
        bgpos.y += uBGConfig[3].MapOffset;
        vec3 capcoord = vec3(bgpos / vec2(uBGConfig[3].Size), uBGConfig[3].TileOffset);

        // due to the possible weirdness of display capture buffers,
        // we need to do custom wraparound handling
        if (uBGConfig[3].Clamp)
        {
            if (any(lessThan(capcoord.xy, vec2(0))) || any(greaterThanEqual(capcoord.xy, vec2(1))))
                return vec4(0);
        }

        if (uBGConfig[3].Type == 7)
            return texture(Capture128Tex, capcoord);
        else
            return texture(Capture256Tex, capcoord);
    }

    return BG3Fetch(bgpos / vec2(uBGConfig[3].Size));
}

void CalcSpriteMosaic(in ivec2 coord, out ivec4 objflags, out vec4 objcolor)
{
    for (int i = 0; i < 16; i++)
    {
        ivec2 curpos = ivec2(coord.x - 15 + i, coord.y);

        if (curpos.x < 0)
        {
            objflags = ivec4(0);
            objcolor = vec4(0);
        }
        else
        {
            int mosx = texelFetch(MosaicTex, ivec2(curpos.x, uScanline[curpos.y].MosaicSize.z), 0).r;
            vec4 color = texelFetch(OBJLayerTex, ivec3(curpos * uScaleFactor, 0), 0);
            ivec4 flags = ivec4(texelFetch(OBJLayerTex, ivec3(curpos * uScaleFactor, 1), 0) * 255.0);

            bool latch = false;
            if (mosx == 0)
                latch = true;
            else if (flags.g == 0)
                latch = true;
            else if (objflags.g == 0)
                latch = true;
            else if (flags.a < objflags.a)
                latch = true;

            if (latch)
            {
                objflags = flags;
                objcolor = color;
            }
        }
    }
}

bool isValidConsideringCropSquareCorners(vec2 finalPos, vec4 cropSquareCorners, ivec2 squareInitialSize)
{
    return (finalPos.x + finalPos.y >= cropSquareCorners[0]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + finalPos.y >= cropSquareCorners[1]) &&
           (finalPos.x + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[2]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[3]);
}

bool isInsideRoundedCorner(vec2 pos, vec2 center, float radius)
{
    vec2 d = pos - center;
    return dot(d, d) < radius * radius;
}

bool isValidConsideringSquareBorderRadius(vec2 finalPos, vec4 radius, ivec2 squareInitialSize) {
    float squareWidth = squareInitialSize[0];
    float squareHeight = squareInitialSize[1];

    // Top-left corner
    if (finalPos.x < radius[0] && finalPos.y < radius[0]) {
        return isInsideRoundedCorner(finalPos, vec2(radius[0], radius[0]), radius[0]);
    }

    // Top-right corner
    if (finalPos.x > squareWidth - radius[1] && finalPos.y < radius[1]) {
        return isInsideRoundedCorner(finalPos, vec2(squareWidth - radius[1], radius[1]), radius[1]);
    }

    // Bottom-left corner
    if (finalPos.x < radius[2] && finalPos.y > squareHeight - radius[2]) {
        return isInsideRoundedCorner(finalPos, vec2(radius[2], squareHeight - radius[2]), radius[2]);
    }

    // Bottom-right corner
    if (finalPos.x > squareWidth - radius[3] && finalPos.y > squareHeight - radius[3]) {
        return isInsideRoundedCorner(finalPos, vec2(squareWidth - radius[3], squareHeight - radius[3]), radius[3]);
    }

    return true;
}

vec4 applyBrightnessToColor(vec4 col, int brightmode, int evy)
{
    if (evy > 16) evy = 16;
    float f = float(evy) / 16.0;
    if (brightmode == 1) // up
        col.rgb = col.rgb + (1.0 - col.rgb) * f;
    else if (brightmode == 2) // down
        col.rgb = col.rgb * (1.0 - f);
    return col;
}

vec4 fetch2DLayer(vec2 srcPos)
{
    vec4 bg3 = texture(BGLayerTex[3], srcPos / vec2(uBGConfig[3].Size));
    vec4 bg1 = texture(BGLayerTex[1], srcPos / vec2(uBGConfig[1].Size));
    float a1 = bg1.a;
    return vec4(mix(bg3.rgb, bg1.rgb, a1), max(bg3.a, a1));
}

vec4 applyShapes(vec4 base)
{
    float uiTexScale = 6.0 / ((float(hudScale) - 4.0) / 2.0 + 4.0);
    vec2 texPos = vec2(fTexcoord.x, fTexcoord.y) * uiTexScale;

    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    for (int si = 0; si < shapeCount; si++)
    {
        vec4 finalCoords = shapes[si].squareFinalCoords;
        int  effects = shapes[si].effects;
        bool shouldRotate = ((effects & 0x200) != 0) || ((effects & 0x400) != 0);
        bool isRepeatBG = (effects & 0x40) != 0;

        if (!isRepeatBG && !(all(greaterThanEqual(texPos, finalCoords.xy)) && all(lessThanEqual(texPos, finalCoords.zw))))
            continue;

        vec2 finalPos = (fixStretch / shapes[si].sourceScale) * (texPos - finalCoords.xy);

        // repeat as background
        if ((effects & 0x40) != 0 || (effects & 0x80) != 0)
        {
            vec2 limits = (fixStretch / shapes[si].sourceScale) *
                          (finalCoords.zw - finalCoords.xy);
            if ((effects & 0x40) != 0) finalPos.x = mod(finalPos.x, limits.x);
            if ((effects & 0x80) != 0) finalPos.y = mod(finalPos.y, limits.y);
        }

        bool validArea = true;

        // crop corner as triangle
        if ((effects & 0x2) != 0)
        {
            ivec2 csz = shouldRotate ? shapes[si].squareInitialCoords.wz
                                     : shapes[si].squareInitialCoords.zw;
            validArea = isValidConsideringCropSquareCorners(
                finalPos, shapes[si].squareCornersModifier, csz);
        }

        // rounded corners
        if ((effects & 0x4) != 0)
        {
            ivec2 csz = shouldRotate ? shapes[si].squareInitialCoords.wz
                                     : shapes[si].squareInitialCoords.zw;
            validArea = isValidConsideringSquareBorderRadius(
                finalPos, shapes[si].squareCornersModifier, csz);
        }

        if (!validArea) continue;

        // mirror X / Y
        if ((effects & 0x8)  != 0) finalPos.x = float(shapes[si].squareInitialCoords.z) - finalPos.x;
        if ((effects & 0x10) != 0) finalPos.y = float(shapes[si].squareInitialCoords.w) - finalPos.y;

        // rotate to the left
        if ((effects & 0x200) != 0)
        {
            float nx = float(shapes[si].squareInitialCoords.z) - finalPos.y;
            finalPos  = vec2(nx, finalPos.x);
        }
        // rotate to the right
        else if ((effects & 0x400) != 0)
        {
            float ny = float(shapes[si].squareInitialCoords.w) - finalPos.x;
            finalPos  = vec2(finalPos.y, ny);
        }

        vec2 srcPos = vec2(shapes[si].squareInitialCoords.xy) + finalPos;

        vec4 color = fetch2DLayer(srcPos);
        if (color.a <= 0.0) continue; // invisible pixel; ignore it

        // single color to alpha
        bool shouldSkip = false;
        for (int ci = 0; ci < SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE; ci++)
        {
            ivec4 sc = shapes[si].singleColorToAlpha[ci];
            if (sc.a > 0)
            {
                vec3 refCol = vec3(sc.rgb) / 255.0;
                if (all(lessThan(abs(color.rgb - refCol), vec3(0.02))))
                {
                    shouldSkip = true;
                    break;
                }
            }
        }
        if (shouldSkip) continue;

        // invert gray scale colors
        if ((effects & 0x1) != 0)
        {
            bool isGray = (abs(color.r - color.g) < 0.02) &&
                          (abs(color.r - color.b) < 0.02) &&
                          (abs(color.g - color.b) < 0.02);
            if (isGray) color.rgb = 1.0 - color.rgb;
        }

        // fade borders / opacity
        float blendFactor = color.a;
        if ((effects & 0x20) != 0)
        {
            vec4  fbs     = shapes[si].fadeBorderSize;
            float opacity = shapes[si].opacity;

            if (any(greaterThan(fbs, vec4(0.0))) || opacity < 1.0)
            {
                float ld = texPos.x - finalCoords[0];
                float rd = finalCoords[2] - texPos.x;
                float td = texPos.y - finalCoords[1];
                float bd = finalCoords[3] - texPos.y;

                float lf = fbs[0] == 0.0 ? 1.0 : clamp(ld / fbs[0], 0.0, 1.0);
                float tf = fbs[1] == 0.0 ? 1.0 : clamp(td / fbs[1], 0.0, 1.0);
                float rf = fbs[2] == 0.0 ? 1.0 : clamp(rd / fbs[2], 0.0, 1.0);
                float bf = fbs[3] == 0.0 ? 1.0 : clamp(bd / fbs[3], 0.0, 1.0);

                blendFactor = min(lf, rf) * min(tf, bf) * opacity * color.a;
            }

            // color to alpha
            ivec4 cta = shapes[si].colorToAlpha;
            if (cta.a == 1)
            {
                vec3  refCol = vec3(cta.rgb) / 255.0;
                float dist   = (abs(refCol.r - color.r) +
                                abs(refCol.g - color.g) +
                                abs(refCol.b - color.b)) / 3.0;
                blendFactor = clamp(dist * 2.0, 0.0, 1.0);
            }
        }

        base = mix(base, vec4(color.rgb, 1.0), clamp(blendFactor, 0.0, 1.0));
        return base;
    }

    return base;
}

vec4 CompositeLayers()
{
    ivec2 coord = ivec2(fTexcoord.zw);
    vec2 bgcoord = vec2(fTexcoord.x, fract(fTexcoord.y));
    int xpos = int(fTexcoord.x);
    int line = int(fTexcoord.y);

    if (uScanline[line].MosaicSize.x > 0)
        MosaicX = texelFetch(MosaicTex, ivec2(bgcoord.x, uScanline[line].MosaicSize.x), 0).r;

    ivec4 col1 = ivec4(ConvertColor(uScanline[line].BackColor), 0x20);
    int mask1 = 0x20;
    ivec4 col2 = ivec4(0);
    int mask2 = 0;
    bool specialcase = false;
    bool useShapes = (screenIndex == 1 && shapeCount != 0);

    vec4 layercol[6];
    layercol[0] = BG0CalcAndFetch(bgcoord, line);
    layercol[1] = useShapes ? vec4(0.0) : BG1CalcAndFetch(bgcoord, line);
    layercol[2] = BG2CalcAndFetch(bgcoord, line);
    layercol[3] = useShapes ? vec4(0.0) : BG3CalcAndFetch(bgcoord, line);

    ivec4 objflags;
    if (uScanline[line].MosaicSize.z > 0)
    {
        CalcSpriteMosaic(ivec2(fTexcoord.xy), objflags, layercol[4]);
    }
    else
    {
        layercol[4] = texelFetch(OBJLayerTex, ivec3(coord, 0), 0);
        layercol[5] = texelFetch(OBJLayerTex, ivec3(coord, 1), 0);
        objflags = ivec4(layercol[5] * 255.0);
    }

    int winmask = uScanline[line].WinMask;
    bool inside_win0, inside_win1;

    if (xpos < uScanline[line].WinPos[0])
        inside_win0 = ((winmask & (1<<0)) != 0);
    else if (xpos < uScanline[line].WinPos[1])
        inside_win0 = ((winmask & (1<<1)) != 0);
    else
        inside_win0 = ((winmask & (1<<2)) != 0);

    if (xpos < uScanline[line].WinPos[2])
        inside_win1 = ((winmask & (1<<3)) != 0);
    else if (xpos < uScanline[line].WinPos[3])
        inside_win1 = ((winmask & (1<<4)) != 0);
    else
        inside_win1 = ((winmask & (1<<5)) != 0);

    uint winregs = uScanline[line].WinRegs;
    uint winsel = winregs;
    if (objflags.b > 0)
        winsel = winregs >> 8;
    if (inside_win1)
        winsel = winregs >> 16;
    if (inside_win0)
        winsel = winregs >> 24;

    for (int prio = 3; prio >= 0; prio--)
    {
        for (int bg = 3; bg >= 0; bg--)
        {
            if ((uBGPrio[bg] == prio) && (layercol[bg].a > 0) && ((winsel & (1u << bg)) != 0u))
            {
                col2 = col1;
                mask2 = mask1 << 8;
                col1 = ivec4(layercol[bg] * 255.0) >> ivec4(2,2,2,3);
                mask1 = (1 << bg);
                specialcase = (bg == 0) && uEnable3D;
            }
        }

        if (uEnableOBJ && (objflags.a == prio) && (layercol[4].a > 0) && ((winsel & (1u << 4)) != 0u))
        {
            col2 = col1;
            mask2 = mask1 << 8;
            col1 = ivec4(layercol[4] * 255.0) >> ivec4(2,2,2,3);
            mask1 = (1 << 4);
            specialcase = (objflags.r != 0);
        }
    }

    int effect = 0;
    int eva, evb, evy = uBlendCoef[2];

    if (specialcase && (uBlendCnt & mask2) != 0)
    {
        if (mask1 == (1<<0))
        {
            // 3D layer blending
            effect = 4;
            eva = (col1.a & 0x1F) + 1;
            evb = 32 - eva;
        }
        else if (objflags.r == 1)
        {
            // semi-transparent sprite
            effect = 1;
            eva = uBlendCoef[0];
            evb = uBlendCoef[1];
        }
        else //if (objflags.r == 2)
        {
            // bitmap sprite
            effect = 1;
            eva = col1.a;
            evb = 16 - eva;
        }
    }
    else if (((uBlendCnt & mask1) != 0) && ((winsel & (1u << 5)) != 0u))
    {
        effect = uBlendEffect;
        if (effect == 1)
        {
            if ((uBlendCnt & mask2) != 0)
            {
                eva = uBlendCoef[0];
                evb = uBlendCoef[1];
            }
            else
                effect = 0;
        }
    }

    if (effect == 1)
    {
        // blending
        col1 = ((col1 * eva) + (col2 * evb) + 0x8) >> 4;
        col1 = min(col1, 0x3F);
    }
    else if (effect == 2)
    {
        // brightness up
        col1 = col1 + ((((0x3F - col1) * evy) + 0x8) >> 4);
    }
    else if (effect == 3)
    {
        // brightness down
        col1 = col1 - (((col1 * evy) + 0x7) >> 4);
    }
    else if (effect == 4)
    {
        // 3D layer blending
        col1 = ((col1 * eva) + (col2 * evb) + 0x10) >> 5;
    }

    return vec4(vec3(col1.rgb << 2) / 255.0, 1);
}

void main()
{
    vec4 col = CompositeLayers();
    if (screenIndex != 1 || shapeCount == 0)
    {
        oColor = col;
        return;
    }

    col = applyShapes(col);

    int brightmode = 0;
    int evy = 0;

    if (brightnessMode == 4) // forced black screen
    {
        brightmode = 2;
        evy = 16;
    }
    else if (brightnessMode != 5 && brightnessMode != 0)
    {
        brightmode = (uBlendEffect == 2) ? 1 :
                     (uBlendEffect == 3) ? 2 : 0;
        evy = uBlendCoef[2];
    }

    col = applyBrightnessToColor(col, brightmode, evy);

    oColor = col;
}
)";

// language=GLSL
const char* kFinalPassFS_Plugin = R"(#version 140

#define SHAPES_DATA_ARRAY_SIZE 32
#define SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE 4

struct ShapeData2D {
    vec2 sourceScale;

    int effects;

    float opacity;

    ivec4 squareInitialCoords;
    vec4 squareFinalCoords;

    vec4 fadeBorderSize;

    vec4 squareCornersModifier;

    ivec4 colorToAlpha;
    ivec4 singleColorToAlpha[SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE];
};

layout(std140) uniform ShapeBlock2D {
    ShapeData2D shapes[SHAPES_DATA_ARRAY_SIZE];
};

uniform float currentAspectRatio;
uniform float forcedAspectRatio;

uniform int hudScale;
uniform bool showOriginalHud;
uniform int screenLayout; // 0 = top screen, 1 = bottom screen, 2 = both vertical, 3 = both horizontal
uniform int brightnessMode; // 0 = default, 1 = top screen, 2 = bottom screen, 3 = horizontal, 4 = no brightness

uniform int shapeCount;


uniform sampler2D MainInputTexA;
uniform sampler2D MainInputTexB;
uniform sampler2DArray AuxInputTex;

layout(std140) uniform ubFinalPassConfig
{
    bvec4 uScreenSwap[48]; // one bool per scanline
    int uScaleFactor;
    int uAuxLayer;
    int uDispModeA;
    int uDispModeB;
    int uBrightModeA;
    int uBrightModeB;
    int uBrightFactorA;
    int uBrightFactorB;
    float uAuxColorFactor;
};

smooth in vec3 fTexcoord;

out vec4 oTopColor;
out vec4 oBottomColor;

bool isValidConsideringCropSquareCorners(vec2 finalPos, vec4 cropSquareCorners, ivec2 squareInitialSize) {
    return (finalPos.x + finalPos.y >= cropSquareCorners[0]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + finalPos.y >= cropSquareCorners[1]) &&
           (finalPos.x + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[2]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[3]);
}

bool isInsideRoundedCorner(vec2 pos, vec2 center, float radius) {
    return (pos.x - center.x) * (pos.x - center.x) +
           (pos.y - center.y) * (pos.y - center.y) < radius * radius;
}

bool isValidConsideringSquareBorderRadius(vec2 finalPos, vec4 radius, ivec2 squareInitialSize) {
    bool validArea = true;
    float squareWidth = squareInitialSize[0];
    float squareHeight = squareInitialSize[1];

    // Top-left corner
    if (finalPos.x < radius[0] && finalPos.y < radius[0]) {
        validArea = isInsideRoundedCorner(finalPos, vec2(radius[0], radius[0]), radius[0]);
    }

    // Top-right corner
    else if (finalPos.x > squareWidth - radius[1] && finalPos.y < radius[1]) {
        validArea = isInsideRoundedCorner(finalPos, vec2(squareWidth - radius[1], radius[1]), radius[1]);
    }

    // Bottom-left corner
    else if (finalPos.x < radius[2] && finalPos.y > squareHeight - radius[2]) {
        validArea = isInsideRoundedCorner(finalPos, vec2(radius[2], squareHeight - radius[2]), radius[2]);
    }

    // Bottom-right corner
    else if (finalPos.x > squareWidth - radius[3] && finalPos.y > squareHeight - radius[3]) {
        validArea = isInsideRoundedCorner(finalPos, vec2(squareWidth - radius[3], squareHeight - radius[3]), radius[3]);
    }

    return validArea;
}

vec3 getHorizontalDualScreen2DTextureCoordinates(float xpos, float ypos)
{
    int screenScale = 2;
    vec2 texPosition3d = vec2(xpos*screenScale, ypos*screenScale);
    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(1.0, heightScale);

    // screen 1
    {
        int sourceScreenHeight = 192;
        int sourceScreenWidth = 256;
        int sourceScreenTopMargin = 0;
        int sourceScreenLeftMargin = 0;
        int screenHeight = int(sourceScreenHeight*widthScale);
        int screenWidth = sourceScreenWidth;
        int screenTopMargin = (192*screenScale - screenHeight)/2;
        int screenLeftMargin = 0;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            vec2 result = fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
            return vec3(result.x, result.y, 1);
        }
    }

    // screen 2
    {
        int sourceScreenHeight = 192;
        int sourceScreenWidth = 256;
        int sourceScreenTopMargin = 0;
        int sourceScreenLeftMargin = 0;
        int screenHeight = int(sourceScreenHeight*widthScale);
        int screenWidth = sourceScreenWidth;
        int screenTopMargin = (192*screenScale - screenHeight)/2;
        int screenLeftMargin = 256;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            vec2 result = fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
            return vec3(result.x, result.y, 2);
        }
    }


    // nothing (clear screen)
    return vec3(0, 0, 0);
}

vec3 getVerticalDualScreen2DTextureCoordinates(float xpos, float ypos)
{
    int screenScale = 2;
    vec2 texPosition3d = vec2(xpos*screenScale, ypos*screenScale);
    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    // screen 1
    {
        int sourceScreenHeight = 192;
        int sourceScreenWidth = 256;
        int sourceScreenTopMargin = 0;
        int sourceScreenLeftMargin = 0;
        int screenHeight = sourceScreenHeight;
        int screenWidth = int(sourceScreenWidth*heightScale);
        int screenTopMargin = 0;
        int screenLeftMargin = (256*screenScale - screenWidth)/2;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            vec2 result = fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
            return vec3(result.x, result.y, 1);
        }
    }

    // screen 2
    {
        int sourceScreenHeight = 192;
        int sourceScreenWidth = 256;
        int sourceScreenTopMargin = 0;
        int sourceScreenLeftMargin = 0;
        int screenHeight = sourceScreenHeight;
        int screenWidth = int(sourceScreenWidth*heightScale);
        int screenTopMargin = 192;
        int screenLeftMargin = (256*screenScale - screenWidth)/2;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            vec2 result = fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
            return vec3(result.x, result.y, 2);
        }
    }

    // nothing (clear screen)
    return vec3(0, 0, 0);
}

ivec4 getRegularScreenColor(vec2 textureBeginning, bool isBottomScreen) {
    if (isBottomScreen) {
        return ivec4(texture(MainInputTexB, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
    }
    return ivec4(texture(MainInputTexA, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
}

// RGB; all values from 0 to 255
ivec4 getTopScreenColor(vec2 pos)
{
    float xpos = pos.x * 256.0;
    float ypos = pos.y * 192.0;
    if (screenLayout == 2) { // vertical
        vec3 textureBeginning = getVerticalDualScreen2DTextureCoordinates(xpos, ypos);
        if (textureBeginning.z == 0) {
            return ivec4(0, 0, 0, 0);
        }

        return getRegularScreenColor(textureBeginning.xy, textureBeginning.z == 2);
    }

    if (screenLayout == 3) { // horizontal
        vec3 textureBeginning = getHorizontalDualScreen2DTextureCoordinates(xpos, ypos);
        if (textureBeginning.z == 0) {
            return ivec4(0, 0, 0, 0);
        }

        return getRegularScreenColor(textureBeginning.xy, textureBeginning.z == 2);
    }

    if (showOriginalHud) {
        vec2 textureBeginning = vec2(xpos, ypos);
        if (currentAspectRatio != forcedAspectRatio) {
            float heightScale = (1.0/forcedAspectRatio)/currentAspectRatio;

            float sourceScreenWidth = 256.0;
            float screenLeftMargin = (sourceScreenWidth - sourceScreenWidth/heightScale)/2;
            float screenFinalWidth = fTexcoord.x/heightScale; // TODO: KH fTexcoord shouldn't be used here

            textureBeginning.x = int(screenLeftMargin + screenFinalWidth);
            if (textureBeginning.x < 0 || textureBeginning.x > screenFinalWidth) {
                return ivec4(0, 0, 0, 0);
            }
        }

        return getRegularScreenColor(textureBeginning.xy, screenLayout == 1);
    }

    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    float uiTexScale = (6.0/((float(hudScale) - 4) / 2 + 4));
    vec2 texPosition3d = vec2(xpos, ypos)*uiTexScale;

    for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
        vec4 squareFinalCoords = shapes[shapeIndex].squareFinalCoords;

        if ((all(greaterThanEqual(texPosition3d, squareFinalCoords.xy)) &&
                all(lessThanEqual(texPosition3d, squareFinalCoords.zw))) || ((shapes[shapeIndex].effects & 0x40) != 0)) {
            int effects = shapes[shapeIndex].effects;
            bool shouldRotate = ((effects & 0x200) != 0) || ((effects & 0x400) != 0);

            vec2 finalPos = (fixStretch/shapes[shapeIndex].sourceScale)*(texPosition3d - squareFinalCoords.xy);
            bool validArea = true;

            // repeat as background
            if ((effects & 0x40) != 0 || (effects & 0x80) != 0) {
                finalPos = (fixStretch/shapes[shapeIndex].sourceScale)*(texPosition3d - squareFinalCoords.xy);
                vec2 limits = (fixStretch/shapes[shapeIndex].sourceScale)*(squareFinalCoords.zw - squareFinalCoords.xy);
                if ((effects & 0x40) != 0) {
                    while (finalPos.x >= limits.x) {
                        finalPos.x -= limits.x;
                    }
                }
                if ((effects & 0x80) != 0) {
                    while (finalPos.y >= limits.y) {
                        finalPos.y -= limits.y;
                    }
                }
            }

            // crop corner as triangle
            if ((effects & 0x2) != 0) {
                ivec2 cropAreaSize = shapes[shapeIndex].squareInitialCoords.zw;
                if (shouldRotate) {
                    cropAreaSize = shapes[shapeIndex].squareInitialCoords.wz;
                }
                validArea = isValidConsideringCropSquareCorners(finalPos, shapes[shapeIndex].squareCornersModifier, cropAreaSize);
            }
            // rounded corners
            if ((effects & 0x4) != 0) {
                ivec2 cropAreaSize = shapes[shapeIndex].squareInitialCoords.zw;
                if (shouldRotate) {
                    cropAreaSize = shapes[shapeIndex].squareInitialCoords.wz;
                }
                validArea = isValidConsideringSquareBorderRadius(finalPos, shapes[shapeIndex].squareCornersModifier, cropAreaSize);
            }

            if (validArea) {
                // mirror X
                if ((effects & 0x8) != 0) {
                    finalPos.x = shapes[shapeIndex].squareInitialCoords.z - finalPos.x;
                }
                // mirror Y
                if ((effects & 0x10) != 0) {
                    finalPos.y = shapes[shapeIndex].squareInitialCoords.w - finalPos.y;
                }
                if (shouldRotate) {
                    // rotate to the left
                    if ((effects & 0x200) != 0) {
                        float newFinalPosX = shapes[shapeIndex].squareInitialCoords.z - finalPos.y;
                        float newFinalPosY = finalPos.x;
                        finalPos.x = newFinalPosX;
                        finalPos.y = newFinalPosY;
                    }
                    // rotate to the right
                    if ((effects & 0x400) != 0) {
                        float newFinalPosX = finalPos.y;
                        float newFinalPosY = shapes[shapeIndex].squareInitialCoords.w - finalPos.x;
                        finalPos.x = newFinalPosX;
                        finalPos.y = newFinalPosY;
                    }
                }

                vec2 textureBeginning = finalPos + vec2(shapes[shapeIndex].squareInitialCoords.xy);
                bool isBottomScreen = textureBeginning.y >= 192.0;
                if (isBottomScreen) {
                    textureBeginning.y = textureBeginning.y - 192.0;
                }

                // single color to alpha
                bool shouldSkipColor = false;
                for (int colorIndex = 0; colorIndex < SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE; colorIndex++) {
                    ivec4 singleColorToAlpha = shapes[shapeIndex].singleColorToAlpha[colorIndex];
                    if (singleColorToAlpha.a > 0)
                    {
                        ivec4 colorZero = getRegularScreenColor(textureBeginning.xy, isBottomScreen);
                        if (colorZero.r == singleColorToAlpha.r &&
                            colorZero.g == singleColorToAlpha.g &&
                            colorZero.b == singleColorToAlpha.b) {
                            shouldSkipColor = true;
                            break;
                        }
                    }
                }
                if (shouldSkipColor) {
                    continue;
                }

                ivec4 color = getRegularScreenColor(textureBeginning.xy, isBottomScreen);

                // invert gray scale colors
                if ((effects & 0x1) != 0) {
                    bool isShadeOfGray = (abs(color.r - color.g) < 20) && (abs(color.r - color.b) < 20) && (abs(color.g - color.b) < 20);
                    if (isShadeOfGray) {
                        color = ivec4(255 - color.r, 255 - color.g, 255 - color.b, color.a);
                    }
                }

                // TODO: KH brightnessMode_Auto
                /*if (brightnessMode == 6) { // brightnessMode_Auto
                    color = applyBrightness(color, ivec4(texelFetch(ScreenTex, ivec2(256*3, int(textureBeginning.y)), 0)));
                }*/

                // manipulate transparency
                if ((effects & 0x20) != 0) {
                    // TODO: KH manipulate transparency
                    // I will need to reimplement that, merging colors based on transparency
                    return color;

                    /*
                    ivec2 coordinates = textureBeginning + ivec2(512,0);
                    ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));

                    vec4 fadeBorderSize = shapes[shapeIndex].fadeBorderSize;
                    float opacity = shapes[shapeIndex].opacity;
                    if (any(greaterThan(fadeBorderSize, vec4(0))) || opacity < 1.0)
                    {
                        float leftDiff = texPosition3d.x - squareFinalCoords[0];
                        float rightDiff = squareFinalCoords[2] - texPosition3d.x;
                        float topDiff = texPosition3d.y - squareFinalCoords[1];
                        float bottomDiff = squareFinalCoords[3] - texPosition3d.y;

                        float leftBlurFactor   = fadeBorderSize[0] == 0 ? 1.0 : clamp(leftDiff   / (fadeBorderSize[0] * heightScale), 0.0, 1.0);
                        float topBlurFactor    = fadeBorderSize[1] == 0 ? 1.0 : clamp(topDiff    /  fadeBorderSize[1], 0.0, 1.0);
                        float rightBlurFactor  = fadeBorderSize[2] == 0 ? 1.0 : clamp(rightDiff  / (fadeBorderSize[2] * heightScale), 0.0, 1.0);
                        float bottomBlurFactor = fadeBorderSize[3] == 0 ? 1.0 : clamp(bottomDiff /  fadeBorderSize[3], 0.0, 1.0);

                        float xBlur = min(leftBlurFactor, rightBlurFactor);
                        float yBlur = min(topBlurFactor, bottomBlurFactor);
                        int visibilityOf2D = (shapes[shapeIndex].squareInitialCoords.y >= 192 || color.a > 0x4) ? 63 : (color.a == 0x4 ? 0 : (color.g << 2 - 1));
                        float visibilityOf2DFactor = xBlur * yBlur;

                        int blurVal = int(visibilityOf2DFactor * visibilityOf2D * opacity);
                        color = ivec4(color.r, blurVal // 2D visibility //, 63 - blurVal // 3D visibility //, 0x01);

                        // TODO: The fade does not work properly if you need this shape to blend with another shape
                    }

                    ivec4 colorToAlpha = shapes[shapeIndex].colorToAlpha;
                    if (colorToAlpha.a == 1)
                    {
                        ivec4 colorZero = ivec4(texelFetch(ScreenTex, textureBeginning, 0));
                        int blur = ((abs(colorToAlpha.r - colorZero.r) +
                                     abs(colorToAlpha.g - colorZero.g) +
                                     abs(colorToAlpha.b - colorZero.b))*2)/3;
                        color = ivec4(color.r, blur, 64 - blur, 0x01);
                    }
                    */
                }

                return color;
            }
        }
    }

    return ivec4(texture(MainInputTexA, fTexcoord.xy, 0) * 255.0);
}

ivec3 MasterBrightness(ivec3 color, int brightmode, int evy)
{
    if (brightmode == 1)
    {
        // up
        color += (((0x3F - color) * evy) >> 4);
    }
    else if (brightmode == 2)
    {
        // down
        color -= (((color * evy) + 0xF) >> 4);
    }

    return color;
}

void main()
{
    // ivec4 col_main = ivec4(texture(MainInputTexA, fTexcoord.xy, 0) * 255.0) >> 2;
    ivec4 col_main = getTopScreenColor(fTexcoord.xy) >> 2;
    ivec4 col_sub = ivec4(texture(MainInputTexB, fTexcoord.xy, 0) * 255.0) >> 2;

    ivec3 output_main, output_sub;

    if (uDispModeA == 0)
    {
        // screen disabled (white)
        output_main = ivec3(63, 63, 63);
    }
    else if (uDispModeA == 1)
    {
        // BG/OBJ layers
        output_main = col_main.rgb;
    }
    else
    {
        // VRAM display / mainmem FIFO
        output_main = ivec3(texture(AuxInputTex, vec3(fTexcoord.xz, uAuxLayer)).rgb * uAuxColorFactor);
    }

    if (uDispModeB == 0)
    {
        // screen disabled (white)
        output_sub = ivec3(63, 63, 63);
    }
    else
    {
        // BG/OBJ layers
        output_sub = col_sub.rgb;
    }

    if (uDispModeA != 0)
        output_main = MasterBrightness(output_main, uBrightModeA, uBrightFactorA);
    if (uDispModeB != 0)
        output_sub = MasterBrightness(output_sub, uBrightModeB, uBrightFactorB);

    output_main = (output_main << 2) | (output_main >> 6);
    output_sub = (output_sub << 2) | (output_sub >> 6);

    int line = int(fTexcoord.y * 192);
    bool swapbit = uScreenSwap[line>>2][line&0x3];

    if (!swapbit)
    {
        oTopColor = vec4(vec3(output_sub) / 255.0, 1.0);
        oBottomColor = vec4(vec3(output_main) / 255.0, 1.0);
    }
    else
    {
        oTopColor = vec4(vec3(output_main) / 255.0, 1.0);
        oBottomColor = vec4(vec3(output_sub) / 255.0, 1.0);
    }
}
)";

}

#endif // PLUGIN_GPU_OPENGL_SHADERS_H
