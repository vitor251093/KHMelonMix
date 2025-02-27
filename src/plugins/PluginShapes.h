#ifndef PLUGIN_SHAPES_H
#define PLUGIN_SHAPES_H

#define SHAPES_DATA_ARRAY_SIZE 32

#define SCREEN_SCALE 6.0

namespace Plugins
{

struct vec2 {
    float x, y;
};

struct ivec2 {
    int x, y;
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

    float opacity;

    ivec4 squareInitialCoords; // 16 bytes (X, Y, Width, Height)
    vec4 squareFinalCoords;    // 16 bytes (X, Y, Width, Height)

    vec4 fadeBorderSize;       // 16 bytes (left fade, top fade, right fade, down fade)

    vec4 squareCornersModifier;    // 16 bytes (top left, top right, bottom left, bottom right)

    ivec4 colorToAlpha;        // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
    ivec4 singleColorToAlpha;  // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
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

    int polygonAttributes[4];
    int negatedPolygonAttributes[4];

    int color[4];
    int negatedColor[4];

    int colorCount = 0;
    int negatedColorCount = 0;
    int _pad0, _pad1;
};

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
    brightnessMode_Off
};

enum
{
    mirror_None = 0x00,
    mirror_X    = 0x08,
    mirror_Y    = 0x10,
    mirror_XY   = 0x18
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

    void precompute3DCoordinatesOf2DSquareShape(float aspectRatio)
    {
        float iuTexScale = SCREEN_SCALE/_hudScale;
        float scaleX = shapeData.sourceScale.x;
        float scaleY = shapeData.sourceScale.y;
        
        float heightScale = 1.0/aspectRatio;

        float squareFinalWidth = shapeData.squareInitialCoords.z*scaleX*heightScale;
        float squareFinalHeight = shapeData.squareInitialCoords.w*scaleY;

        float squareFinalX1 = 0.0;
        float squareFinalY1 = 0.0;

        switch (_corner)
        {
            case corner_PreservePosition:
                squareFinalX1 = (shapeData.squareInitialCoords.x + shapeData.squareInitialCoords.z/2)*scaleX - squareFinalWidth/2;
                squareFinalY1 = (shapeData.squareInitialCoords.y + shapeData.squareInitialCoords.w/2)*scaleY - squareFinalHeight/2;
                break;

            case corner_Center:
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
                break;
            
            case corner_TopLeft:
                break;
            
            case corner_Top:
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                break;

            case corner_TopRight:
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                break;

            case corner_Right:
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
                break;

            case corner_BottomRight:
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case corner_Bottom:
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case corner_BottomLeft:
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case corner_Left:
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
        }

        squareFinalX1 = squareFinalX1 + _margin.x*heightScale - _margin.z*heightScale;
        squareFinalY1 = squareFinalY1 + _margin.y - _margin.w;

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
    ShapeBuilder3D& polygonMode() {
        shapeData.effects |= 0x1;
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
