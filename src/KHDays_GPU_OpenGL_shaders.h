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

#ifndef KHDAYS_GPU_OPENGL_SHADERS_H
#define KHDAYS_GPU_OPENGL_SHADERS_H

namespace melonDS
{
const char* kCompositorFS_KhDays = R"(#version 140

uniform uint u3DScale;
uniform int u3DXPos;
uniform int KHGameScene;
uniform int KHUIScale;
uniform float TopScreenAspectRatio;

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

bool isMissionInformationVisible()
{
    ivec4 missionInfoTopLeft = ivec4(texelFetch(ScreenTex, ivec2(0, 0) + ivec2(512,0), 0));
    ivec4 missionInfoMiddleLeft = ivec4(texelFetch(ScreenTex, ivec2(0, 192*0.078) + ivec2(512,0), 0));
    ivec4 missionInfoMiddleRight = ivec4(texelFetch(ScreenTex, ivec2(255.0 - 1.0, 192*0.078) + ivec2(512,0), 0));
    return (missionInfoTopLeft.a & 0xF) == 1 || (missionInfoMiddleLeft.a & 0xF) == 1 || (missionInfoMiddleRight.a & 0xF) == 1;
}

bool isCutsceneFromChallengeMissionVisible()
{
    return is2DGraphicDifferentFromColor(ivec4(63,0,0,31), ivec2(256/4, 0));
}

bool isDialogVisible()
{
    return is2DGraphicDifferentFromColor(ivec4(0,0,0,31), ivec2(256/2, 192*0.809));
}

bool isMinimapVisible()
{
    ivec4 minimapSecurityPixel = ivec4(texelFetch(ScreenTex, ivec2(99, 53) + ivec2(0,192), 0));
    return minimapSecurityPixel.r > 60 && minimapSecurityPixel.g > 60 && minimapSecurityPixel.b > 60;
}

bool isColorBlack(ivec4 pixel)
{
    return pixel.r < 5 && pixel.g < 5 && pixel.b < 5;
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
bool isScreenBackgroundBlack(int index)
{
    ivec4 pixel1 = getSimpleColorAtCoordinate(0, index*192.0 + 0);
    ivec4 pixel2 = getSimpleColorAtCoordinate(0, index*192.0 + 192.0*(1.0/3.0));
    ivec4 pixel3 = getSimpleColorAtCoordinate(0, index*192.0 + 192.0*(2.0/3.0));
    ivec4 pixel4 = getSimpleColorAtCoordinate(0, index*192.0 + 192.0 - 1.0);
    return isColorBlack(pixel1) && isColorBlack(pixel2) && isColorBlack(pixel3) && isColorBlack(pixel4);
}

vec2 getGenericHudTextureCoordinates(float xpos, float ypos)
{
    vec2 texPosition3d = vec2(xpos, ypos);
    float heightScale = (4.0/3)/TopScreenAspectRatio;
    float widthScale = 1.0/heightScale;
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

vec2 getMainMenuHudTextureCoordinates(float xpos, float ypos)
{
    int iuScale = KHUIScale;
    float iuTexScale = (6.0)/iuScale;
    vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
    float heightScale = (4.0/3)/TopScreenAspectRatio;
    float widthScale = 1.0/heightScale;
    vec2 fixStretch = vec2(widthScale, 1.0);

    // logo
    float logoScale = 1.2;
    float bottomLogoHeight = 192.0;
    float bottomLogoWidth = 256.0;
    float bottomLogoLeftMargin = 0;
    float bottomLogoTopMargin = 0;
    float logoHeight = bottomLogoHeight/logoScale;
    float logoWidth = (bottomLogoWidth/logoScale)*heightScale;
    float logoLeftMargin = 0.0;
    float logoTopMargin = 0.0;
    if (texPosition3d.x >= logoLeftMargin &&
        texPosition3d.x < (logoWidth + logoLeftMargin) && 
        texPosition3d.y <= (logoHeight + logoTopMargin) && 
        texPosition3d.y >= logoTopMargin) {
        return logoScale*fixStretch*(texPosition3d - vec2(logoLeftMargin, logoTopMargin)) +
            vec2(bottomLogoLeftMargin, bottomLogoTopMargin);
    }

    // menu options
    float bottomMenuOptionsHeight = 76.0;
    float bottomMenuOptionsWidth = 128.0;
    float bottomMenuOptionsLeftMargin = 0;
    float bottomMenuOptionsBottomMargin = 12.0;
    float menuOptionsHeight = bottomMenuOptionsHeight;
    float menuOptionsWidth = bottomMenuOptionsWidth*heightScale;
    float menuOptionsLeftMargin = 0.0;
    float menuOptionsBottomMargin = 0.0;
    float menuOptionsTopMargin = 192.0*iuTexScale - menuOptionsHeight - menuOptionsBottomMargin;
    if (texPosition3d.x >= menuOptionsLeftMargin &&
        texPosition3d.x < (menuOptionsWidth + menuOptionsLeftMargin) && 
        texPosition3d.y <= (menuOptionsHeight + menuOptionsTopMargin) && 
        texPosition3d.y >= menuOptionsTopMargin) {
        return fixStretch*(texPosition3d - vec2(menuOptionsLeftMargin, menuOptionsTopMargin)) +
            vec2(0, 192.0) + vec2(bottomMenuOptionsLeftMargin, 192.0 - bottomMenuOptionsHeight - bottomMenuOptionsBottomMargin);
    }

    // nothing (clear screen)
    return vec2(0, 0);
}

vec2 getIngameHudTextureCoordinates(float xpos, float ypos)
{
    int iuScale = KHUIScale;
    float iuTexScale = (6.0)/iuScale;
    vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
    float heightScale = (4.0/3)/TopScreenAspectRatio;
    float widthScale = 1.0/heightScale;
    vec2 fixStretch = vec2(widthScale, 1.0);

    if (isDialogVisible()) {
        return vec2(fTexcoord);
    }

    if (isMissionInformationVisible()) {
        return vec2(fTexcoord);
    }

    if (isCutsceneFromChallengeMissionVisible()) {
        return vec2(fTexcoord);
    }

    if (isScreenBackgroundBlack(0) && isScreenBackgroundBlack(1)) {
        return getGenericHudTextureCoordinates(xpos, ypos);
    }

    // item notification
    float sourceItemNotificationHeight = 86.0;
    float sourceItemNotificationWidth = 108.0;
    float itemNotificationHeight = sourceItemNotificationHeight;
    float itemNotificationWidth = sourceItemNotificationWidth*heightScale;
    float itemNotificationLeftMargin = 0.0;
    float itemNotificationTopMargin = 15.0*iuTexScale;
    if (texPosition3d.x <= itemNotificationWidth + itemNotificationLeftMargin &&
        texPosition3d.x > itemNotificationLeftMargin &&
        texPosition3d.y <= itemNotificationHeight + itemNotificationTopMargin &&
        texPosition3d.y >= itemNotificationTopMargin) {
        return fixStretch*(texPosition3d - vec2(itemNotificationLeftMargin, itemNotificationTopMargin));
    }

    // countdown and locked on
    float sourceCountdownHeight = 20.0;
    float sourceCountdownWidth = 70.0;
    float countdownHeight = sourceCountdownHeight;
    float countdownWidth = sourceCountdownWidth*heightScale;
    float countdownRightMargin = (256.0*iuTexScale - countdownWidth)/2;
    float countdownTopMargin = 9.0;
    if (texPosition3d.x >= (256.0*iuTexScale - countdownWidth - countdownRightMargin) &&
        texPosition3d.x < (256.0*iuTexScale - countdownRightMargin) && 
        texPosition3d.y <= countdownHeight + countdownTopMargin && 
        texPosition3d.y >= countdownTopMargin) {
        return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - countdownWidth - countdownRightMargin, countdownTopMargin)) + 
            vec2(128.0 - sourceCountdownWidth/2, 0);
    }

    if (KHGameScene == 7 && isMinimapVisible()) // gameScene_InGameWithMap
    {
        // minimap
        float bottomMinimapHeight = 50.0;
        float bottomMinimapWidth = 60.0;
        float minimapHeight = bottomMinimapHeight;
        float minimapWidth = bottomMinimapWidth*heightScale;
        float minimapRightMargin = 9.0;
        float minimapTopMargin = 30.0;
        float minimapLeftMargin = 256.0*iuTexScale - minimapWidth - minimapRightMargin;
        float bottomMinimapCenterX = 158.0;
        float bottomMinimapCenterY = 95.0;
        float bottomMinimapLeftMargin = bottomMinimapCenterX - bottomMinimapWidth/2;
        float bottomMinimapTopMargin = bottomMinimapCenterY - bottomMinimapHeight/2;
        float increaseMapSize = 1.2;
        if (texPosition3d.x >= minimapLeftMargin &&
            texPosition3d.x < (256.0*iuTexScale - minimapRightMargin) && 
            texPosition3d.y <= minimapHeight + minimapTopMargin && 
            texPosition3d.y >= minimapTopMargin) {
            return increaseMapSize*fixStretch*(texPosition3d - vec2(minimapLeftMargin, minimapTopMargin)) +
                vec2(0, 192.0) + vec2(bottomMinimapLeftMargin, bottomMinimapTopMargin);
        }
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

    // sigils and death counter
    float sourceMiscHeight = 30.0;
    float sourceMiscWidth = 93.0;
    float miscHeight = sourceMiscHeight;
    float miscWidth = sourceMiscWidth*heightScale;
    float miscRightMargin = 12.0;
    float miscTopMargin = 82.5;
    float sourceMiscTopMargin = 25.0;
    if (texPosition3d.x >= (256.0*iuTexScale - miscWidth - miscRightMargin) &&
        texPosition3d.x < (256.0*iuTexScale - miscRightMargin) && 
        texPosition3d.y <= miscHeight + miscTopMargin && 
        texPosition3d.y >= miscTopMargin) {
        return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - miscWidth - miscRightMargin, miscTopMargin)) + 
            vec2(256.0 - sourceMiscWidth, sourceMiscTopMargin);
    }

