#ifndef PLUGIN_SHAPES_H
#define PLUGIN_SHAPES_H

#define SHAPES_DATA_ARRAY_SIZE 32

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
struct alignas(16) ShapeData { // 128 bytes
    int shape;       // 4 bytes
    float uiScale;   // 4 bytes
    vec2 scale;      // 8 bytes

    ivec4 squareInitialCoords; // 16 bytes (X, Y, Width, Height)
    vec4 squareFinalCoords;    // 16 bytes (X, Y, Width, Height)

    vec4 fadeBorderSize;       // 16 bytes (left fade, top fade, right fade, down fade)

    int invertGrayScaleColors; // 4 bytes (bool -> int for std140)
    int _pad3, _pad4, _pad5;   // Padding to align the struct to 16 bytes

    vec4 cropSquareCorners;    // 16 bytes (top left, top right, bottom left, bottom right)
    vec4 squareBorderRadius;

    ivec4 colorToAlpha;        // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
    ivec4 singleColorToAlpha;  // 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
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

class ShapeBuilder
{
public:
    static ShapeBuilder square() {
        auto shapeBuilder = ShapeBuilder();
        shapeBuilder.shapeData.shape = 0;
        shapeBuilder.shapeData.uiScale = 1.0;
        shapeBuilder.shapeData.scale.x = 1.0;
        shapeBuilder.shapeData.scale.y = 1.0;
        shapeBuilder.shapeData.squareInitialCoords.x = 0;
        shapeBuilder.shapeData.squareInitialCoords.y = 0;
        shapeBuilder.shapeData.squareInitialCoords.z = 256;
        shapeBuilder.shapeData.squareInitialCoords.w = 192;
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
    ShapeBuilder& scale(float _scale) {
        shapeData.scale.x = _scale;
        shapeData.scale.y = _scale;
        return *this;
    }
    ShapeBuilder& scale(float _scaleX, float _scaleY) {
        shapeData.scale.x = _scaleX;
        shapeData.scale.y = _scaleY;
        return *this;
    }
    ShapeBuilder& uiScale(float uiScale) {
        shapeData.uiScale = uiScale;
        return *this;
    }
    ShapeBuilder& preserveDsScale() {
        shapeData.uiScale = 6.0;
        return *this;
    }
    ShapeBuilder& fromPosition(int x, int y) {
        if (shapeData.shape == 0) {
            shapeData.squareInitialCoords.x = x;
            shapeData.squareInitialCoords.y = y;
        }
        return *this;
    }
    ShapeBuilder& withSize(int width, int height) {
        if (shapeData.shape == 0) {
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
        shapeData.invertGrayScaleColors = 1;
        return *this;
    }
    ShapeBuilder& cropSquareCorners(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.cropSquareCorners.x = topLeft;
        shapeData.cropSquareCorners.y = topRight;
        shapeData.cropSquareCorners.z = bottomLeft;
        shapeData.cropSquareCorners.w = bottomRight;
        return *this;
    }
    ShapeBuilder& squareBorderRadius(float topLeft, float topRight, float bottomLeft, float bottomRight) {
        shapeData.squareBorderRadius.x = topLeft;
        shapeData.squareBorderRadius.y = topRight;
        shapeData.squareBorderRadius.z = bottomLeft;
        shapeData.squareBorderRadius.w = bottomRight;
        return *this;
    }
    ShapeBuilder& squareBorderRadius(float radius) {
        return squareBorderRadius(radius, radius, radius, radius);
    }
    ShapeBuilder& colorToAlpha(int red, int green, int blue) {
        shapeData.colorToAlpha.x = red;
        shapeData.colorToAlpha.y = green;
        shapeData.colorToAlpha.z = blue;
        shapeData.colorToAlpha.w = 1;
        return *this;
    }
    ShapeBuilder& singleColorToAlpha(int red, int green, int blue) {
        shapeData.singleColorToAlpha.x = red;
        shapeData.singleColorToAlpha.y = green;
        shapeData.singleColorToAlpha.z = blue;
        shapeData.singleColorToAlpha.w = 1;
        return *this;
    }

    void precompute3DCoordinatesOf2DSquareShape(float aspectRatio)
    {
        float iuTexScale = (6.0)/shapeData.uiScale;
        float scaleX = shapeData.scale.x;
        float scaleY = shapeData.scale.y;
        
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
                squareFinalX1 = _margin.x*heightScale;
                squareFinalY1 = _margin.y;
                break;
            
            case 2: // square at top
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = _margin.y;
                break;

            case 3: // square at top right corner
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - _margin.z*heightScale;
                squareFinalY1 = _margin.y;
                break;

            case 4: // square at right
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - _margin.z*heightScale;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
                break;

            case 5: // square at bottom right corner
                squareFinalX1 = 256.0*iuTexScale - squareFinalWidth - _margin.z*heightScale;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - _margin.w;
                break;

            case 6: // square at bottom
                squareFinalX1 = (256.0*iuTexScale - squareFinalWidth)/2;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - _margin.w;
                break;

            case 7: // square at left bottom corner
                squareFinalX1 = _margin.x*heightScale;
                squareFinalY1 = 192.0*iuTexScale - squareFinalHeight - _margin.w;
                break;

            case 8: // square at left
                squareFinalX1 = _margin.x*heightScale;
                squareFinalY1 = (192.0*iuTexScale - squareFinalHeight)/2;
        }

        float squareFinalX2 = squareFinalX1 + squareFinalWidth;
        float squareFinalY2 = squareFinalY1 + squareFinalHeight;

        shapeData.squareFinalCoords = {squareFinalX1, squareFinalY1, squareFinalX2, squareFinalY2};
    }

    ShapeData build(float aspectRatio) {
        if (_fromBottomScreen) {
            shapeData.squareInitialCoords.y += 192;
        }

        if (shapeData.shape == 0) { // square
            precompute3DCoordinatesOf2DSquareShape(aspectRatio);
        }

        return shapeData;
    }
private:
    ShapeData shapeData;

    bool _fromBottomScreen;
    int _corner;
    vec4 _margin;
};

}

#endif
