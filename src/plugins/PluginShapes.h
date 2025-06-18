#ifndef PLUGIN_SHAPES_H
#define PLUGIN_SHAPES_H

#define SHAPES_DATA_ARRAY_SIZE 32

#define SCREEN_SCALE 6.0

namespace Plugins
{

enum
{
    shape_Square
};

enum
{
    corner_PreservePosition,
    corner_Center,
    corner_TopLeft,
    corner_Top,
    corner_TopRight,
    corner_Right,
    corner_BottomRight,
    corner_Bottom,
    corner_BottomLeft,
    corner_Left
};

enum
{
    screenLayout_Top,
    screenLayout_Bottom,
    screenLayout_BothVertical,
    screenLayout_BothHorizontal
};

enum
{
    brightnessMode_Default,
    brightnessMode_TopScreen,
    brightnessMode_BottomScreen,
    brightnessMode_Horizontal,
    brightnessMode_BlackScreen,
    brightnessMode_DisableBrightnessControl,
    brightnessMode_Auto
};

enum
{
    mirror_None = 0x00,
    mirror_X    = 0x08,
    mirror_Y    = 0x10,
    mirror_XY   = 0x18
};

struct vec2 {
    float x, y;
};

struct ivec2 {
    int x, y;
};

struct vec3 {
    float x, y, z;
};

struct ivec3 {
    int x, y, z;
};

struct ivec4 {
    int x, y, z, w;
};

struct vec4 {
    float x, y, z, w;
};

// UBO-compatible struct with proper padding
struct alignas(16) ShapeData2D { // 112 bytes
    vec2 sourceScale;  // 8 bytes (X factor, Y factor)

    int effects;
    // 0x01 => invertGrayScaleColors
    // 0x02 => crop corner as triangle
    // 0x04 => rounded corners
    // 0x08 => mirror X
    // 0x10 => mirror Y
    // 0x20 => manipulate transparency
    // 0x40 => repeat as background (X axis)
    // 0x80 => repeat as background (Y axis)

    float opacity;

    ivec4 squareInitialCoords;  // 16 bytes (X, Y, Width, Height)
    vec4 squareFinalCoords;     // 16 bytes (X, Y, Width, Height)

    vec4 fadeBorderSize;        // 16 bytes (left fade, top fade, right fade, down fade)

    vec4 squareCornersModifier; // 16 bytes (top left, top right, bottom left, bottom right)

    ivec4 colorToAlpha;         // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
    ivec4 singleColorToAlpha;   // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
};

// UBO-compatible struct with proper padding
struct alignas(16) ShapeData3D {
    vec2 sourceScale;  // 8 bytes (X factor, Y factor)

    int corner;     // 4 bytes
    float hudScale; // 4 bytes

    ivec4 squareInitialCoords; // 16 bytes (X, Y, Width, Height)

    vec4 margin; // 16 bytes (left, top, right, bottom)

    vec2 zRange;
    int polygonVertexesCount;

    int effects;
    // 0x01 => polygon mode
    // 0x02 => hide

    unsigned int polygonAttributes[4];
    unsigned int negatedPolygonAttributes[4];

    int color[4];
    int negatedColor[4];

    unsigned int textureParams[4];
    unsigned int negatedTextureParams[4];

    int colorCount = 0;
    int negatedColorCount = 0;
    int textureParamCount = 0;
    int negatedTextureParamCount = 0;

    bool doesAttributeMatch(unsigned int polygonAttr) {
        bool attrMatchEqual = false;
        bool attrMatchEqual2 = false;
        bool attrMatchNeg = false;
        for (int i = 0; i < 4; i++) {
            if (polygonAttributes[i] != 0) {
                attrMatchEqual = true;
                if (polygonAttributes[i] == polygonAttr) {
                    attrMatchEqual2 = true;
                    break;
                }
            }
        }
        for (int i = 0; i < 4; i++) {
            if (negatedPolygonAttributes[i] != 0 && negatedPolygonAttributes[i] == polygonAttr) {
                attrMatchNeg = true;
                break;
            }
        }
        return (attrMatchEqual ? attrMatchEqual2 : true) && !attrMatchNeg;
    }

