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

#ifndef KHRECODED_GPU_OPENGL_SHADERS_H
#define KHRECODED_GPU_OPENGL_SHADERS_H

namespace Plugins
{
const char* kCompositorFS_KhReCoded = R"(#version 140

uniform uint u3DScale;
uniform int GameScene;
uniform int KHUIScale;
uniform float TopScreenAspectRatio;
uniform bool ShowMap;
uniform int MinimapCenterX;
uniform int MinimapCenterY;
uniform bool HideAllHUD;
uniform int DSCutsceneState;

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

            val1 = ((val1 * eva) + (_3dpix * evb) + 0x8) >> 4;
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
            evy = val3.g;

            val1 = _3dpix;
            if      (compmode == 2) val1 += (((0x3F - val1) * evy) + 0x8) >> 4;
            else if (compmode == 3) val1 -= ((val1 * evy) + 0x7) >> 4;
        }
        else
            val1 = val2;
    }

    return val1;
}

bool is2DGraphicDifferentFromColor(ivec4 diffColor, ivec2 texcoord)
{
    ivec4 val1 = ivec4(texelFetch(ScreenTex, texcoord, 0));
    ivec4 val2 = ivec4(texelFetch(ScreenTex, texcoord + ivec2(256,0), 0));
    ivec4 val3 = ivec4(texelFetch(ScreenTex, texcoord + ivec2(512,0), 0));
    ivec4 pixel = combineLayers(diffColor, val1, val2, val3);
    return (!(pixel.r == diffColor.r && pixel.g == diffColor.g && pixel.b == diffColor.b));
}

bool is2DGraphicEqualToColor(ivec4 diffColor, ivec2 texcoord)
{
    ivec4 val1 = ivec4(texelFetch(ScreenTex, texcoord, 0));
    ivec4 val2 = ivec4(texelFetch(ScreenTex, texcoord + ivec2(256,0), 0));
    ivec4 val3 = ivec4(texelFetch(ScreenTex, texcoord + ivec2(512,0), 0));
    ivec4 pixel = combineLayers(diffColor, val1, val2, val3);
    return (pixel.r == diffColor.r && pixel.g == diffColor.g && pixel.b == diffColor.b);
}

bool isMissionInformationVisibleOnTopScreen()
{
    return is2DGraphicDifferentFromColor(ivec4(63,0,0,31), ivec2(256/2, 0)) ||
           is2DGraphicDifferentFromColor(ivec4(63,0,0,31), ivec2(256/2, 10));
}

bool isMissionInformationVisible()
{
    return isMissionInformationVisibleOnTopScreen();
}

bool isDialogVisible()
{
    return is2DGraphicDifferentFromColor(ivec4(0,0,0,31), ivec2(256/2, 192*0.809));
}

bool isMinimapVisible()
{
    ivec4 minimapSecurityPixel = ivec4(texelFetch(ScreenTex, ivec2(1, 190) + ivec2(0,192), 0));
    return minimapSecurityPixel.r < 5 && minimapSecurityPixel.g < 15 && minimapSecurityPixel.b > 39;
}

bool isCommandMenuVisible()
{
    return is2DGraphicDifferentFromColor(ivec4(0,63,0,31), ivec2(35, 185));
}

bool isHealthVisible()
{
    return is2DGraphicDifferentFromColor(ivec4(0,63,0,31), ivec2(225, 170));
}

