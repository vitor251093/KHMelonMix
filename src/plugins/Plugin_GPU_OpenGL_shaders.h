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

struct ShapeData2D {
    vec2 sourceScale;

    int effects;

    float opacity;

    ivec4 squareInitialCoords;
    vec4 squareFinalCoords;

    vec4 fadeBorderSize;

    vec4 squareCornersModifier;

    ivec4 colorToAlpha;
    ivec4 singleColorToAlpha;
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

    if (screenLayout == 2) { // vertical
        return getVerticalDualScreen3DColor(xpos, ypos);
    }
    if (screenLayout == 3) { // horizontal
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (currentAspectRatio != forcedAspectRatio) {
        return getForcedAspectRatioScreen3DColor(xpos, ypos);
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
        ivec2 textureBeginning = (screenLayout == 1) ? (ivec2(fTexcoord) + ivec2(0, 192)) : ivec2(fTexcoord);
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

    float uiTexScale = (6.0/hudScale);
    vec2 texPosition3d = vec2(xpos, ypos)*uiTexScale;

    for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
        vec4 squareFinalCoords = shapes[shapeIndex].squareFinalCoords;

        if ((all(greaterThanEqual(texPosition3d, squareFinalCoords.xy)) &&
                all(lessThanEqual(texPosition3d, squareFinalCoords.zw))) || ((shapes[shapeIndex].effects & 0x40) != 0)) {
            int effects = shapes[shapeIndex].effects;

            vec2 finalPos = (fixStretch/shapes[shapeIndex].sourceScale)*(texPosition3d - squareFinalCoords.xy);
            bool validArea = true;

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
                validArea = isValidConsideringCropSquareCorners(finalPos, shapes[shapeIndex].squareCornersModifier, shapes[shapeIndex].squareInitialCoords.zw);
            }
            // rounded corners
            if ((effects & 0x4) != 0) {
                validArea = isValidConsideringSquareBorderRadius(finalPos, shapes[shapeIndex].squareCornersModifier, shapes[shapeIndex].squareInitialCoords.zw);
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

                ivec4 alphaColor = ivec4(texelFetch(ScreenTex, ivec2(finalPos) + shapes[shapeIndex].squareInitialCoords.xy + ivec2(512,0), 0));
                if ((effects & 0x100) == 0 && (
                        (alphaColor.a == 0x0) ||
                        (alphaColor.a == 0x1 && _3dpix.a > 0 && alphaColor.g == 0) ||
                        (alphaColor.a == 0x2 && _3dpix.a > 0 && alphaColor.g < 4) ||
                        //(alphaColor.a == 0x3 && _3dpix.a > 0) ||
                        (alphaColor.a == 0x4 && (_3dpix.a & 0x1F) == 0x1F)
                )) {
                    continue; // invisible pixel; ignore it
                }

                if (index == 0) {
                    ivec2 coordinates = ivec2(finalPos) + shapes[shapeIndex].squareInitialCoords.xy;
                    ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));

                    // invert gray scale colors
                    if ((effects & 0x1) != 0) {
                        bool isShadeOfGray = (abs(color.r - color.g) < 5) && (abs(color.r - color.b) < 5) && (abs(color.g - color.b) < 5);
                        if (isShadeOfGray) {
                            color = ivec4(64 - color.r, 64 - color.g, 64 - color.b, color.a);
                        }
                    }

                    if (brightnessMode == 6) { // brightnessMode_Auto
                        color = applyBrightness(color, ivec4(texelFetch(ScreenTex, ivec2(256*3, int(coordinates.y)), 0)));
                    }

                    return color;
                }
                else if (index == 1) {
                    ivec2 coordinates = ivec2(finalPos + shapes[shapeIndex].squareInitialCoords.xy + vec2(256,0));
                    return ivec4(texelFetch(ScreenTex, coordinates, 0));
                }
                else { // index == 2
                    ivec2 textureBeginning = ivec2(finalPos) + shapes[shapeIndex].squareInitialCoords.xy;
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

                        ivec4 singleColorToAlpha = shapes[shapeIndex].singleColorToAlpha;
                        if (singleColorToAlpha.a > 0)
                        {
                            ivec4 colorZero = ivec4(texelFetch(ScreenTex, textureBeginning, 0));
                            if (colorZero.r == singleColorToAlpha.r &&
                                colorZero.g == singleColorToAlpha.g &&
                                colorZero.b == singleColorToAlpha.b) {
                                color = ivec4(color.r, singleColorToAlpha.a - 1, 65 - singleColorToAlpha.a, 0x01);
                            }
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

}

#endif // PLUGIN_GPU_OPENGL_SHADERS_H