    // command menu
    float sourceCommandMenuHeight = 106.0;
    float sourceCommandMenuWidth = 108.0;
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

    // player health
    float sourcePlayerHealthHeight = 128.0;
    float sourcePlayerHealthWidth = 108.0;
    float playerHealthHeight = sourcePlayerHealthHeight;
    float playerHealthWidth = sourcePlayerHealthWidth*heightScale;
    float playerHealthRightMargin = 8.0;
    float playerHealthBottomMargin = 3.0;
    if (texPosition3d.x >= (256.0*iuTexScale - playerHealthWidth - playerHealthRightMargin) &&
        texPosition3d.x <= (256.0*iuTexScale - playerHealthRightMargin) &&
        texPosition3d.y >= (192.0*iuTexScale - playerHealthHeight - playerHealthBottomMargin) &&
        texPosition3d.y < (192.0*iuTexScale - playerHealthBottomMargin)) {
        return fixStretch*(texPosition3d - vec2(256.0*iuTexScale - playerHealthWidth - playerHealthRightMargin, 192.0*iuTexScale - playerHealthHeight - playerHealthBottomMargin)) +
            vec2(256.0 - sourcePlayerHealthWidth, 192.0 - sourcePlayerHealthHeight);
    }

    // nothing (clear screen)
    return vec2(255, 191);
}

