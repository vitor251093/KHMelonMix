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
const char* kCompositorFS_Plugin = R"(#version 140

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


uniform uint u3DScale;
uniform usampler2D ScreenTex;
uniform sampler2D _3DTex;

smooth in vec2 fTexcoord;

out vec4 oColor;

ivec4 combineLayers(ivec4 _3dpix, ivec4 val1, ivec4 val2, ivec4 val3)
{
    int compmode = val3.a & 0xF;
    int eva, evb, evy;

    if (compmode == 4)
    {
        // 3D on top, blending

        if (_3dpix.a > 0)
        {
            eva = (_3dpix.a & 0x1F) + 1;
            evb = 32 - eva;

            val1 = ((_3dpix * eva) + (val1 * evb) + 0x10) >> 5;
            val1 = min(val1, 0x3F);
        }
        else
            val1 = val2;
    }
    else if (compmode == 1)
    {
        // 3D on bottom, blending

        if (_3dpix.a > 0)
        {
            eva = val3.g;
            evb = val3.b;

            val1 = ((val1 * eva) + (_3dpix * evb) + (0x8 << 2)) >> 6;
            val1 = min(val1, 0x3F);
        }
        else
            val1 = val2;
    }
    else if (compmode <= 3)
    {
        // 3D on top, normal/fade

        if (_3dpix.a > 0)
        {
            evy = val3.g >> 2;

            val1 = _3dpix;
            if      (compmode == 2) val1 += (((0x3F - val1) * evy) + 0x8) >> 4;
            else if (compmode == 3) val1 -= ((val1 * evy) + 0x7) >> 4;
        }
        else
            val1 = val2;
    }

    return val1;
}

ivec4 applyBrightness(ivec4 pixel, ivec4 mbright)
{
    int brightmode = mbright.g >> 6;
    if (brightmode == 1)
    {
        // up
        int evy = mbright.r & 0x1F;
        if (evy > 16) evy = 16;

        pixel += ((0x3F - pixel) * evy) >> 4;
    }
    else if (brightmode == 2)
    {
        // down
        int evy = mbright.r & 0x1F;
        if (evy > 16) evy = 16;

        pixel -= ((pixel * evy) + 0xF) >> 4;
    }
    return pixel;
}

vec2 getHorizontalDualScreen2DTextureCoordinates(float xpos, float ypos)
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
            return fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
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
            return fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)) + vec2(0, 192);
        }
    }


    // nothing (clear screen)
    return vec2(-1, -1);
}

vec2 getVerticalDualScreen2DTextureCoordinates(float xpos, float ypos)
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
            return fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin));
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
            return fixStretch*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)) + vec2(0, 192);
        }
    }

    // nothing (clear screen)
    return vec2(-1, -1);
}

ivec4 getForcedAspectRatioScreen3DColor(float xpos, float ypos)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = (1.0/forcedAspectRatio)/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    float sourceScreenWidth = 256.0;
    float screenLeftMargin = (sourceScreenWidth - sourceScreenWidth*heightScale)/2;

    ivec2 position3d = ivec2((fixStretch*(texPosition3d - vec2(screenLeftMargin, 0)))*u3DScale);
    return ivec4(texelFetch(_3DTex, position3d, 0).bgra * vec4(63,63,63,31));
}

ivec4 getHorizontalDualScreen3DColor(float xpos, float ypos)
{
    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    float _3dxpos = float(mbright.a - ((mbright.b & 0x80) * 2));

    vec2 texPosition3d = vec2(xpos - _3dxpos, ypos)*u3DScale;
    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(1.0, heightScale);

    // screen 1
    {
        float sourceScreenHeight = 192.0;
        float sourceScreenWidth = 256.0;
        float screenHeight = (sourceScreenHeight*widthScale*u3DScale)/2;
        float screenWidth = (sourceScreenWidth*u3DScale)/2;
        float screenTopMargin = (sourceScreenHeight*u3DScale - screenHeight)/2;
        float screenLeftMargin = 0.0;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            ivec2 position3d = ivec2(fixStretch*2*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)));
            return ivec4(texelFetch(_3DTex, position3d + ivec2(_3dxpos*u3DScale, 0), 0).bgra
                * vec4(63,63,63,31));
        }
    }

    // screen 2
    {
        float sourceScreenHeight = 192.0;
        float sourceScreenWidth = 256.0;
        float screenHeight = (sourceScreenHeight*widthScale*u3DScale)/2;
        float screenWidth = (sourceScreenWidth*u3DScale)/2;
        float screenTopMargin = (sourceScreenHeight*u3DScale - screenHeight)/2;
        float screenLeftMargin = screenWidth;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            ivec2 position3d = ivec2(fixStretch*2*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)));
            return ivec4(texelFetch(_3DTex, position3d + ivec2(_3dxpos*u3DScale, 0), 0).bgra
                * vec4(63,63,63,31));
        }
    }

    return ivec4(63,63,63,0);
}