bool isColorBlack(ivec4 pixel)
{
    return pixel.r < 5 && pixel.g < 5 && pixel.b < 5;
}
bool isColorVeryBlack(ivec4 pixel)
{
    return pixel.r < 2 && pixel.g < 2 && pixel.b < 2;
}
bool isColorWhite(ivec4 pixel)
{
    return pixel.r > 60 && pixel.g > 60 && pixel.b > 60;
}
ivec4 getSimpleColorAtCoordinate(float xpos, float ypos)
{
    ivec2 position3d = ivec2(vec2(xpos, ypos)*u3DScale);
    ivec4 _3dpix = ivec4(texelFetch(_3DTex, position3d, 0).bgra
            * vec4(63,63,63,31));

    vec2 texcoord = vec2(xpos, ypos);
    ivec4 val1 = ivec4(texelFetch(ScreenTex, ivec2(texcoord), 0));
    ivec4 val2 = ivec4(texelFetch(ScreenTex, ivec2(texcoord) + ivec2(256,0), 0));
    ivec4 val3 = ivec4(texelFetch(ScreenTex, ivec2(texcoord) + ivec2(512,0), 0));
    return combineLayers(_3dpix, val1, val2, val3);
}
bool isScreenWhite(int index)
{
    return isColorWhite(getSimpleColorAtCoordinate(64, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorWhite(getSimpleColorAtCoordinate(64, index*192.0 + 192.0*(2.0/3.0))) &&
           isColorWhite(getSimpleColorAtCoordinate(128, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorWhite(getSimpleColorAtCoordinate(128, index*192.0 + 192.0*(2.0/3.0))) &&
           isColorWhite(getSimpleColorAtCoordinate(192, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorWhite(getSimpleColorAtCoordinate(192, index*192.0 + 192.0*(2.0/3.0)));
}
bool isScreenBlack(int index)
{
    return isColorBlack(getSimpleColorAtCoordinate(64, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(64, index*192.0 + 192.0*(2.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(128, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(128, index*192.0 + 192.0*(2.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(192, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(192, index*192.0 + 192.0*(2.0/3.0)));
}
bool isScreenBackgroundBlack(int index)
{
    return isColorBlack(getSimpleColorAtCoordinate(0, index*192.0 + 0)) &&
           isColorBlack(getSimpleColorAtCoordinate(0, index*192.0 + 192.0*(1.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(0, index*192.0 + 192.0*(2.0/3.0))) &&
           isColorBlack(getSimpleColorAtCoordinate(0, index*192.0 + 192.0 - 1.0));
}

vec2 getGenericHudTextureCoordinates(float xpos, float ypos)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    float sourceHeight = 192.0;
    float sourceWidth = 256.0;
    float height = sourceHeight;
    float width = sourceWidth*heightScale;
    float leftMargin = 256.0/2 - width/2;
    if (texPosition3d.x <= width + leftMargin &&
        texPosition3d.x > leftMargin)
    {
        return fixStretch*(texPosition3d - vec2(leftMargin, 0));
    }

    // nothing (clear screen)
    return vec2(0, 0);
}

vec2 getSingleSquaredScreenTextureCoordinates(float xpos, float ypos, int screenIndex, vec2 clearVect)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);
    vec2 initialScreenMargin = (screenIndex == 2 ? vec2(0, 192.0) : vec2(0, 0));

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
        return fixStretch*(texPosition3d - vec2(screenLeftMargin, screenTopMargin)) + initialScreenMargin;
    }

    // nothing (clear screen)
    return initialScreenMargin + clearVect;
}

vec2 getSingleScreenTextureCoordinates(float xpos, float ypos, int screenIndex)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    vec2 initialScreenMargin = (screenIndex == 2 ? vec2(0, 192.0) : vec2(0, 0));
    return texPosition3d + initialScreenMargin;
}

vec2 getHorizontalDualScreenTextureCoordinates(float xpos, float ypos, vec2 clearVect)
{
    int screenScale = 2;
    vec2 texPosition3d = vec2(xpos*screenScale, ypos*screenScale);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
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

vec2 getVerticalDualScreenTextureCoordinates(float xpos, float ypos, vec2 clearVect)
{
    int screenScale = 2;
    vec2 texPosition3d = vec2(xpos*screenScale, ypos*screenScale);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
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

vec2 getMissionInformationCoordinates(vec2 texPosition3d)
{
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    // mission information
    float sourceMissionInfoHeight = 40.0;
    float sourceMissionInfoWidth = 256.0;
    float missionInfoHeight = sourceMissionInfoHeight;
    float missionInfoWidth = sourceMissionInfoWidth*heightScale;
    float missionInfoY1 = 0;
    float missionInfoY2 = missionInfoHeight;
    float missionInfoDetailsLeftMargin = -5.4*heightScale;

    if (texPosition3d.x >= 0 &&
        texPosition3d.x <  missionInfoWidth &&
        texPosition3d.y >= 0 &&
        texPosition3d.y <  missionInfoY2) {
        return fixStretch*(texPosition3d);
    }

    return vec2(-1, -1);
}

vec2 getIngameHudTextureCoordinates(float xpos, float ypos)
{
    if (HideAllHUD) {
        return vec2(255, 191);
    }

    int iuScale = KHUIScale;
    float iuTexScale = (6.0)/iuScale;
    vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    if (isMissionInformationVisible()) {
        vec2 missionInfoCoords = getMissionInformationCoordinates(texPosition3d);
        if (missionInfoCoords.x != -1 && missionInfoCoords.y != -1) {
            return missionInfoCoords;
        }

        if (texPosition3d.y <= (192*iuTexScale)/3) {
            // nothing (clear screen)
            return vec2(-1, -1);
        }
    }

    if (!isHealthVisible() && !isCommandMenuVisible())
    {
        // texts over screen, like in the tutorial
        return getSingleSquaredScreenTextureCoordinates(xpos, ypos, 1, vec2(128, 190));
    }

    if (isDialogVisible()) {
        return vec2(fTexcoord);
    }

    if (ShowMap && GameScene == 5 && isMinimapVisible()) // gameScene_InGameWithMap
    {
        // minimap
        float bottomMinimapWidth = 60.0;
        float bottomMinimapHeight = 60.0;
        float minimapWidth = bottomMinimapWidth*heightScale;
        float minimapHeight = bottomMinimapHeight;
        float minimapRightMargin = 9.0;
        float minimapTopMargin = 30.0;
        float minimapLeftMargin = 256.0*iuTexScale - minimapWidth - minimapRightMargin;
        float bottomMinimapCenterX = 128.0 - 0.5;
        float bottomMinimapCenterY = 93.0 - 0.5;
        float increaseMapSize = 1.8;
        float bottomMinimapLeftMargin = bottomMinimapCenterX - (bottomMinimapWidth*increaseMapSize)/2;
        float bottomMinimapTopMargin = bottomMinimapCenterY - (bottomMinimapHeight*increaseMapSize)/2;
        if (texPosition3d.x >= minimapLeftMargin &&
            texPosition3d.x < (256.0*iuTexScale - minimapRightMargin) && 
            texPosition3d.y <= minimapHeight + minimapTopMargin && 
            texPosition3d.y >= minimapTopMargin) {
            int finalMinimapCenterX = (MinimapCenterY > 198) ? 198 : ((MinimapCenterX < 58) ? 58 : MinimapCenterX);
            int finalMinimapCenterY = (MinimapCenterY > 105) ? 105 : ((MinimapCenterY < 87) ? 87 : MinimapCenterY);
            return increaseMapSize*fixStretch*(texPosition3d - vec2(minimapLeftMargin, minimapTopMargin)) +
                vec2(0, 192.0) + vec2(bottomMinimapLeftMargin + finalMinimapCenterX - 128, bottomMinimapTopMargin + finalMinimapCenterY - 96);
        }
    }

    if (isHealthVisible())
    {
        // player health
        float sourcePlayerHealthHeight = 78.0;
        float sourcePlayerHealthWidth = 92.0;
        float playerHealthHeight = sourcePlayerHealthHeight;
        float playerHealthWidth = sourcePlayerHealthWidth*heightScale;
        float playerHealthRightMargin = 8.0;
        float playerHealthBottomMargin = 3.0;
        if (texPosition3d.x >= (256.0*iuTexScale - playerHealthWidth - playerHealthRightMargin) &&
            texPosition3d.x <= (256.0*iuTexScale - playerHealthRightMargin) &&
            texPosition3d.y >= (192.0*iuTexScale - playerHealthHeight - playerHealthBottomMargin) &&
            texPosition3d.y < (192.0*iuTexScale - playerHealthBottomMargin)) {

            vec2 finalPos = vec2((playerHealthWidth + playerHealthRightMargin) - (256.0*iuTexScale - texPosition3d.x),
                                 (playerHealthHeight + playerHealthBottomMargin) - (192.0*iuTexScale - texPosition3d.y));
            if (finalPos.x*1.7 + finalPos.y > 64.0) {
                return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - playerHealthWidth - playerHealthRightMargin,
                                                        192.0*iuTexScale - playerHealthHeight - playerHealthBottomMargin)) +
                    vec2(256.0 - sourcePlayerHealthWidth, 192.0 - sourcePlayerHealthHeight);
            }
        }
    }

    if (isCommandMenuVisible())
    {
        // command menu
        float sourceCommandMenuHeight = 84.0;
        float sourceCommandMenuWidth = 88.0;
        float commandMenuHeight = sourceCommandMenuHeight;
        float commandMenuWidth = sourceCommandMenuWidth*heightScale;
        float commandMenuLeftMargin = 10.0;
        float commandMenuBottomMargin = 0.0;
        if (texPosition3d.x >= commandMenuLeftMargin &&
            texPosition3d.x <= commandMenuWidth + commandMenuLeftMargin &&
            texPosition3d.y >= (192.0*iuTexScale - commandMenuHeight - commandMenuBottomMargin) &&
            texPosition3d.y < (192.0*iuTexScale - commandMenuBottomMargin)) {
            return fixStretch*(texPosition3d - vec2(commandMenuLeftMargin, 192.0*iuTexScale - commandMenuHeight - commandMenuBottomMargin)) +
                vec2(0, 192.0 - sourceCommandMenuHeight);
        }

        // next area name
        float sourceNextAreaNameHeight = 32.0;
        float sourceNextAreaNameWidth = 78.0;
        float nextAreaNameHeight = sourceNextAreaNameHeight;
        float nextAreaNameWidth = sourceNextAreaNameWidth*heightScale;
        float nextAreaNameRightMargin = 0.5;
        float nextAreaNameBottomMargin = 0.0;
        if (texPosition3d.x >= (128.0*iuTexScale - sourceNextAreaNameWidth*heightScale/2) &&
            texPosition3d.x <= (128.0*iuTexScale + sourceNextAreaNameWidth*heightScale/2) &&
            texPosition3d.y >= (192.0*iuTexScale - nextAreaNameHeight - nextAreaNameBottomMargin) &&
            texPosition3d.y < (192.0*iuTexScale - nextAreaNameBottomMargin)) {
            return fixStretch*(texPosition3d - vec2(128.0*iuTexScale - sourceNextAreaNameWidth*heightScale/2 + nextAreaNameRightMargin, 192.0*iuTexScale - nextAreaNameHeight - nextAreaNameBottomMargin)) +
                vec2(128.0 - sourceNextAreaNameWidth/2, 192.0 - sourceNextAreaNameHeight);
        }
    }

    // overclock notification
    float sourceOverclockNotificationHeight = 27.0;
    float sourceOverclockNotificationWidth = 95.0;
    float overclockNotificationHeight = sourceOverclockNotificationHeight;
    float overclockNotificationWidth = sourceOverclockNotificationWidth*heightScale;
    float overclockNotificationLeftMargin = 0.0;
    float overclockNotificationBottomMargin = 84.0;
    if (texPosition3d.x >= overclockNotificationLeftMargin &&
        texPosition3d.x <= overclockNotificationWidth + overclockNotificationLeftMargin &&
        texPosition3d.y >= (192.0*iuTexScale - overclockNotificationHeight - overclockNotificationBottomMargin) &&
        texPosition3d.y < (192.0*iuTexScale - overclockNotificationBottomMargin)) {
        return fixStretch*(texPosition3d - vec2(overclockNotificationLeftMargin, 192.0*iuTexScale - overclockNotificationHeight - overclockNotificationBottomMargin)) +
            vec2(0, 192.0 - sourceOverclockNotificationHeight - overclockNotificationBottomMargin);
    }

    // item notification
    float sourceItemNotificationHeight = 32.0;
    float sourceItemNotificationWidth = 95.0;
    float itemNotificationHeight = sourceItemNotificationHeight;
    float itemNotificationWidth = sourceItemNotificationWidth*heightScale;
    float itemNotificationLeftMargin = 0.0;
    float itemNotificationBottomMargin = 121.0;
    if (texPosition3d.x >= itemNotificationLeftMargin &&
        texPosition3d.x <= itemNotificationWidth + itemNotificationLeftMargin &&
        texPosition3d.y >= (192.0*iuTexScale - itemNotificationHeight - itemNotificationBottomMargin) &&
        texPosition3d.y < (192.0*iuTexScale - itemNotificationBottomMargin)) {
        return fixStretch*(texPosition3d - vec2(itemNotificationLeftMargin, 192.0*iuTexScale - itemNotificationHeight - itemNotificationBottomMargin)) +
            vec2(0, 192.0 - sourceItemNotificationHeight - itemNotificationBottomMargin);
    }

    // level up notification
    float sourceLevelNotificationHeight = 32.0;
    float sourceLevelNotificationWidth = 95.0;
    float levelNotificationHeight = sourceLevelNotificationHeight;
    float levelNotificationWidth = sourceLevelNotificationWidth*heightScale;
    float levelNotificationRightMargin = 0.0;
    float levelNotificationBottomMargin = 121.0;
    if (texPosition3d.x >= (256.0*iuTexScale - levelNotificationWidth - levelNotificationRightMargin) &&
        texPosition3d.x <= (256.0*iuTexScale - levelNotificationRightMargin) &&
        texPosition3d.y >= (192.0*iuTexScale - levelNotificationHeight - levelNotificationBottomMargin) &&
        texPosition3d.y < (192.0*iuTexScale - levelNotificationBottomMargin)) {
        return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - levelNotificationWidth - levelNotificationRightMargin, 192.0*iuTexScale - levelNotificationHeight - levelNotificationBottomMargin)) +
            vec2(256.0 - sourceLevelNotificationWidth, 192.0 - sourceLevelNotificationHeight - levelNotificationBottomMargin);
    }

    // enemy health
    float sourceEnemyHealthHeight = 22.0;
    float sourceEnemyHealthWidth = 93.0;
    float enemyHealthHeight = sourceEnemyHealthHeight;
    float enemyHealthWidth = sourceEnemyHealthWidth*heightScale;
    float enemyHealthRightMargin = 9.0;
    float enemyHealthTopMargin = 7.5;
    if (texPosition3d.x >= (256.0*iuTexScale - enemyHealthWidth - enemyHealthRightMargin) &&
        texPosition3d.x < (256.0*iuTexScale - enemyHealthRightMargin) && 
        texPosition3d.y <= enemyHealthHeight + enemyHealthTopMargin && 
        texPosition3d.y >= enemyHealthTopMargin - 1) {
        return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - enemyHealthWidth - enemyHealthRightMargin, enemyHealthTopMargin)) + 
            vec2(256.0 - sourceEnemyHealthWidth, 0);
    }

    // nothing (clear screen)
    return vec2(-1, -1);
}

vec2 getPauseHudTextureCoordinates(float xpos, float ypos)
{
    int iuScale = KHUIScale;
    float iuTexScale = (6.0)/iuScale;
    vec2 texPosition3d = vec2(vec2(xpos, ypos)*iuTexScale);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
    vec2 fixStretch = vec2(widthScale, 1.0);

    if (GameScene == 8) // gameScene_PauseMenu
    {
        if (!isScreenBlack(1))
        {
            // mission gauge
            float sourceMissionGaugeHeight = 39.0;
            float sourceMissionGaugeWidth = 246.0;
            float missionGaugeHeight = sourceMissionGaugeHeight;
            float missionGaugeWidth = sourceMissionGaugeWidth*heightScale;
            float missionGaugeRightMargin = (256.0*iuTexScale - missionGaugeWidth)/2;
            float bottomMissionGaugeCenterX = 128.0;
            float bottomMissionGaugeCenterY = 172.5;
            float bottomMissionGaugeLeftMargin = bottomMissionGaugeCenterX - sourceMissionGaugeWidth/2;
            float bottomMissionGaugeTopMargin = bottomMissionGaugeCenterY - sourceMissionGaugeHeight/2;
            if (texPosition3d.x >= (256.0*iuTexScale - missionGaugeWidth - missionGaugeRightMargin) &&
                texPosition3d.x <= (256.0*iuTexScale - missionGaugeRightMargin) &&
                texPosition3d.y >= (192.0*iuTexScale - missionGaugeHeight) &&
                texPosition3d.y < (192.0*iuTexScale)) {

                vec2 finalPos = fixStretch*(texPosition3d - vec2(256.0*iuTexScale - missionGaugeWidth - missionGaugeRightMargin, 192.0*iuTexScale - missionGaugeHeight)) +
                        vec2(0, 192.0) + vec2(bottomMissionGaugeLeftMargin, bottomMissionGaugeTopMargin);
                if (finalPos.x + finalPos.y > 355.0 && finalPos.y - finalPos.x > 99.0)
                {
                    return finalPos;
                }
            }
        }
    }

    // pause menu
    float height = 192.0;
    float width = 256.0;
    float x1 = (256.0*iuTexScale)/2 - width/2;
    float x2 = (256.0*iuTexScale)/2 + width/2;
    float y1 = (192.0*iuTexScale)/2 - height/2;
    float y2 = (192.0*iuTexScale)/2 + height/2;
    if (texPosition3d.x >= x1 && texPosition3d.x < x2 && texPosition3d.y >= y1 && texPosition3d.y < y2)
    {
        return texPosition3d - vec2(x1, y1);
    }

    // nothing (clear screen)
    return vec2(0, 0);
}

ivec2 getCutsceneTextureCoordinates(float xpos, float ypos)
{
    if ((DSCutsceneState & 2) == 2 && (DSCutsceneState & 1) == 0) { // top only
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 1, vec2(-1, -1)));
    }
    return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(-1, 0)));
}

ivec2 getTopScreenTextureCoordinates(float xpos, float ypos)
{
    if (GameScene == 0) { // gameScene_Intro
        return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(0, 0)));
    }
    if (GameScene == 1) { // gameScene_MainMenu
        return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(0, 0)));
    }
    if (GameScene == 2) { // gameScene_IntroLoadMenu
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 2, vec2(255, 191)));
    }
    if (GameScene == 3) { // gameScene_DayCounter
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 1, vec2(0, 0)));
    }
    if (GameScene == 4) { // gameScene_Cutscene
        return ivec2(getCutsceneTextureCoordinates(xpos, ypos));
    }
    if (GameScene == 5) { // gameScene_InGameWithMap
        return ivec2(getIngameHudTextureCoordinates(xpos, ypos));
    }
    if (GameScene == 6) { // gameScene_InGameMenu
        return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(128, 191)));
    }
    if (GameScene == 8) { // gameScene_PauseMenu
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 1, vec2(0, 0)));
    }
    if (GameScene == 9) { // gameScene_Tutorial
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 2, vec2(0, 0)));
    }
    if (GameScene == 10) { // gameScene_InGameWithCutscene
        if (!is2DGraphicDifferentFromColor(ivec4(0,63,0,31), ivec2(130, 190))) {
            return ivec2(getIngameHudTextureCoordinates(xpos, ypos));
        }
        return ivec2(-1, -1);
    }
    if (GameScene == 11) { // gameScene_Shop
        return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(128, 190)));
    }
    if (GameScene == 13) { // gameScene_CutsceneWithStaticImages
        return ivec2(getSingleSquaredScreenTextureCoordinates(xpos, ypos, 1, vec2(0, 0)));
    }
    if (GameScene == 14) { // gameScene_WorldSelection
        return ivec2(getHorizontalDualScreenTextureCoordinates(xpos, ypos, vec2(1, 1)));
    }
    if (GameScene == 15) { // gameScene_Other2D
        return ivec2(getCutsceneTextureCoordinates(xpos, ypos));
    }
    return ivec2(fTexcoord);
}