ivec2 getPauseHudTextureCoordinates(float xpos, float ypos)
{
    int iuScale = KHUIScale;
    float iuTexScale = (6.0)/iuScale;
    ivec2 texPosition3d = ivec2(vec2(xpos, ypos)*iuTexScale);

    if (KHGameScene == 13) // gameScene_PauseMenuWithGauge
    {
        // gauge bar
        float gaugeBarHeight = 33.0*iuTexScale;
        if (texPosition3d.y >= (192.0*iuTexScale - gaugeBarHeight)) {
            return ivec2(fTexcoord) + ivec2(0,192);
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
        return texPosition3d - ivec2(x1, y1);
    }

    // nothing (clear screen)
    return ivec2(0, 0);
}

ivec2 getTopScreenTextureCoordinates(float xpos, float ypos)
{
    if (KHGameScene == 1) // gameScene_MainMenu
    {
        return ivec2(getMainMenuHudTextureCoordinates(xpos, ypos));
    }
    if (KHGameScene == 7 || KHGameScene == 8) // gameScene_InGameWithMap or gameScene_InGameWithoutMap
    {
        return ivec2(getIngameHudTextureCoordinates(xpos, ypos));
    }
    if (KHGameScene == 12 || KHGameScene == 13) // gameScene_PauseMenu or gameScene_PauseMenuWithGauge
    {
        return getPauseHudTextureCoordinates(xpos, ypos);
    }
    return ivec2(fTexcoord);
}

ivec4 getTopScreen3DColor(float xpos, float ypos)
{
    ivec2 position3d = ivec2(vec2(xpos, ypos)*u3DScale);
    ivec4 _3dpix = ivec4(texelFetch(_3DTex, position3d, 0).bgra
                * vec4(63,63,63,31));

    if (KHGameScene == 1) {
        int iuScale = KHUIScale;
        float iuTexScale = (6.0)/iuScale;
        vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
        float heightScale = (4.0/3)/TopScreenAspectRatio;
        float widthScale = 1.0/heightScale;
        vec2 fixStretch = vec2(widthScale, 1.0);

        // logo
        float logoScale = 1.2;
        float bottomLogoHeight = 192.0;
        float bottomLogoWidth = 256.0;
        float bottomLogoLeftMargin = 0;
        float bottomLogoTopMargin = 0;
        float logoHeight = bottomLogoHeight/logoScale;
        float logoWidth = (bottomLogoWidth/logoScale)*heightScale;
        float logoLeftMargin = 0.0;
        float logoTopMargin = 0.0;
        if (texPosition3d.x >= logoLeftMargin &&
            texPosition3d.x < (logoWidth + logoLeftMargin) && 
            texPosition3d.y <= (logoHeight + logoTopMargin) && 
            texPosition3d.y >= logoTopMargin) {
            position3d = ivec2((logoScale*fixStretch*(texPosition3d - vec2(logoLeftMargin, logoTopMargin)) +
                vec2(bottomLogoLeftMargin, bottomLogoTopMargin))*u3DScale);
            _3dpix = ivec4(texelFetch(_3DTex, position3d, 0).bgra
                * vec4(63,63,63,31));
        }
        else {
            _3dpix = ivec4(63,63,63,0);
        }
    }

    return _3dpix;
}

ivec4 getTopScreenColor(float xpos, float ypos, int index)
{
    ivec2 textureBeginning = getTopScreenTextureCoordinates(xpos, ypos);
    ivec2 coordinates = textureBeginning + ivec2(256,0)*index;
    ivec4 color = ivec4(texelFetch(ScreenTex, coordinates, 0));

    if (KHGameScene == 7 && isMinimapVisible() && !isMissionInformationVisible()) // gameScene_InGameWithMap
    {
        int iuScale = KHUIScale;
        float iuTexScale = (6.0)/iuScale;
        vec2 texPosition3d = vec2(xpos, ypos)*iuTexScale;
        float heightScale = (4.0/3)/TopScreenAspectRatio;
        float widthScale = 1.0/heightScale;

        // minimap
        float bottomMinimapWidth = 60.0;
        float bottomMinimapHeight = 50.0;
        float minimapWidth = bottomMinimapWidth*heightScale;
        float minimapHeight = bottomMinimapHeight;
        float minimapRightMargin = 9.0;
        float minimapTopMargin = 30.0;
        float minimapLeftMargin = 256.0*iuTexScale - minimapWidth - minimapRightMargin;
        if (texPosition3d.x >= minimapLeftMargin &&
            texPosition3d.x < (256.0*iuTexScale - minimapRightMargin) && 
            texPosition3d.y <= minimapHeight + minimapTopMargin && 
            texPosition3d.y >= minimapTopMargin) {

            bool isShadeOfGray = (abs(color.r - color.g) < 5) && (abs(color.r - color.b) < 5) && (abs(color.g - color.b) < 5);
            if (isShadeOfGray) {
                color = ivec4(64 - color.r, 64 - color.g, 64 - color.b, color.a);
            }
        }
    }

    return color;
}

void main()
{
    ivec4 pixel = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord), 0));

    float _3dxpos = float(u3DXPos);
    float xpos = fTexcoord.x + _3dxpos;
    float ypos = mod(fTexcoord.y, 192);

    ivec4 mbright = ivec4(texelFetch(ScreenTex, ivec2(256*3, int(fTexcoord.y)), 0));
    int dispmode = mbright.b & 0x3;

    if (dispmode == 1)
    {
        ivec4 _3dpix = getTopScreen3DColor(xpos, ypos);

        ivec4 val1 = pixel;
        ivec4 val2 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(256,0), 0));
        ivec4 val3 = ivec4(texelFetch(ScreenTex, ivec2(fTexcoord) + ivec2(512,0), 0));

        if (fTexcoord.y <= 192)
        {
            val1 = getTopScreenColor(xpos, ypos, 0);
            val2 = getTopScreenColor(xpos, ypos, 1);
            val3 = getTopScreenColor(xpos, ypos, 2);
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

#endif // KHDAYS_GPU_OPENGL_SHADERS_H
