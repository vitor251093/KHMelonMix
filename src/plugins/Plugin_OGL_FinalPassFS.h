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

#ifndef MELONDS_PLUGIN_OGL_FINALPASSFS_H
#define MELONDS_PLUGIN_OGL_FINALPASSFS_H

namespace Plugins
{
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
        ivec4 color = ivec4(texture(MainInputTexB, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
        color.a = 255;
        return color;
    }
    ivec4 color = ivec4(texture(MainInputTexA, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
    color.a = 255;
    return color;
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

    ivec4 currentColor = ivec4(texture(MainInputTexA, fTexcoord.xy, 0) * 255.0);
    currentColor.w = 255;

    for (int shapeIndex = shapeCount - 1; shapeIndex >= 0; shapeIndex--) {
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

            if (!validArea)
            {
                continue;
            }

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
            if ((effects & 0x20) != 0)
            {
                ivec4 colorToAlpha = shapes[shapeIndex].colorToAlpha;
                if (colorToAlpha.a == 1)
                {
                    int blur = ((abs(colorToAlpha.r - color.r) +
                                 abs(colorToAlpha.g - color.g) +
                                 abs(colorToAlpha.b - color.b))*2)/3;
                    color.a = min(blur, 255);
                }

                vec4 fadeBorderSize = shapes[shapeIndex].fadeBorderSize;
                float opacity = shapes[shapeIndex].opacity;
                if (any(greaterThan(fadeBorderSize, vec4(0))) || opacity < 1.0)
                {
                    float leftDiff = texPosition3d.x - squareFinalCoords[0];
                    float topDiff  = texPosition3d.y - squareFinalCoords[1];
                    float rightDiff  = squareFinalCoords[2] - texPosition3d.x;
                    float bottomDiff = squareFinalCoords[3] - texPosition3d.y;

                    float leftBlurFactor   = fadeBorderSize[0] == 0 ? 1.0 : clamp(leftDiff   / (fadeBorderSize[0] * heightScale), 0.0, 1.0);
                    float topBlurFactor    = fadeBorderSize[1] == 0 ? 1.0 : clamp(topDiff    /  fadeBorderSize[1], 0.0, 1.0);
                    float rightBlurFactor  = fadeBorderSize[2] == 0 ? 1.0 : clamp(rightDiff  / (fadeBorderSize[2] * heightScale), 0.0, 1.0);
                    float bottomBlurFactor = fadeBorderSize[3] == 0 ? 1.0 : clamp(bottomDiff /  fadeBorderSize[3], 0.0, 1.0);

                    float xBlur = min(leftBlurFactor, rightBlurFactor);
                    float yBlur = min(topBlurFactor, bottomBlurFactor);
                    color.a = int(xBlur * yBlur * opacity * color.a);
                }
            }

            currentColor = ivec4(mix(currentColor, color, color.a / 255.0));
        }
    }

    return currentColor;
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

#endif //MELONDS_PLUGIN_OGL_FINALPASSFS_H