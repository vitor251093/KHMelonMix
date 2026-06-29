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

    vec4 squareInitialCoords;
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
uniform float hudScale;
uniform int brightnessMode;

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


bool swapScreens() {
    int line = int(fTexcoord.y * 192);
    bool swapbit = uScreenSwap[line>>2][line&0x3];
    return !swapbit;
}
int topDispMode(bool shouldSwapScreens) {
    return shouldSwapScreens ? uDispModeB : uDispModeA;
}
int bottomDispMode(bool shouldSwapScreens) {
    return shouldSwapScreens ? uDispModeA : uDispModeB;
}
int topBrightMode(bool shouldSwapScreens) {
    return shouldSwapScreens ? uBrightModeB : uBrightModeA;
}
int bottomBrightMode(bool shouldSwapScreens) {
    return shouldSwapScreens ? uBrightModeA : uBrightModeB;
}
int topBrightFactor(bool shouldSwapScreens) {
    return shouldSwapScreens ? uBrightFactorB : uBrightFactorA;
}
int bottomBrightFactor(bool shouldSwapScreens) {
    return shouldSwapScreens ? uBrightFactorA : uBrightFactorB;
}

ivec4 MasterBrightnessWithAlpha(ivec4 color, int dispMode, int brightmode, int evy, bool evyToAlpha)
{
    int alpha = color.a;

    if (evyToAlpha)
    {
        if (dispMode == 0)
        {
            // screen disabled (white)
            alpha = 0;
        }

        if (brightmode == 1 || brightmode == 2)
        {
            int evyFactor = (evy == 0) ? 0 : ((evy + 1) << 3) - 1;
            alpha = (alpha * (255 - evyFactor)) / 255;
        }

        color.a = alpha;
        return color;
    }

    if (dispMode == 0)
    {
        // screen disabled (white)
        return ivec4(0xFF, 0xFF, 0xFF, alpha);
    }
    if (brightmode == 1)
    {
        // up
        color += (((0xFF - color) * evy) >> 4);
    }
    else if (brightmode == 2)
    {
        // down
        color -= (((color * evy) + 0xF) >> 4);
    }

    color.a = alpha;
    return color;
}

