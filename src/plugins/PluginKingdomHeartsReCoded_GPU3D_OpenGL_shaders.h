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

#ifndef KHRECODED_GPU3D_OPENGL_SHADERS_H
#define KHRECODED_GPU3D_OPENGL_SHADERS_H

namespace Plugins
{
const char* kRenderVS_Z_KhReCoded = R"(

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

    float widthScale = TopScreenAspectRatio;
    int iuScale = KHUIScale;
    float iuTexScale = (5.0)/iuScale;

    float commandMenuLeftMargin = 33.5;
    float commandMenuBottomMargin = 2.5;
    float commandMenuWidth = (256.0*u3DScale)/(2.4*widthScale);
    float commandMenuHeight = (192.0*u3DScale)/2.4;
    if (fpos.x >= -(1.00)*fpos.w && fpos.x <= -(0.375)*fpos.w &&
        fpos.y >= -(0.25)*fpos.w && fpos.y <= +(1.00)*fpos.w &&
        fpos.z == -1.0*fpos.w &&
        vColor.r < 200.0) {
        fpos.x = ((((fpos.x/fpos.w + 1.0)*(commandMenuWidth/iuTexScale) + commandMenuLeftMargin/iuTexScale)/uScreenSize.x)*2.0 - 1.0)*fpos.w;
        fpos.y = (1.0 - (((1.0 - fpos.y/fpos.w)*(commandMenuHeight/iuTexScale) + commandMenuBottomMargin/iuTexScale)/uScreenSize.y)*2.0)*fpos.w;
    }

    fColor = vec4(vColor) / vec4(255.0,255.0,255.0,31.0);
    fTexcoord = vec2(vTexcoord) / 16.0;
    fPolygonAttr = vPolygonAttr;

    gl_Position = fpos;
}
)";

}

#endif // KHRECODED_GPU3D_OPENGL_SHADERS_H