ivec4 getVerticalDualScreen3DColor(float xpos, float ypos)
{
    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    float _3dxpos = float(mbright.a - ((mbright.b & 0x80) * 2));

    vec2 texPosition3d = vec2(xpos - _3dxpos, ypos)*u3DScale;
    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    // screen 1
    {
        float sourceScreenHeight = 192.0;
        float sourceScreenWidth = 256.0;
        float screenHeight = (sourceScreenHeight*u3DScale)/2;
        float screenWidth = (sourceScreenWidth*heightScale*u3DScale)/2;
        float screenTopMargin = 0.0;
        float screenLeftMargin = (sourceScreenWidth*u3DScale - screenWidth)/2;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            ivec2 position3d = ivec2(fixStretch*2*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)));
            return ivec4(texelFetch(_3DTex, position3d + ivec2(_3dxpos*u3DScale, 0), 0).bgra
                * vec4(63,63,63,31));
        }
    }


    // screen 2
    {
        float sourceScreenHeight = 192.0;
        float sourceScreenWidth = 256.0;
        float screenHeight = (sourceScreenHeight*u3DScale)/2;
        float screenWidth = (sourceScreenWidth*heightScale*u3DScale)/2;
        float screenTopMargin = screenHeight;
        float screenLeftMargin = (sourceScreenWidth*u3DScale - screenWidth)/2;
        if (texPosition3d.x >= screenLeftMargin &&
            texPosition3d.x < (screenWidth + screenLeftMargin) &&
            texPosition3d.y <= (screenHeight + screenTopMargin) &&
            texPosition3d.y >= screenTopMargin) {
            ivec2 position3d = ivec2(fixStretch*2*vec2(texPosition3d - vec2(screenLeftMargin, screenTopMargin)));
            return ivec4(texelFetch(_3DTex, position3d + ivec2(_3dxpos*u3DScale, 0), 0).bgra
                * vec4(63,63,63,31));
        }
    }

    return ivec4(63,63,63,0);
}

ivec4 getTopScreen3DColor()
{
    int yCoord = int(fTexcoord.y);

    if (screenLayout == 1) { // bottom
        yCoord = int(mod(yCoord + 192, 192*2));
    }

    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256 * 3, yCoord), 0));
    float _3dxpos = float(mbright.a - ((mbright.b & 0x80) * 2));

    float xpos = fTexcoord.x + _3dxpos;
    float ypos = mod(fTexcoord.y, 192);

    ivec2 position3d = ivec2(vec2(xpos, ypos) * u3DScale);
    ivec4 _3dpix = ivec4(texelFetch(_3DTex, position3d, 0).bgra * vec4(63, 63, 63, 31));

    if (fTexcoord.y <= 192) { // top screen
        if (screenLayout == 2) { // vertical
            return getVerticalDualScreen3DColor(xpos, ypos);
        }
        if (screenLayout == 3) { // horizontal
            return getHorizontalDualScreen3DColor(xpos, ypos);
        }
        if (currentAspectRatio != forcedAspectRatio) {
            return getForcedAspectRatioScreen3DColor(xpos, ypos);
        }
    }

    return _3dpix;
}

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

vec2 mod_vec2(vec2 vector1, vec2 vector2)
{
    float x = vector1.x;
    float y = vector1.y;
    while (x >= vector2.x) {
        x -= vector2.x;
    }
    while (y >= vector2.y) {
        y -= vector2.y;
    }
    return vec2(x, y);
}