ivec3 MasterBrightness(ivec3 color, int dispMode, int brightmode, int evy)
{
    if (dispMode == 0)
    {
        // screen disabled (white)
        return ivec3(63, 63, 63);
    }
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

bool isValidConsideringCropSquareCorners(vec2 finalPos, vec4 cropSquareCorners, vec2 squareInitialSize) {
    return (finalPos.x + finalPos.y >= cropSquareCorners[0]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + finalPos.y >= cropSquareCorners[1]) &&
           (finalPos.x + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[2]) &&
           ((0 - finalPos.x + squareInitialSize[0]) + (0 - finalPos.y + squareInitialSize[1]) >= cropSquareCorners[3]);
}

bool isInsideRoundedCorner(vec2 pos, vec2 center, float radius) {
    return (pos.x - center.x) * (pos.x - center.x) +
           (pos.y - center.y) * (pos.y - center.y) < radius * radius;
}

bool isValidConsideringSquareBorderRadius(vec2 finalPos, vec4 radius, vec2 squareInitialSize) {
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

ivec4 ApplyBrightnessWithAlpha(ivec4 color, bool shouldSwapScreens, bool isBottomScreen) {
    int finalTopDispMode = topDispMode(shouldSwapScreens);
    int finalBottomDispMode = bottomDispMode(shouldSwapScreens);
    int finalTopBrightMode = topBrightMode(shouldSwapScreens);
    int finalBottomBrightMode = bottomBrightMode(shouldSwapScreens);
    int finalTopBrightFactor = topBrightFactor(shouldSwapScreens);
    int finalBottomBrightFactor = bottomBrightFactor(shouldSwapScreens);

    int offset = isBottomScreen ? 0 : 8;
    if ((brightnessMode & (1 << offset)) > 0 && (finalTopBrightMode == 1 || finalTopDispMode == 0)) {
        bool evyToAlpha = (brightnessMode & (4 << offset)) > 0;
        color = MasterBrightnessWithAlpha(color, finalTopDispMode, finalTopBrightMode, finalTopBrightFactor, evyToAlpha);
    }
    if ((brightnessMode & (2 << offset)) > 0 && (finalTopBrightMode == 2)) {
        bool evyToAlpha = (brightnessMode & (8 << offset)) > 0;
        color = MasterBrightnessWithAlpha(color, finalTopDispMode, finalTopBrightMode, finalTopBrightFactor, evyToAlpha);
    }
    if ((brightnessMode & (16 << offset)) > 0 && (finalBottomBrightMode == 1 || finalBottomDispMode == 0)) {
        bool evyToAlpha = (brightnessMode & (64 << offset)) > 0;
        color = MasterBrightnessWithAlpha(color, finalBottomDispMode, finalBottomBrightMode, finalBottomBrightFactor, evyToAlpha);
    }
    if ((brightnessMode & (32 << offset)) > 0 && (finalBottomBrightMode == 2)) {
        bool evyToAlpha = (brightnessMode & (128 << offset)) > 0;
        color = MasterBrightnessWithAlpha(color, finalBottomDispMode, finalBottomBrightMode, finalBottomBrightFactor, evyToAlpha);
    }
    return color;
}

ivec3 ApplyBrightness(ivec3 color, bool shouldSwapScreens, bool isBottomScreen) {
    int finalTopDispMode = topDispMode(shouldSwapScreens);
    int finalBottomDispMode = bottomDispMode(shouldSwapScreens);
    int finalTopBrightMode = topBrightMode(shouldSwapScreens);
    int finalBottomBrightMode = bottomBrightMode(shouldSwapScreens);
    int finalTopBrightFactor = topBrightFactor(shouldSwapScreens);
    int finalBottomBrightFactor = bottomBrightFactor(shouldSwapScreens);

    int offset = isBottomScreen ? 0 : 4;
    if (((brightnessMode & (1 << offset)) > 0 && (finalTopBrightMode == 1 || finalTopDispMode == 0)) ||
        ((brightnessMode & (2 << offset)) > 0 && (finalTopBrightMode == 2))) {
        color = MasterBrightness(color, finalTopDispMode, finalTopBrightMode, finalTopBrightFactor);
    }
    if (((brightnessMode & (16 << offset)) > 0 && (finalBottomBrightMode == 1 || finalBottomDispMode == 0)) ||
        ((brightnessMode & (32 << offset)) > 0 && (finalBottomBrightMode == 2))) {
        color = MasterBrightness(color, finalBottomDispMode, finalBottomBrightMode, finalBottomBrightFactor);
    }
    return color;
}

ivec4 getRegularScreenColor(vec2 textureBeginning, bool shouldSwapScreens, bool isBottomScreen) {
    if (isBottomScreen) {
        ivec4 color = shouldSwapScreens ?
                ivec4(texture(MainInputTexA, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0) :
                ivec4(texture(MainInputTexB, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
        color = ApplyBrightnessWithAlpha(color, shouldSwapScreens, isBottomScreen);
        return color;
    }

    ivec4 color = shouldSwapScreens ?
            ivec4(texture(MainInputTexB, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0) :
            ivec4(texture(MainInputTexA, textureBeginning.xy / vec2(256.0, 192.0), 0) * 255.0);
    color = ApplyBrightnessWithAlpha(color, shouldSwapScreens, isBottomScreen);
    return color;
}

// RGB; all values from 0 to 255
ivec4 getTopScreenColor(vec2 pos, bool shouldSwapScreens)
{
    float xpos = pos.x * 256.0;
    float ypos = pos.y * 192.0;

    float heightScale = 1.0/currentAspectRatio;
    float widthScale = currentAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    vec2 texPosition3d = vec2(xpos, ypos)*hudScale;

    ivec4 currentColor = getRegularScreenColor(vec2(xpos, ypos), shouldSwapScreens, false);

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
                vec2 cropAreaSize = shapes[shapeIndex].squareInitialCoords.zw;
                if (shouldRotate) {
                    cropAreaSize = shapes[shapeIndex].squareInitialCoords.wz;
                }
                validArea = isValidConsideringCropSquareCorners(finalPos, shapes[shapeIndex].squareCornersModifier, cropAreaSize);
            }
            // rounded corners
            if ((effects & 0x4) != 0) {
                vec2 cropAreaSize = shapes[shapeIndex].squareInitialCoords.zw;
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

            vec2 textureBeginning = finalPos + shapes[shapeIndex].squareInitialCoords.xy;
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
                    ivec4 colorZero = getRegularScreenColor(textureBeginning.xy, shouldSwapScreens, isBottomScreen);
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

            ivec4 color = getRegularScreenColor(textureBeginning.xy, shouldSwapScreens, isBottomScreen);

            // invert gray scale colors
            if ((effects & 0x1) != 0) {
                bool isShadeOfGray = (abs(color.r - color.g) < 20) && (abs(color.r - color.b) < 20) && (abs(color.g - color.b) < 20);
                if (isShadeOfGray) {
                    color.rgb = 255 - color.rgb;
                }
            }

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

void main()
{
    bool shouldSwapScreens = swapScreens();

    // BG/OBJ layers
    ivec4 col_main = getTopScreenColor(fTexcoord.xy, shouldSwapScreens);
    ivec4 col_sub = shouldSwapScreens ?
            (ivec4(texture(MainInputTexA, fTexcoord.xy, 0) * 255.0) >> 2) :
            (ivec4(texture(MainInputTexB, fTexcoord.xy, 0) * 255.0) >> 2);

    ivec3 output_main, output_sub;
    output_main = col_main.rgb;

    // BG/OBJ layers
    output_sub = col_sub.rgb;
    output_sub = MasterBrightness(output_sub, bottomDispMode(shouldSwapScreens), bottomBrightMode(shouldSwapScreens), bottomBrightFactor(shouldSwapScreens));
    output_sub = (output_sub << 2) | (output_sub >> 6);

    if (uDispModeA > 1)
    {
        // VRAM display / mainmem FIFO
        if (!shouldSwapScreens) {
            output_main = ivec3(texture(AuxInputTex, vec3(fTexcoord.xz, uAuxLayer)).rgb * uAuxColorFactor);
            output_main = ApplyBrightness(output_main, shouldSwapScreens, false);
            output_main = (output_main << 2) | (output_main >> 6);
        }
        else {
            output_sub = ivec3(texture(AuxInputTex, vec3(fTexcoord.xz, uAuxLayer)).rgb * uAuxColorFactor);
            output_sub = ApplyBrightness(output_sub, shouldSwapScreens, false);
            output_sub = (output_sub << 2) | (output_sub >> 6);
        }
    }

    oTopColor = vec4(vec3(output_main) / 255.0, 1.0);
    oBottomColor = vec4(vec3(output_sub) / 255.0, 1.0);
}
)";

}

#endif //MELONDS_PLUGIN_OGL_FINALPASSFS_H