ivec4 getSingleSquaredScreen3DColor(float xpos, float ypos)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
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
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
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
    float heightScale = 1.0/TopScreenAspectRatio;
    float widthScale = TopScreenAspectRatio;
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
    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    float _3dxpos = float(mbright.a - ((mbright.b & 0x80) * 2));

    float xpos = fTexcoord.x + _3dxpos;
    float ypos = mod(fTexcoord.y, 192);

    ivec2 position3d = ivec2(vec2(xpos, ypos)*u3DScale);
    ivec4 _3dpix = ivec4(texelFetch(_3DTex, position3d, 0).bgra
                * vec4(63,63,63,31));

    if (GameScene == 1) { // gameScene_MainMenu
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (GameScene == 3) { // gameScene_DayCounter
        return getSingleSquaredScreen3DColor(xpos, ypos);
    }
    if (GameScene == 4) { // gameScene_Cutscene
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (GameScene == 6) { // gameScene_InGameMenu
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (GameScene == 11) { // gameScene_Shop
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (GameScene == 13) { // gameScene_CutsceneWithStaticImages
        return getSingleSquaredScreen3DColor(xpos, ypos);
    }
    if (GameScene == 14) { // gameScene_WorldSelection
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }
    if (GameScene == 15) { // gameScene_Other2D
        return getHorizontalDualScreen3DColor(xpos, ypos);
    }

    return _3dpix;
}

ivec4 getTopScreenColor(float xpos, float ypos, int index)
{
    ivec2 textureBeginning = getTopScreenTextureCoordinates(xpos, ypos);
    ivec2 coordinates = textureBeginning + ivec2(256,0)*index;
    ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));

    if (ShowMap && GameScene == 5 && isMinimapVisible()) // gameScene_InGameWithMap
    {
        if (!isHealthVisible() && !isCommandMenuVisible()) {
            return color;
        }

        if (!isDialogVisible() && !isMissionInformationVisible())
        {
            int iuScale = KHUIScale;
            float iuTexScale = (6.0)/iuScale;
            vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
            float heightScale = 1.0/TopScreenAspectRatio;
            float widthScale = TopScreenAspectRatio;

            // minimap
            float bottomMinimapWidth = 60.0;
            float bottomMinimapHeight = 60.0;
            float minimapWidth = bottomMinimapWidth*heightScale;
            float minimapHeight = bottomMinimapHeight;
            float minimapRightMargin = 9.0;
            float minimapTopMargin = 30.0;
            float minimapLeftMargin = 256.0*iuTexScale - minimapWidth - minimapRightMargin;
            float blurBorder = 5.0;
            if (texPosition3d.x >= minimapLeftMargin &&
                texPosition3d.x < (256.0*iuTexScale - minimapRightMargin) && 
                texPosition3d.y <= minimapHeight + minimapTopMargin && 
                texPosition3d.y >= minimapTopMargin) {

                if (index == 2)
                {
                    int xBlur = 16;
                    if (texPosition3d.x - minimapLeftMargin <= (blurBorder*heightScale)) {
                        xBlur = int(((texPosition3d.x - minimapLeftMargin)*16.0)/(blurBorder*heightScale));
                    }
                    if ((256.0*iuTexScale - minimapRightMargin) - texPosition3d.x <= (blurBorder*heightScale)) {
                        xBlur = int((((256.0*iuTexScale - minimapRightMargin) - texPosition3d.x)*16.0)/(blurBorder*heightScale));
                    }

                    int yBlur = 16;
                    if (texPosition3d.y - minimapTopMargin <= blurBorder) {
                        yBlur = int(((texPosition3d.y - minimapTopMargin)*16.0)/blurBorder);
                    }
                    if ((minimapHeight + minimapTopMargin) - texPosition3d.y <= blurBorder) {
                        yBlur = int((((minimapHeight + minimapTopMargin) - texPosition3d.y)*16.0)/blurBorder);
                    }

                    float transparency = 15.0/16;
                    int blur = int((xBlur * yBlur * transparency)/16);
                    color = ivec4(color.r, blur, 16 - blur, 0x01);
                }
            }
        }
    }

    return color;
}

ivec4 brightness()
{
    if (fTexcoord.y >= 192) {
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }

    if (GameScene == 1) { // gameScene_MainMenu
        ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
        int brightmode = mbright.g >> 6;
        if ((mbright.b & 0x3) != 0 && brightmode == 2) {
            return mbright;
        }
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }
    if (GameScene == 4) { // gameScene_Cutscene
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }
    if (GameScene == 5) { // gameScene_InGameWithMap
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }
    if (GameScene == 6) { // gameScene_InGameMenu
        ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
        int brightmode = mbright.g >> 6;
        if ((mbright.b & 0x3) != 0 && brightmode == 2) {
            return mbright;
        }
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }
    if (GameScene == 8) { // gameScene_PauseMenu
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }
    if (GameScene == 9) { // gameScene_Tutorial
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
    }
    if (GameScene == 10) { // gameScene_InGameWithCutscene
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }
    if (GameScene == 11) { // gameScene_Shop
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    }
    if (GameScene == 13) { // gameScene_CutsceneWithStaticImages
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }
    if (GameScene == 14) { // gameScene_WorldSelection
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
    }
    if (GameScene == 15) { // gameScene_Other2D
        return ivec4(texelFetch(ScreenTex, ivec2(256*3, 0), 0));
    }

    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, 192), 0));
    int brightmode = mbright.g >> 6;
    if (brightmode != 0) {
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

        ivec4 val1 = pixel;
        ivec4 val2 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(256,0), 0));
        ivec4 val3 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(512,0), 0));

        if (fTexcoord.y <= 192)
        {
            val1 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 0);
            val2 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 1);
            val3 = getTopScreenColor(fTexcoord.x, fTexcoord.y, 2);
        }

        pixel = combineLayers(_3dpix, val1, val2, val3);
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

    // TODO: filters

    oColor = vec4(vec3(pixel.bgr) / 255.0, 1.0);
}
)";

}

#endif // KHRECODED_GPU_OPENGL_SHADERS_H
