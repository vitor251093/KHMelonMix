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

#ifndef KHDAYS_GPU3D_OPENGL_SHADERS_H
#define KHDAYS_GPU3D_OPENGL_SHADERS_H

namespace Plugins
{
const char* kRenderVS_Z_KhDays = R"(

uniform int GameScene;
uniform int KHUIScale;
uniform float TopScreenAspectRatio;
uniform bool ShowMissionInfo;
uniform bool HideAllHUD;

void main()
{
    int attr = vPolygonAttr.x;
    int zshift = (attr >> 16) & 0x1F;

    vec4 fpos;
    float u3DScale = uScreenSize.x/256.0;
    fpos.xy = (((vec2(vPosition.xy) ) * 2.0) / uScreenSize) - 1.0;
    fpos.z = (float(vPosition.z << zshift) / 8388608.0) - 1.0;
    fpos.w = float(vPosition.w) / 65536.0f;
    fpos.xyz *= fpos.w;

    if (HideAllHUD)
    {
        // gameScene_InGameWithMap or gameScene_PauseMenu or gameScene_InGameWithDouble3D
        if (GameScene == 5 || GameScene == 7 || GameScene == 9)
        {
            if (fpos.x >= -(1.00)*fpos.w && fpos.x <= +(1.00)*fpos.w &&
                fpos.y >= -(1.00)*fpos.w && fpos.y <= +(1.00)*fpos.w &&
                fpos.z >= -(1.00)*fpos.w && fpos.z <= -(0.30)*fpos.w) {
                fpos.x = (0 - 1.0)*fpos.w;
                fpos.y = (0 - 1.0)*fpos.w;
            }
        }
    }
    else 
    {
        if (GameScene == 5) // gameScene_InGameWithMap
        {
            float effectLayer = -0.300; // blue shine behind the heart counter and "CHAIN" label
            float textLayer = -0.900; // heart counter, timer, "BONUS" label and +X floating labels

            float widthScale = TopScreenAspectRatio;
            int iuScale = KHUIScale;
            float iuTexScale = (5.0)/iuScale;

            float heartTopMargin = (ShowMissionInfo ? 20.0 : 2.0)*u3DScale;
            float heartWidth = (256.0*u3DScale*9)/20.0;
            float heartHeight = (192.0*u3DScale)/2.5;
            if ((fpos.x >= -(1.000)*fpos.w && fpos.x <= -(0.000)*fpos.w &&
                fpos.y >= -(1.000)*fpos.w && fpos.y <= -(0.200)*fpos.w &&
                (abs(fpos.z - effectLayer * fpos.w) < 0.01))
                ||
                (fpos.x >= -(1.000)*fpos.w && fpos.x <= -(0.200)*fpos.w &&
                fpos.y >= -(1.000)*fpos.w && fpos.y <= -(0.500)*fpos.w &&
                (abs(fpos.z - textLayer * fpos.w) < 0.01))) {
                fpos.x = ((((fpos.x/fpos.w + 1.0)*(heartWidth/iuTexScale))/uScreenSize.x)*2.0/widthScale - 1.0)*fpos.w;
                fpos.y = ((((fpos.y/fpos.w + 1.0)*(heartHeight/iuTexScale) + heartTopMargin/iuTexScale)/uScreenSize.y)*2.0 - 1.0)*fpos.w;
            }
        }

        if (GameScene == 7) // gameScene_PauseMenu
        {
            if (fpos.x >= -(1.00)*fpos.w && fpos.x <= -(0.000)*fpos.w &&
                fpos.y >= -(1.00)*fpos.w && fpos.y <= -(0.500)*fpos.w &&
                fpos.z <  -(0.30)*fpos.w && fpos.z >= -(0.900)*fpos.w) {
                fpos.x = (0 - 1.0)*fpos.w;
                fpos.y = (0 - 1.0)*fpos.w;
            }
        }
    }

    fColor = vec4(vColor) / vec4(255.0,255.0,255.0,31.0);
    fTexcoord = vec2(vTexcoord) / 16.0;
    fPolygonAttr = vPolygonAttr;

    gl_Position = fpos;
}
)";

}

#endif // KHDAYS_GPU3D_OPENGL_SHADERS_H