ivec4 getTopScreenColor(ivec4 _3dpix, float xpos, float ypos, int index)
{
    if (screenLayout == 2) { // vertical
        ivec2 textureBeginning = ivec2(getVerticalDualScreen2DTextureCoordinates(xpos, ypos));
        if (textureBeginning.x == -1 && textureBeginning.y == -1) {
            if (index == 1)
            {
                return ivec4(0, 0, 0, 0);
            }
            if (index == 2)
            {
                return ivec4(0, 0, 63, 0x01);
            }
            return ivec4(0, 0, 0, 0);
        }

        ivec2 coordinates = textureBeginning + ivec2(256,0)*index;
        ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));
        if (index == 2) {
            // provides full transparency support to the transparency layer
            color.g = color.g << 2;
            color.b = color.b << 2;
        }
        return color;
    }

    if (screenLayout == 3) { // horizontal
        ivec2 textureBeginning = ivec2(getHorizontalDualScreen2DTextureCoordinates(xpos, ypos));
        if (textureBeginning.x == -1 && textureBeginning.y == -1) {
            textureBeginning = ivec2(128, 0);

            int yOffset = (xpos < 128) ? 96 : (192 + 96);
            ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256 * 3, yOffset), 0));
            if ((mbright.b & 0x3) != 1 || ((mbright.g >> 6) == 2 && mbright.r >= 16))
            {
                // black screen due to brightness
                textureBeginning = ivec2(-1, -1);
            }
        }

        if (textureBeginning.x == -1 && textureBeginning.y == -1) {
            if (index == 2) {
                return ivec4(255, 255, 255, 0);
            }
        }

        ivec2 coordinates = textureBeginning + ivec2(256,0)*index;
        ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));
        if (index == 2) {
            // provides full transparency support to the transparency layer
            color.g = color.g << 2;
            color.b = color.b << 2;
        }
        return color;
    }

    if (showOriginalHud) {
        ivec2 textureBeginning = ivec2(fTexcoord);
        if (currentAspectRatio != forcedAspectRatio) {
            float heightScale = (1.0/forcedAspectRatio)/currentAspectRatio;

            float sourceScreenWidth = 256.0;
            float screenLeftMargin = (sourceScreenWidth - sourceScreenWidth/heightScale)/2;
            float screenFinalWidth = fTexcoord.x/heightScale;

            textureBeginning.x = int(screenLeftMargin + screenFinalWidth);
            if (textureBeginning.x < 0 || textureBeginning.x > screenFinalWidth) {
                if (index == 2)
                {
                    return ivec4(0, 0, 63, 0x01);
                }
                return ivec4(0, 0, 0, 0);
            }
        }

        textureBeginning = (screenLayout == 1) ? (textureBeginning + ivec2(0, 192)) : textureBeginning;
        ivec2 coordinates = textureBeginning + ivec2(256,0)*index;
        ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));
        if (index == 2) {
            // provides full transparency support to the transparency layer
            color.g = color.g << 2;
            color.b = color.b << 2;
        }
        return color;
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

                ivec2 textureBeginning = ivec2(finalPos) + shapes[shapeIndex].squareInitialCoords.xy;
                ivec4 alphaColor = ivec4(texelFetch(ScreenTex, textureBeginning + ivec2(512,0), 0));
                if ((effects & 0x100) == 0 && (
                        (alphaColor.a == 0x0) ||
                        (alphaColor.a == 0x1 && _3dpix.a > 0 && alphaColor.g == 0) ||
                        (alphaColor.a == 0x2 && _3dpix.a > 0 && alphaColor.g < 4) ||
                        (alphaColor.a == 0x3 && _3dpix.a > 0 && alphaColor.g < 4) ||
                        (alphaColor.a == 0x4 && (_3dpix.a & 0x1F) == 0x1F)
                )) {
                    continue; // invisible pixel; ignore it
                }

                // single color to alpha
                bool shouldSkipColor = false;
                for (int colorIndex = 0; colorIndex < SINGLE_COLOR_TO_ALPHA_ARRAY_SIZE; colorIndex++) {
                    ivec4 singleColorToAlpha = shapes[shapeIndex].singleColorToAlpha[colorIndex];
                    if (singleColorToAlpha.a > 0)
                    {
                        ivec4 colorZero = ivec4(texelFetch(ScreenTex, textureBeginning, 0));
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

                if (index == 0) {
                    ivec4 color = ivec4(texelFetch(ScreenTex, textureBeginning, 0));

                    // invert gray scale colors
                    if ((effects & 0x1) != 0) {
                        bool isShadeOfGray = (abs(color.r - color.g) < 5) && (abs(color.r - color.b) < 5) && (abs(color.g - color.b) < 5);
                        if (isShadeOfGray) {
                            color = ivec4(64 - color.r, 64 - color.g, 64 - color.b, color.a);
                        }
                    }

                    if (brightnessMode == 6) { // brightnessMode_Auto
                        color = applyBrightness(color, ivec4(texelFetch(ScreenTex, ivec2(256*3, int(textureBeginning.y)), 0)));
                    }

                    return color;
                }
                else if (index == 1) {
                    ivec2 coordinates = textureBeginning + ivec2(256,0);
                    return ivec4(texelFetch(ScreenTex, coordinates, 0));
                }
                else { // index == 2
                    ivec2 coordinates = textureBeginning + ivec2(512,0);
                    ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));

                    // provides full transparency support to the transparency layer
                    color.g = color.g << 2;
                    color.b = color.b << 2;

                    // manipulate transparency
                    if ((effects & 0x20) != 0)
                    {
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
                            color = ivec4(color.r, blurVal /* 2D visibility */, 63 - blurVal /* 3D visibility */, 0x01);

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
                    }
                    return color;
                }
            }
        }
    }

    if (index == 2)
    {
        return ivec4(0, 0, 63, 0x01);
    }
    return ivec4(0, 0, 0, 0);
}

