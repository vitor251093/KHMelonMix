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

#ifndef PLUGIN_GPU3D_OPENGL_SHADERS_H
#define PLUGIN_GPU3D_OPENGL_SHADERS_H

namespace Plugins
{
// language=GLSL
const char* kRenderVS_Z_Plugin = R"(

#define SCREEN_SCALE 6.0

#define SHAPES_DATA_ARRAY_SIZE 32

struct ShapeData3D {
    vec2 sourceScale;

    int corner;
    float hudScale;

    ivec4 squareInitialCoords;

    vec4 margin;

    vec2 zRange;
    int polygonVertexesCount;

    int effects;

    uint polygonAttributes[4];
    uint negatedPolygonAttributes[4];

    int color[4];
    int negatedColor[4];

    uint textureParams[4];
    uint negatedTextureParams[4];

    int colorCount;
    int negatedColorCount;
    int textureParamCount;
    int negatedTextureParamCount;
};

layout(std140) uniform ShapeBlock3D {
    ShapeData3D shapes[SHAPES_DATA_ARRAY_SIZE];
};

uniform int shapeCount;

uniform int hudScale;
uniform float currentAspectRatio;

void main()
{
    int attr = vPolygonAttr.x;
    int zshift = (attr >> 16) & 0xFFFF;

    vec4 fpos;
    fpos.xy = vec2(vPosition.xy);
    fpos.z = (float(vPosition.z << zshift) / 0xFFFFFF)*2 - 1.0;
    fpos.w = float(vPosition.w) / 65536.0f;

    int ScreenWidth = int(uScreenSize.x);
    int ScreenHeight = int(uScreenSize.y);
    float aspectRatio = currentAspectRatio;
    float resolutionScale = uScreenSize.x/256.0;
    float _x = fpos.x;
    float _y = fpos.y;
    float _z = fpos.z;

    for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++)
    {
        // vertex mode
        if ((shapes[shapeIndex].effects & 0x1) == 0) {
            bool attrMatchEqual = false;
            bool attrMatchEqual2 = false;
            bool attrMatchNeg = false;
            for (int i = 0; i < 4; i++) {
                if (shapes[shapeIndex].polygonAttributes[i] != 0) {
                    attrMatchEqual = true;
                    if (shapes[shapeIndex].polygonAttributes[i] == attr) {
                        attrMatchEqual2 = true;
                        break;
                    }
                }
            }
            for (int i = 0; i < 4; i++) {
                if (shapes[shapeIndex].negatedPolygonAttributes[i] != 0 && shapes[shapeIndex].negatedPolygonAttributes[i] == attr) {
                    attrMatchNeg = true;
                    break;
                }
            }
            bool attrMatch = (attrMatchEqual ? attrMatchEqual2 : true) && !attrMatchNeg;

            bool colorMatchEqual = false;
            bool colorMatchEqual2 = false;
            bool colorMatchNeg = false;
            // for (int i = 0; i < shapes[shapeIndex].colorCount; i++) {
            //     colorMatchEqual = true;
            //     if ((((shapes[shapeIndex].color[i] >> 8) & 0xFF) == (rgb[0] >> 1))
            //         && (((shapes[shapeIndex].color[i] >> 4) & 0xFF) == (rgb[1] >> 1))
            //         && (((shapes[shapeIndex].color[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
            //         colorMatchEqual2 = true;
            //         break;
            //     }
            // }
            // for (int i = 0; i < shapes[shapeIndex].negatedColorCount; i++) {
            //     if ((((shapes[shapeIndex].negatedColor[i] >> 8) & 0xFF) == (rgb[0] >> 1))
            //         && (((shapes[shapeIndex].negatedColor[i] >> 4) & 0xFF) == (rgb[1] >> 1))
            //         && (((shapes[shapeIndex].negatedColor[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
            //         colorMatchNeg = true;
            //         break;
            //     }
            // }
            bool colorMatch = (colorMatchEqual ? !colorMatchEqual2 : true) && !colorMatchNeg;

            float iuTexScale = SCREEN_SCALE/shapes[shapeIndex].hudScale;

            float scaleX = shapes[shapeIndex].sourceScale.x;
            float scaleY = shapes[shapeIndex].sourceScale.y;

            float heightScale = 1.0/aspectRatio;

            float squareFinalWidth = (shapes[shapeIndex].squareInitialCoords.z*scaleX*resolutionScale*heightScale)/iuTexScale;
            float squareFinalHeight = (shapes[shapeIndex].squareInitialCoords.w*scaleY*resolutionScale)/iuTexScale;

            if (_x >= shapes[shapeIndex].squareInitialCoords.x*resolutionScale && _x <= (shapes[shapeIndex].squareInitialCoords.x + shapes[shapeIndex].squareInitialCoords.z)*resolutionScale &&
                _y >= shapes[shapeIndex].squareInitialCoords.y*resolutionScale && _y <= (shapes[shapeIndex].squareInitialCoords.y + shapes[shapeIndex].squareInitialCoords.w)*resolutionScale &&
                _z >= shapes[shapeIndex].zRange.x && _z <= shapes[shapeIndex].zRange.y && attrMatch && colorMatch)
            {
                // hide vertex
                if ((shapes[shapeIndex].effects & 0x2) != 0)
                {
                    _x = 0;
                    _y = 0;
                }
                else {
                    float squareFinalX1 = 0.0;
                    float squareFinalY1 = 0.0;

                    switch (shapes[shapeIndex].corner)
                    {
                        case 0: // corner_PreservePosition
                            squareFinalX1 = (shapes[shapeIndex].squareInitialCoords.x + shapes[shapeIndex].squareInitialCoords.z/2)*scaleX - squareFinalWidth/2;
                            squareFinalY1 = (shapes[shapeIndex].squareInitialCoords.y + shapes[shapeIndex].squareInitialCoords.w/2)*scaleY - squareFinalHeight/2;
                            break;

                        case 1: // corner_Center
                            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                            break;
                        
                        case 2: // corner_TopLeft
                            break;
                        
                        case 3: // corner_Top
                            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                            break;

                        case 4: // corner_TopRight
                            squareFinalX1 = ScreenWidth - squareFinalWidth;
                            break;

                        case 5: // corner_Right
                            squareFinalX1 = ScreenWidth - squareFinalWidth;
                            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                            break;

                        case 6: // corner_BottomRight
                            squareFinalX1 = ScreenWidth - squareFinalWidth;
                            squareFinalY1 = ScreenHeight - squareFinalHeight;
                            break;

                        case 7: // corner_Bottom
                            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                            squareFinalY1 = ScreenHeight - squareFinalHeight;
                            break;

                        case 8: // corner_BottomLeft
                            squareFinalY1 = ScreenHeight - squareFinalHeight;
                            break;

                        case 9: // corner_Left
                            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                    }

                    _x = ((_x - shapes[shapeIndex].squareInitialCoords.x*resolutionScale)/iuTexScale)*scaleX*heightScale + squareFinalX1 + (shapes[shapeIndex].margin.x - shapes[shapeIndex].margin.z)*(resolutionScale/(iuTexScale*aspectRatio));
                    _y = ((_y - shapes[shapeIndex].squareInitialCoords.y*resolutionScale)/iuTexScale)*scaleY             + squareFinalY1 + (shapes[shapeIndex].margin.y - shapes[shapeIndex].margin.w)*(resolutionScale/iuTexScale);
                }

                fpos.x = _x;
                fpos.y = _y;
                break;
            }
        }
    }

    fpos.xy = ((fpos.xy * 2.0) / uScreenSize) - 1.0;
    fpos.xyz *= fpos.w;

    fColor = vec4(vColor) / vec4(255.0,255.0,255.0,31.0);
    fTexcoord = vec2(vTexcoord) / 16.0;
    fPolygonAttr = vPolygonAttr;

    gl_Position = fpos;
}
)";

}

#endif // PLUGIN_GPU3D_OPENGL_SHADERS_H
