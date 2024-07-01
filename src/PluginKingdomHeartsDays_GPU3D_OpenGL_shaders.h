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

    if (GameScene == 5 || GameScene == 6) // gameScene_InGameWithMap and gameScene_InGameWithoutMap
    {
        float aspectRatio = TopScreenAspectRatio/(4.0/3.0);
        int iuScale = KHUIScale;
        float iuTexScale = (4.0)/iuScale;
        float heartWidth = (256.0*u3DScale*9)/20.0;
        float heartHeight = (192.0*u3DScale)/2.5;
        if (fpos.x >= -(1.00)*fpos.w && fpos.x <= -(0.500)*fpos.w &&
            fpos.y >= -(1.00)*fpos.w && fpos.y <= -(0.666)*fpos.w &&
            fpos.z < -0.80*fpos.w) {
            fpos.x = ((((fpos.x/fpos.w + 1.0)*(heartWidth/(iuTexScale*aspectRatio)))/uScreenSize.x)*2.0 - 1.0)*fpos.w;
            fpos.y = ((((fpos.y/fpos.w + 1.0)*(heartHeight/iuTexScale))/uScreenSize.y)*2.0 - 1.0)*fpos.w;
        }
    }

    if (GameScene == 10) // gameScene_PauseMenu
    {
        float heartWidth = (256.0*u3DScale)/2.5;
        float heartHeight = (192.0*u3DScale)/2.5;
        if (fpos.x >= -(1.00)*fpos.w && fpos.x <= -(0.500)*fpos.w &&
            fpos.y >= -(1.00)*fpos.w && fpos.y <= -(0.666)*fpos.w &&
            fpos.z < -0.80*fpos.w) {
            fpos.x = (0 - 1.0)*fpos.w;
            fpos.y = (0 - 1.0)*fpos.w;
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