ivec4 brightness()
{
    if (brightnessMode == 1) { // brightnessMode_TopScreen
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }
    if (brightnessMode == 2) { // brightnessMode_BottomScreen
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 192 + int(fTexcoord.y)), 0));
    }
    if (brightnessMode == 3) { // brightnessMode_Horizontal
        int yOffset = (fTexcoord.x < 128) ? 96 : (192 + 96);
        return ivec4(texelFetch(ScreenTex, ivec2(256 * 3, yOffset), 0));
    }
    if (brightnessMode == 4) { // brightnessMode_BlackScreen
        return ivec4(0x1F, 2 << 6, 0x2, 0);
    }
    if (brightnessMode == 5) { // brightnessMode_DisableBrightnessControl
        return ivec4(0, 0, 0x1, 0);
    }
    if (brightnessMode == 6) { // brightnessMode_Auto
        return ivec4(0, 0, 0x1, 0);
    }

    // brightnessMode_Default
    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
    int brightmode = mbright.g >> 6;
    if ((mbright.b & 0x3) != 0 && brightmode == 2) {
        return mbright;
    }
    return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
}

void main()
{
    ivec4 pixel = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord), 0));

    ivec4 mbright = brightness();
    int dispmode = mbright.b & 0x3;

    if (dispmode == 1)
    {
        ivec4 _3dpix = getTopScreen3DColor();

        if (fTexcoord.y <= 192) // top screen
        {
            ivec4 val1 = getTopScreenColor(_3dpix, fTexcoord.x, fTexcoord.y, 0);
            ivec4 val2 = getTopScreenColor(_3dpix, fTexcoord.x, fTexcoord.y, 1);
            ivec4 val3 = getTopScreenColor(_3dpix, fTexcoord.x, fTexcoord.y, 2);
            pixel = combineLayers(_3dpix, val1, val2, val3);
        }
        else // bottom screen
        {
            ivec4 val1 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord), 0));
            ivec4 val2 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(256,0), 0));
            ivec4 val3 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(512,0), 0));
            pixel = combineLayers(_3dpix, val1, val2, val3);
        }
    }

    if (dispmode != 0)
    {
        pixel = applyBrightness(pixel, mbright);
    }

    pixel.rgb <<= 2;
    pixel.rgb |= (pixel.rgb >> 6);

    oColor = vec4(vec3(pixel.bgr) / 255.0, 1.0);
}
)";

// language=GLSL
const char* k2DCompositorFS_Plugin = R"(#version 140

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

    vec4 layercol[6];
    layercol[0] = BG0CalcAndFetch(bgcoord, line);
    layercol[1] = BG1CalcAndFetch(bgcoord, line);
    layercol[2] = BG2CalcAndFetch(bgcoord, line);
    layercol[3] = BG3CalcAndFetch(bgcoord, line);

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
    oColor = CompositeLayers();
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

                ivec2 textureBeginning = ivec2(finalPos) + shapes[shapeIndex].squareInitialCoords.xy;
                bool isBottomScreen = textureBeginning.y >= 192;
                if (isBottomScreen) {
                    textureBeginning.y = textureBeginning.y - 192;
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
