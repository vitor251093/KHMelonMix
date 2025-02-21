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
struct alignas(16) ShapeData2D { // 128 bytes
    int shape;         // 4 bytes
    int _pad2;         // 4 bytes
    vec2 sourceScale;  // 8 bytes

    ivec4 squareInitialCoords; // 16 bytes (X, Y, Width, Height)
    vec4 squareFinalCoords;    // 16 bytes (X, Y, Width, Height)

    vec4 fadeBorderSize;       // 16 bytes (left fade, top fade, right fade, down fade)

    int effects;
    // 0x01 => invertGrayScaleColors
    // 0x02 => crop corner as triangle
    // 0x04 => rounded corners
    // 0x08 => mirror X
    // 0x10 => mirror Y

    float opacity;
    int _pad0, _pad1;   // Padding to align the struct to 16 bytes

    vec4 squareCornersModifier;    // 16 bytes (top left, top right, bottom left, bottom right)

    ivec4 colorToAlpha;        // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
    ivec4 singleColorToAlpha;  // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
};

enum
{
    shape_Square
};

enum
{
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

class ShapeBuilder
{
public:
    static ShapeBuilder square() {
        auto shapeBuilder = ShapeBuilder();
        shapeBuilder.shapeData.shape = shape_Square;
        shapeBuilder.shapeData.sourceScale.x = 1.0;
        shapeBuilder.shapeData.sourceScale.y = 1.0;
        shapeBuilder.shapeData.squareInitialCoords.x = 0;
        shapeBuilder.shapeData.squareInitialCoords.y = 0;
        shapeBuilder.shapeData.squareInitialCoords.z = 256;
        shapeBuilder.shapeData.squareInitialCoords.w = 192;
        shapeBuilder.shapeData.opacity = 1.0;
        shapeBuilder._hudScale = 1.0;
        shapeBuilder._corner = corner_Center;
        shapeBuilder._margin.x = 0;
        shapeBuilder._margin.y = 0;
        shapeBuilder._margin.z = 0;
        shapeBuilder._margin.w = 0;
        shapeBuilder._fromBottomScreen = false;
        return shapeBuilder;
    }

    ShapeBuilder& fromBottomScreen() {
        _fromBottomScreen = true;
        return *this;
    }
    ShapeBuilder& placeAtCorner(int corner) {
        _corner = corner;
        return *this;
    }
    ShapeBuilder& sourceScale(float _scale) {
        shapeData.sourceScale.x = _scale;
        shapeData.sourceScale.y = _scale;
        return *this;
    }
    ShapeBuilder& sourceScale(float _scaleX, float _scaleY) {
        shapeData.sourceScale.x = _scaleX;
        shapeData.sourceScale.y = _scaleY;
        return *this;
    }
    ShapeBuilder& hudScale(float hudScale) {
        _hudScale = hudScale;
        return *this;
    }
    ShapeBuilder& preserveDsScale() {
        return sourceScale(SCREEN_SCALE);
    }
    ShapeBuilder& fromPosition(int x, int y) {
        if (shapeData.shape == shape_Square) {
            shapeData.squareInitialCoords.x = x;
            shapeData.squareInitialCoords.y = y;
        }
        return *this;
    }
    ShapeBuilder& withSize(int width, int height) {
        if (shapeData.shape == shape_Square) {
            shapeData.squareInitialCoords.z = width;
            shapeData.squareInitialCoords.w = height;
        }
        return *this;
    }
    ShapeBuilder& withMargin(float left, float top, float right, float bottom) {
        _margin.x = left;
        _margin.y = top;
        _margin.z = right;
        _margin.w = bottom;
        return *this;
    }
    ShapeBuilder& fadeBorderSize(float left, float top, float right, float bottom) {
        shapeData.fadeBorderSize.x = left;
        shapeData.fadeBorderSize.y = top;
        shapeData.fadeBorderSize.z = right;
        shapeData.fadeBorderSize.w = bottom;
        return *this;
    }
    ShapeBuilder& invertGrayScaleColors() {
        shapeData.effects |= 0x1;
        return *this;
    }
    ShapeBuilder& mirror(int mirror) {
        shapeData.effects |= mirror;
        return *this;
    }
    ShapeBuilder& cropSquareCorners(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.effects |= 0x2;
        shapeData.squareCornersModifier.x = topLeft;
        shapeData.squareCornersModifier.y = topRight;
        shapeData.squareCornersModifier.z = bottomLeft;
        shapeData.squareCornersModifier.w = bottomRight;
        return *this;
    }
    ShapeBuilder& squareBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.effects |= 0x4;
        shapeData.squareCornersModifier.x = topLeft;
        shapeData.squareCornersModifier.y = topRight;
        shapeData.squareCornersModifier.z = bottomLeft;
        shapeData.squareCornersModifier.w = bottomRight;
        return *this;
    }
    ShapeBuilder& squareBorderRadius(float radius) {
        return squareBorderRadius(radius, radius, radius, radius);
    }
    ShapeBuilder& opacity(float opacity) {
        shapeData.opacity = opacity;
        return *this;
    }
    ShapeBuilder& colorToAlpha(int red, int green, int blue) {
        shapeData.colorToAlpha.x = red >> 2;
        shapeData.colorToAlpha.y = green >> 2;
        shapeData.colorToAlpha.z = blue >> 2;
        shapeData.colorToAlpha.w = 1;
        return *this;
    }
    ShapeBuilder& singleColorToAlpha(int red, int green, int blue) {
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

        float squareFinalHeight = shapeData.squareInitialCoords.w*scaleY;
        float squareFinalWidth = shapeData.squareInitialCoords.z*scaleX*heightScale;

        float squareFinalX1 = 0.0;
        float squareFinalY1 = 0.0;

        switch (_corner)
        {
            case 0: // square at center
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
                break;
            
            case 1: // square at top left corner
                break;
            
            case 2: // square at top
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                break;

            case 3: // square at top right corner
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                break;

            case 4: // square at right
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
                break;

            case 5: // square at bottom right corner
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case 6: // square at bottom
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case 7: // square at left bottom corner
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight;
                break;

            case 8: // square at left
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

        if (shapeData.shape == shape_Square) {
            precompute3DCoordinatesOf2DSquareShape(aspectRatio);
        }

        return shapeData;
    }
private:
    ShapeData2D shapeData;

    bool _fromBottomScreen;
    int _corner;
    float _hudScale;
    vec4 _margin;
};

}

#endif
