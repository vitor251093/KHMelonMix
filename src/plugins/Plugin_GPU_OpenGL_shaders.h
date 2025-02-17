/*
    Copyright 2016-2024 VitorMM

    This file is part of KhDays Melox Mix, which is based on melonDS.

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
const char* kCompositorFS_Plugin = R"(#version 140

#define MAX_SHAPES 100

struct ShapeData {
    int enabled;

    int shape; // 0 = SQUARE, 1 = FREEFORM
    int corner; // 0 = center, 1-8 = corners
    float scale;

    ivec4 square;      // X, Y, Width, Height (only valid if shape == 0)
    ivec4 freeForm[4]; // Four (X, Y) points (only valid if shape == 1)
    vec4 margin;       // left, top, right, down

    // effects
    vec4 fadeBorderSize; // left fade border, top fade border, right fade border, down fade border
    int invertGrayScaleColors;
    int _pad0, _pad1, _pad2;  // Padding to align the struct to 16 bytes

    vec4 cropSquareCorners;

    ivec4 colorToAlpha;
};

layout(std140) uniform ShapeBlock {
    ShapeData shapes[MAX_SHAPES];
};

uniform float currentAspectRatio;
uniform float forcedAspectRatio;

uniform int uiScale;
uniform bool showOriginalHud;
uniform int screenLayout; // 0 = top screen, 1 = bottom screen, 2 = both vertical, 3 = both horizontal
uniform int brightnessMode; // 0 = default, 1 = top screen, 2 = bottom screen, 3 = no brightness

uniform int shapeCount;


uniform uint u3DScale;
uniform usampler2D ScreenTex;
uniform sampler2D _3DTex;

smooth in vec2 fTexcoord;

out vec4 oColor;

// provides full transparency support to the transparency layer
ivec4 fixTransparencyLayer(ivec4 layer)
{
    layer.g = layer.g << 2;
    layer.b = layer.b << 2;
    return layer;
}

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

vec2 getHorizontalDualScreen2DTextureCoordinates(float xpos, float ypos, vec2 clearVect)
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
    return clearVect;
}

vec2 getVerticalDualScreen2DTextureCoordinates(float xpos, float ypos, vec2 clearVect)
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
    return clearVect;
}

vec4 get3DCoordinatesOf2DSquareShape(ShapeData shapeData)
{
    float iuTexScale = (6.0)/uiScale;
    float heightScale = 1.0/currentAspectRatio;

    float scale = shapeData.scale;
    if (scale == 0) {
        iuTexScale = 1.0;
        scale = 1.0;
    }
    float squareFinalHeight = shapeData.square[3]*scale;
    float squareFinalWidth = shapeData.square[2]*scale*heightScale;

    float squareFinalX1 = 0.0;
    float squareFinalY1 = 0.0;

    switch (shapeData.corner)
    {
        case 0: // square at center
            squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
            squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
            break;
        
        case 1: // square at top left corner
            squareFinalX1 = shapeData.margin[0];
            squareFinalY1 = shapeData.margin[1];
            break;
        
        case 2: // square at top
            squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
            squareFinalY1 = shapeData.margin[1];
            break;

        case 3: // square at top right corner
            squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - shapeData.margin[2];
            squareFinalY1 = shapeData.margin[1];
            break;

        case 4: // square at right
            squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - shapeData.margin[2];
            squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
            break;

        case 5: // square at bottom right corner
            squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - shapeData.margin[2];
            squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - shapeData.margin[3];
            break;

        case 6: // square at bottom
            squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
            squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - shapeData.margin[3];
            break;

        case 7: // square at left bottom corner
            squareFinalX1 = shapeData.margin[0];
            squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - shapeData.margin[3];
            break;

        case 8: // square at left
            squareFinalX1 = shapeData.margin[0];
            squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
    }

    float squareFinalX2 = squareFinalX1 + squareFinalWidth;
    float squareFinalY2 = squareFinalY1 + squareFinalHeight;

    return vec4(squareFinalX1, squareFinalY1, squareFinalX2, squareFinalY2);
}

ivec2 getTopScreen2DTextureCoordinates(float xpos, float ypos)
{
    if (showOriginalHud) {
        if (screenLayout == 1) { // bottom
            return ivec2(fTexcoord) + ivec2(0, 192);
        }
        return ivec2(fTexcoord);
    }

    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
        ShapeData shapeData = shapes[shapeIndex];
    
        if (shapeData.enabled == 1 && shapeData.shape == 0) { // square
            vec4 shape3DCoords = get3DCoordinatesOf2DSquareShape(shapeData);

            float iuTexScale = (6.0)/uiScale;
            float scale = shapeData.scale;
            if (scale == 0) {
                iuTexScale = 1.0;
                scale = 1.0;
            }
            vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;

            if (texPosition3d.x >= shape3DCoords[0] &&
                texPosition3d.x <= shape3DCoords[2] && 
                texPosition3d.y >= shape3DCoords[1] && 
                texPosition3d.y <= shape3DCoords[3]) {

                vec2 finalPos = (1.0/scale)*fixStretch*(texPosition3d - vec2(shape3DCoords[0], shape3DCoords[1]));
                if (dot(shapeData.cropSquareCorners, shapeData.cropSquareCorners) == 0.0) {
                    return ivec2(finalPos) + ivec2(shapeData.square[0], shapeData.square[1]);
                }
                else if ((finalPos.x + finalPos.y >= shapeData.cropSquareCorners[0]) &&
                         (finalPos.y - finalPos.x + shapeData.square[2] >= shapeData.cropSquareCorners[1])) {
                    return ivec2(finalPos) + ivec2(shapeData.square[0], shapeData.square[1]);
                }
            }
        }
    }

    // nothing (clear screen)
    return ivec2(-1, -1);
}

ivec2 getScreen2DTextureCoordinates(float xpos, float ypos)
{
    if (screenLayout == 2) { // vertical
        return ivec2(getVerticalDualScreen2DTextureCoordinates(xpos, ypos, vec2(-1, -1)));
    }
    if (screenLayout == 3) { // horizontal
        return ivec2(getHorizontalDualScreen2DTextureCoordinates(xpos, ypos, vec2(128, 0)));
    }

    return getTopScreen2DTextureCoordinates(xpos, ypos);
}

ivec4 getForcedAspectRatioScreen3DColor(float xpos, float ypos)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = (1.0/forcedAspectRatio)/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    float sourceScreenHeight = 192.0;
    float sourceScreenWidth = 256.0;
    float sourceScreenTopMargin = 0;
    float sourceScreenLeftMargin = 0;
    float screenHeight = sourceScreenHeight;
    float screenWidth = sourceScreenWidth*heightScale;
    float screenTopMargin = 0;
    float screenLeftMargin = (256.0 - screenWidth)/2;
    if (texPosition3d.x >= screenLeftMargin &&
        texPosition3d.x < (screenWidth + screenLeftMargin) && 
        texPosition3d.y <= (screenHeight + screenTopMargin) && 
        texPosition3d.y >= screenTopMargin) {
        ivec2 position3d = ivec2((fixStretch*(texPosition3d - vec2(screenLeftMargin, screenTopMargin)) +
            vec2(sourceScreenLeftMargin, sourceScreenTopMargin))*u3DScale);
        return ivec4(texelFetch(_3DTex, position3d, 0).bgra
            * vec4(63,63,63,31));
    }
    return ivec4(63,63,63,0);
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

ivec4 getTopScreenColor(float xpos, float ypos, int index)
{
    ivec2 textureBeginning = getScreen2DTextureCoordinates(xpos, ypos);
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
    if (index == 1) {
        return color;
    }
    if (index == 2) {
        color = fixTransparencyLayer(color);
    }

    float iuTexScale = (6.0)/uiScale;
    vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
    for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
        ShapeData shapeData = shapes[shapeIndex];
    
        if (shapeData.enabled == 1 && shapeData.shape == 0) { // square
            if (index == 0 && shapeData.invertGrayScaleColors == 1) {
                vec4 shape3DCoords = get3DCoordinatesOf2DSquareShape(shapeData);

                if (texPosition3d.x >= shape3DCoords[0] &&
                    texPosition3d.x <= shape3DCoords[2] && 
                    texPosition3d.y >= shape3DCoords[1] && 
                    texPosition3d.y <= shape3DCoords[3]) {
                    
                    bool isShadeOfGray = (abs(color.r - color.g) < 5) && (abs(color.r - color.b) < 5) && (abs(color.g - color.b) < 5);
                    if (isShadeOfGray) {
                        color = ivec4(64 - color.r, 64 - color.g, 64 - color.b, color.a);
                    }
                }
            }
            if (index == 2 && (shapeData.fadeBorderSize[0] > 0 || shapeData.fadeBorderSize[1] > 0 ||
                               shapeData.fadeBorderSize[2] > 0 || shapeData.fadeBorderSize[3] > 0)) {
                vec4 shape3DCoords = get3DCoordinatesOf2DSquareShape(shapeData);

                if (texPosition3d.x >= shape3DCoords[0] &&
                    texPosition3d.x <= shape3DCoords[2] && 
                    texPosition3d.y >= shape3DCoords[1] && 
                    texPosition3d.y <= shape3DCoords[3]) {
                
                    float leftDiff = texPosition3d.x - shape3DCoords[0];
                    float rightDiff = shape3DCoords[2] - texPosition3d.x;
                    float topDiff = texPosition3d.y - shape3DCoords[1];
                    float bottomDiff = shape3DCoords[3] - texPosition3d.y;

                    float heightScale = 1.0/currentAspectRatio;
                    float leftBlurFactor   = shapeData.fadeBorderSize[0] == 0 ? 1.0 : clamp(leftDiff   / (shapeData.fadeBorderSize[0] * heightScale), 0.0, 1.0);
                    float topBlurFactor    = shapeData.fadeBorderSize[1] == 0 ? 1.0 : clamp(topDiff    /  shapeData.fadeBorderSize[1], 0.0, 1.0);
                    float rightBlurFactor  = shapeData.fadeBorderSize[2] == 0 ? 1.0 : clamp(rightDiff  / (shapeData.fadeBorderSize[2] * heightScale), 0.0, 1.0);
                    float bottomBlurFactor = shapeData.fadeBorderSize[3] == 0 ? 1.0 : clamp(bottomDiff /  shapeData.fadeBorderSize[3], 0.0, 1.0);

                    float xBlur = min(leftBlurFactor, rightBlurFactor);
                    float yBlur = min(topBlurFactor, bottomBlurFactor);

                    float blur = xBlur * yBlur * 63.0;
                    color = ivec4(color.r, int(blur), 64 - int(blur), 0x01);
                }
            }
            if (index == 2 && shapeData.colorToAlpha.r != -1) {
                vec4 shape3DCoords = get3DCoordinatesOf2DSquareShape(shapeData);

                if (texPosition3d.x >= shape3DCoords[0] &&
                    texPosition3d.x <= shape3DCoords[2] && 
                    texPosition3d.y >= shape3DCoords[1] && 
                    texPosition3d.y <= shape3DCoords[3]) {
                    
                    ivec4 colorZero = ivec4(texelFetch(ScreenTex, textureBeginning, 0));
                    int blur = ((abs(shapeData.colorToAlpha.r - colorZero.r) +
                                 abs(shapeData.colorToAlpha.g - colorZero.g) +
                                 abs(shapeData.colorToAlpha.b - colorZero.b))*2)/3;
                    color = ivec4(color.r, blur, 64 - blur, 0x01);
                }
            }
        }
    }

    return color;
}

ivec4 brightness()
{
    if (brightnessMode == 1) { // top screen brightness
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }
    if (brightnessMode == 2) { // bottom screen brightness
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 192 + int(fTexcoord.y)), 0));
    }
    if (brightnessMode == 3) { // no brightness
        return ivec4(0x1F, 2 << 6, 0x2, 0);
    }

    // brightnessMode == 0
    if (screenLayout == 3) { // horizontal
        int yOffset = (fTexcoord.x < 128) ? 96 : (192 + 96);
        return ivec4(texelFetch(ScreenTex, ivec2(256 * 3, yOffset), 0));
    }

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
            ivec4 val1 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 0);
            ivec4 val2 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 1);
            ivec4 val3 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 2);
            pixel = combineLayers(_3dpix, val1, val2, val3);
        }
        else // bottom screen
        {
            oColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
    }

    if (dispmode != 0)
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
    }

    pixel.rgb <<= 2;
    pixel.rgb |= (pixel.rgb >> 6);

    oColor = vec4(vec3(pixel.bgr) / 255.0, 1.0);
}
)";

}

#endif // PLUGIN_GPU_OPENGL_SHADERS_H