    bool doesTextureParamMatch(unsigned int texParam) {
        bool attrMatchEqual = false;
        bool attrMatchEqual2 = false;
        bool attrMatchNeg = false;
        for (int i = 0; i < 4; i++) {
            if (textureParams[i] != 0) {
                attrMatchEqual = true;
                if (textureParams[i] == texParam) {
                    attrMatchEqual2 = true;
                    break;
                }
            }
        }
        for (int i = 0; i < 4; i++) {
            if (negatedTextureParams[i] != 0 && negatedTextureParams[i] == texParam) {
                attrMatchNeg = true;
                break;
            }
        }
        return (attrMatchEqual ? attrMatchEqual2 : true) && !attrMatchNeg;
    }

    bool doesColorMatch(int* rgb)
    {
        bool colorMatchEqual = false;
        bool colorMatchEqual2 = false;
        bool colorMatchNeg = false;
        for (int i = 0; i < colorCount; i++) {
            colorMatchEqual = true;
            if ((((color[i] >> 8) & 0xFF) == (rgb[0] >> 1))
            && (((color[i] >> 4) & 0xFF) == (rgb[1] >> 1))
            && (((color[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
                colorMatchEqual2 = true;
                break;
            }
        }
        for (int i = 0; i < negatedColorCount; i++) {
            if ((((negatedColor[i] >> 8) & 0xFF) == (rgb[0] >> 1))
            && (((negatedColor[i] >> 4) & 0xFF) == (rgb[1] >> 1))
            && (((negatedColor[i] >> 0) & 0xFF) == (rgb[2] >> 1))) {
                colorMatchNeg = true;
                break;
            }
        }
        return (colorMatchEqual ? !colorMatchEqual2 : true) && !colorMatchNeg;
    }

    vec3 compute3DCoordinatesOf3DSquareShapeInVertexMode(float _x, float _y, float _z, unsigned int polygonAttr, unsigned int texParam, int* rgb, float resolutionScale, float aspectRatio)
    {
        float updated = 0;

        bool loggerModeEnabled = (effects & 0x4) != 0;

        bool attrMatch = doesAttributeMatch(polygonAttr);
        if (!attrMatch) {
            return vec3{_x, _y, updated};
        }

        bool texParamMatch = doesTextureParamMatch(texParam);
        if (!texParamMatch) {
            return vec3{_x, _y, updated};
        }

        bool colorMatch = doesColorMatch(rgb);
        if (!colorMatch) {
            return vec3{_x, _y, updated};
        }

        float iuTexScale = SCREEN_SCALE/hudScale;

        float ScreenWidth = 256.0*resolutionScale;
        float ScreenHeight = 192.0*resolutionScale;

        float scaleX = sourceScale.x;
        float scaleY = sourceScale.y;
        
        float heightScale = 1.0/aspectRatio;

        float squareFinalWidth = (squareInitialCoords.z*scaleX*resolutionScale*heightScale)/iuTexScale;
        float squareFinalHeight = (squareInitialCoords.w*scaleY*resolutionScale)/iuTexScale;

        if (_x >= squareInitialCoords.x*resolutionScale && _x <= (squareInitialCoords.x + squareInitialCoords.z)*resolutionScale &&
            _y >= squareInitialCoords.y*resolutionScale && _y <= (squareInitialCoords.y + squareInitialCoords.w)*resolutionScale &&
            _z >= zRange.x && _z <= zRange.y)
        {
            updated = 1;

            // hide vertex
            if ((effects & 0x2) != 0)
            {
                _x = 0;
                _y = 0;
            }
            else {
                float squareFinalX1 = 0.0;
                float squareFinalY1 = 0.0;

                switch (corner)
                {
                    case corner_Center:
                        squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                        squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                        break;
                    
                    case corner_TopLeft:
                        break;
                    
                    case corner_Top:
                        squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                        break;

                    case corner_TopRight:
                        squareFinalX1 = ScreenWidth - squareFinalWidth;
                        break;

                    case corner_Right:
                        squareFinalX1 = ScreenWidth - squareFinalWidth;
                        squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                        break;

                    case corner_BottomRight:
                        squareFinalX1 = ScreenWidth - squareFinalWidth;
                        squareFinalY1 = ScreenHeight - squareFinalHeight;
                        break;

                    case corner_Bottom:
                        squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                        squareFinalY1 = ScreenHeight - squareFinalHeight;
                        break;

                    case corner_BottomLeft:
                        squareFinalY1 = ScreenHeight - squareFinalHeight;
                        break;

                    case corner_Left:
                        squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                        break;

                    case corner_PreservePosition:
                    default:
                        squareFinalX1 = (squareInitialCoords.x + squareInitialCoords.z/2)*scaleX - squareFinalWidth/2;
                        squareFinalY1 = (squareInitialCoords.y + squareInitialCoords.w/2)*scaleY - squareFinalHeight/2;
                        break;
                }

                _x = ((_x - squareInitialCoords.x*resolutionScale)/iuTexScale)*scaleX*heightScale + squareFinalX1 + (margin.x - margin.z)*(resolutionScale/(iuTexScale*aspectRatio));
                _y = ((_y - squareInitialCoords.y*resolutionScale)/iuTexScale)*scaleY             + squareFinalY1 + (margin.y - margin.w)*(resolutionScale/iuTexScale);
            }
        }

        return vec3{_x, _y, updated};
    }
};

class ShapeBuilder2D
{
public:
    static ShapeBuilder2D square() {
        auto shapeBuilder = ShapeBuilder2D();
        shapeBuilder._shape = shape_Square;
        shapeBuilder.shapeData.sourceScale.x = 1.0;
        shapeBuilder.shapeData.sourceScale.y = 1.0;
        shapeBuilder.shapeData.squareInitialCoords.x = 0;
        shapeBuilder.shapeData.squareInitialCoords.y = 0;
        shapeBuilder.shapeData.squareInitialCoords.z = 256;
        shapeBuilder.shapeData.squareInitialCoords.w = 192;
        shapeBuilder.shapeData.opacity = 1.0;
        shapeBuilder._hudScale = 1.0;
        shapeBuilder._corner = corner_PreservePosition;
        shapeBuilder._margin.x = 0;
        shapeBuilder._margin.y = 0;
        shapeBuilder._margin.z = 0;
        shapeBuilder._margin.w = 0;
        shapeBuilder._fromBottomScreen = false;
        return shapeBuilder;
    }

    ShapeBuilder2D& fromBottomScreen() {
        _fromBottomScreen = true;
        return *this;
    }
    ShapeBuilder2D& placeAtCorner(int corner) {
        _corner = corner;
        return *this;
    }
    ShapeBuilder2D& sourceScale(float _scale) {
        shapeData.sourceScale.x = _scale;
        shapeData.sourceScale.y = _scale;
        return *this;
    }
    ShapeBuilder2D& sourceScale(float _scaleX, float _scaleY) {
        shapeData.sourceScale.x = _scaleX;
        shapeData.sourceScale.y = _scaleY;
        return *this;
    }
    ShapeBuilder2D& hudScale(float hudScale) {
        _hudScale = hudScale;
        return *this;
    }
    ShapeBuilder2D& preserveDsScale() {
        float mulScale = SCREEN_SCALE/_hudScale;
        shapeData.sourceScale.x *= mulScale;
        shapeData.sourceScale.y *= mulScale;
        return *this;
    }
    ShapeBuilder2D& fromPosition(int x, int y) {
        if (_shape == shape_Square) {
            shapeData.squareInitialCoords.x = x;
            shapeData.squareInitialCoords.y = y;
        }
        return *this;
    }
    ShapeBuilder2D& withSize(int width, int height) {
        if (_shape == shape_Square) {
            shapeData.squareInitialCoords.z = width;
            shapeData.squareInitialCoords.w = height;
        }
        return *this;
    }
    ShapeBuilder2D& withMargin(float left, float top, float right, float bottom) {
        _margin.x = left;
        _margin.y = top;
        _margin.z = right;
        _margin.w = bottom;
        return *this;
    }
    ShapeBuilder2D& fadeBorderSize(float left, float top, float right, float bottom) {
        shapeData.effects |= 0x20;
        shapeData.fadeBorderSize.x = left;
        shapeData.fadeBorderSize.y = top;
        shapeData.fadeBorderSize.z = right;
        shapeData.fadeBorderSize.w = bottom;
        return *this;
    }
    ShapeBuilder2D& invertGrayScaleColors() {
        shapeData.effects |= 0x1;
        return *this;
    }
    ShapeBuilder2D& repeatAsBackground() {
        shapeData.effects |= 0x40;
        shapeData.effects |= 0x80;
        return *this;
    }
    ShapeBuilder2D& repeatAsBackgroundHorizontally() {
        shapeData.effects |= 0x40;
        return *this;
    }
    ShapeBuilder2D& repeatAsBackgroundVertically() {
        shapeData.effects |= 0x80;
        return *this;
    }
    ShapeBuilder2D& force() {
        shapeData.effects |= 0x100;
        return *this;
    }
    ShapeBuilder2D& mirror(int mirror) {
        shapeData.effects |= mirror;
        return *this;
    }
    ShapeBuilder2D& cropSquareCorners(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.effects |= 0x2;
        shapeData.squareCornersModifier.x = topLeft;
        shapeData.squareCornersModifier.y = topRight;
        shapeData.squareCornersModifier.z = bottomLeft;
        shapeData.squareCornersModifier.w = bottomRight;
        return *this;
    }
    ShapeBuilder2D& squareBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.effects |= 0x4;
        shapeData.squareCornersModifier.x = topLeft;
        shapeData.squareCornersModifier.y = topRight;
        shapeData.squareCornersModifier.z = bottomLeft;
        shapeData.squareCornersModifier.w = bottomRight;
        return *this;
    }
    ShapeBuilder2D& squareBorderRadius(float radius) {
        return squareBorderRadius(radius, radius, radius, radius);
    }
    ShapeBuilder2D& opacity(float opacity) {
        shapeData.effects |= 0x20;
        shapeData.opacity = opacity;
        return *this;
    }
    ShapeBuilder2D& colorToAlpha(int red, int green, int blue) {
        shapeData.effects |= 0x20;
        shapeData.colorToAlpha.x = red >> 2;
        shapeData.colorToAlpha.y = green >> 2;
        shapeData.colorToAlpha.z = blue >> 2;
        shapeData.colorToAlpha.w = 1;
        return *this;
    }
    ShapeBuilder2D& singleColorToAlpha(int red, int green, int blue) {
        shapeData.effects |= 0x20;
        shapeData.singleColorToAlpha.x = red >> 2;
        shapeData.singleColorToAlpha.y = green >> 2;
        shapeData.singleColorToAlpha.z = blue >> 2;
        shapeData.singleColorToAlpha.w = 1;
        return *this;
    }
    ShapeBuilder2D& singleColorToAlpha(int red, int green, int blue, float alpha) {
        shapeData.effects |= 0x20;
        shapeData.singleColorToAlpha.x = red >> 2;
        shapeData.singleColorToAlpha.y = green >> 2;
        shapeData.singleColorToAlpha.z = blue >> 2;
        shapeData.singleColorToAlpha.w = (int)(alpha * 64);
        return *this;
    }

    void precompute3DCoordinatesOf2DSquareShape(float aspectRatio)
    {
        float iuTexScale = SCREEN_SCALE/_hudScale;

        float ScreenWidth = 256.0*iuTexScale;
        float ScreenHeight = 192.0*iuTexScale;

        float scaleX = shapeData.sourceScale.x;
        float scaleY = shapeData.sourceScale.y;
        int corner = _corner;

        float heightScale = 1.0/aspectRatio;

        float squareFinalWidth = shapeData.squareInitialCoords.z*scaleX*heightScale;
        float squareFinalHeight = shapeData.squareInitialCoords.w*scaleY;

        float squareFinalX1 = 0.0;
        float squareFinalY1 = 0.0;

        switch (corner)
        {
            case corner_Center:
                squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                break;
            
            case corner_TopLeft:
                break;
            
            case corner_Top:
                squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                break;

            case corner_TopRight:
                squareFinalX1 = ScreenWidth - squareFinalWidth;
                break;

            case corner_Right:
                squareFinalX1 = ScreenWidth - squareFinalWidth;
                squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                break;

            case corner_BottomRight:
                squareFinalX1 = ScreenWidth - squareFinalWidth;
                squareFinalY1 = ScreenHeight - squareFinalHeight;
                break;

            case corner_Bottom:
                squareFinalX1 = (ScreenWidth - squareFinalWidth)/2;
                squareFinalY1 = ScreenHeight - squareFinalHeight;
                break;

            case corner_BottomLeft:
                squareFinalY1 = ScreenHeight - squareFinalHeight;
                break;

            case corner_Left:
                squareFinalY1 = (ScreenHeight - squareFinalHeight)/2;
                break;

            case corner_PreservePosition:
            default:
                squareFinalX1 = (shapeData.squareInitialCoords.x + shapeData.squareInitialCoords.z/2)*scaleX - squareFinalWidth/2;
                squareFinalY1 = (shapeData.squareInitialCoords.y + shapeData.squareInitialCoords.w/2)*scaleY - squareFinalHeight/2;
                break;
        }

        squareFinalX1 = squareFinalX1 + (_margin.x - _margin.z)*heightScale;
        squareFinalY1 = squareFinalY1 + (_margin.y - _margin.w);

        float squareFinalX2 = squareFinalX1 + squareFinalWidth;
        float squareFinalY2 = squareFinalY1 + squareFinalHeight;

        shapeData.squareFinalCoords = {squareFinalX1, squareFinalY1, squareFinalX2, squareFinalY2};
    }

    ShapeData2D build(float aspectRatio) {
        if (_fromBottomScreen) {
            shapeData.squareInitialCoords.y += 192;
        }

        if (_shape == shape_Square) {
            precompute3DCoordinatesOf2DSquareShape(aspectRatio);
        }

        return shapeData;
    }
private:
    ShapeData2D shapeData;

    int _shape;
    bool _fromBottomScreen;
    int _corner;
    float _hudScale;
    vec4 _margin;
};

class ShapeBuilder3D
{
public:
    static ShapeBuilder3D square() {
        auto shapeBuilder = ShapeBuilder3D();
        shapeBuilder._shape = shape_Square;
        shapeBuilder.shapeData.sourceScale.x = 1.0;
        shapeBuilder.shapeData.sourceScale.y = 1.0;
        shapeBuilder.shapeData.squareInitialCoords.x = 0;
        shapeBuilder.shapeData.squareInitialCoords.y = 0;
        shapeBuilder.shapeData.squareInitialCoords.z = 256;
        shapeBuilder.shapeData.squareInitialCoords.w = 192;
        shapeBuilder.shapeData.hudScale = SCREEN_SCALE;
        shapeBuilder.shapeData.corner = corner_PreservePosition;
        shapeBuilder.shapeData.zRange.x = -1.0;
        shapeBuilder.shapeData.zRange.y = 1.0;
        shapeBuilder.shapeData.margin.x = 0;
        shapeBuilder.shapeData.margin.y = 0;
        shapeBuilder.shapeData.margin.z = 0;
        shapeBuilder.shapeData.margin.w = 0;
        return shapeBuilder;
    }

    ShapeBuilder3D& polygonAttributes(int _polygonAttributes) {
        shapeData.polygonAttributes[_polygonAttributesIndex++] = _polygonAttributes;
        return *this;
    }
    ShapeBuilder3D& negatePolygonAttributes(int _polygonAttributes) {
        shapeData.negatedPolygonAttributes[_negatedPolygonAttributesIndex++] = _polygonAttributes;
        return *this;
    }
    ShapeBuilder3D& color(int _color) {
        shapeData.color[shapeData.colorCount++] = _color;
        return *this;
    }
    ShapeBuilder3D& negateColor(int _color) {
        shapeData.negatedColor[shapeData.negatedColorCount++] = _color;
        return *this;
    }
    ShapeBuilder3D& textureParam(int _textureParam) {
        shapeData.textureParams[shapeData.textureParamCount++] = _textureParam;
        return *this;
    }
    ShapeBuilder3D& negatedTextureParam(int _textureParam) {
        shapeData.negatedTextureParams[shapeData.negatedTextureParamCount++] = _textureParam;
        return *this;
    }
    ShapeBuilder3D& polygonMode() {
        shapeData.effects |= 0x1;
        return *this;
    }
    ShapeBuilder3D& adjustAspectRatioOnly() {
        shapeData.effects |= 0x8;
        return *this;
    }
    ShapeBuilder3D& polygonVertexesCount(int _polygonVertexesCount) {
        shapeData.polygonVertexesCount = _polygonVertexesCount;
        return *this;
    }
    ShapeBuilder3D& placeAtCorner(int corner) {
        shapeData.corner = corner;
        return *this;
    }
    ShapeBuilder3D& zRange(float _minZ, float _maxZ) {
        shapeData.zRange.x = _minZ;
        shapeData.zRange.y = _maxZ;
        return *this;
    }
    ShapeBuilder3D& zValue(float z) {
        shapeData.zRange.x = z;
        shapeData.zRange.y = z;
        return *this;
    }
    ShapeBuilder3D& sourceScale(float _scale) {
        shapeData.sourceScale.x = _scale;
        shapeData.sourceScale.y = _scale;
        return *this;
    }
    ShapeBuilder3D& sourceScale(float _scaleX, float _scaleY) {
        shapeData.sourceScale.x = _scaleX;
        shapeData.sourceScale.y = _scaleY;
        return *this;
    }
    ShapeBuilder3D& hudScale(float hudScale) {
        shapeData.hudScale = hudScale;
        return *this;
    }
    ShapeBuilder3D& preserveDsScale() {
        float mulScale = SCREEN_SCALE/shapeData.hudScale;
        shapeData.sourceScale.x *= mulScale;
        shapeData.sourceScale.y *= mulScale;
        return *this;
    }
    ShapeBuilder3D& fromPosition(int x, int y) {
        if (_shape == shape_Square) {
            shapeData.squareInitialCoords.x = x;
            shapeData.squareInitialCoords.y = y;
        }
        return *this;
    }
    ShapeBuilder3D& withSize(int width, int height) {
        if (_shape == shape_Square) {
            shapeData.squareInitialCoords.z = width;
            shapeData.squareInitialCoords.w = height;
        }
        return *this;
    }
    ShapeBuilder3D& withMargin(float left, float top, float right, float bottom) {
        shapeData.margin.x = left;
        shapeData.margin.y = top;
        shapeData.margin.z = right;
        shapeData.margin.w = bottom;
        return *this;
    }
    ShapeBuilder3D& hide() {
        shapeData.effects |= 0x2;
        return *this;
    }
    ShapeBuilder3D& logger() {
        shapeData.effects |= 0x4;
        return *this;
    }

    ShapeData3D build(float aspectRatio) {
        _aspectRatio = aspectRatio;
        return shapeData;
    }
private:
    ShapeData3D shapeData;

    int _shape;
    float _aspectRatio;
    int _polygonAttributesIndex = 0;
    int _negatedPolygonAttributesIndex = 0;
};

}

#endif // PLUGIN_SHAPES_H
