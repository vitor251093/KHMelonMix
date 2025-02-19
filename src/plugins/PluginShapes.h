#ifndef PLUGIN_SHAPES_H
#define PLUGIN_SHAPES_H

namespace Plugins
{

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
struct alignas(16) ShapeData {
    int enabled;      // 4 bytes (bool is not std140-safe, so we use int)

    int shape;        // 4 bytes
    int corner;       // 4 bytes
    float scale;      // 4 bytes

    ivec4 square;      // 16 bytes (X, Y, Width, Height)
    ivec4 freeForm[4]; // 4 * 8 bytes = 32 bytes
    vec4 margin;       // 16 bytes (left, top, right, down)

    vec4 fadeBorderSize;       // 16 bytes (left fade, top fade, right fade, down fade)
    int invertGrayScaleColors; // 4 bytes (bool -> int for std140)
    int _pad0, _pad1, _pad2;   // Padding to align the struct to 16 bytes

    vec4 cropSquareCorners;

    ivec4 colorToAlpha;       // 16 bytes (RGBA, just ignore the A)
    ivec4 singleColorToAlpha; // 16 bytes (RGBA, just ignore the A)
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
        shapeBuilder.shapeData.enabled = 1;
        shapeBuilder.shapeData.shape = 0;
        shapeBuilder.shapeData.corner = corner_Center;
        shapeBuilder.shapeData.scale = 1.0;
        shapeBuilder.shapeData.square.x = 0;
        shapeBuilder.shapeData.square.y = 0;
        shapeBuilder.shapeData.square.z = 256;
        shapeBuilder.shapeData.square.w = 192;
        shapeBuilder.shapeData.margin.x = 0;
        shapeBuilder.shapeData.margin.y = 0;
        shapeBuilder.shapeData.margin.z = 0;
        shapeBuilder.shapeData.margin.w = 0;
        shapeBuilder._fromBottomScreen = false;
        return shapeBuilder;
    }

    ShapeBuilder& fromBottomScreen() {
        _fromBottomScreen = true;
        return *this;
    }
    ShapeBuilder& placeAtCorner(int _corner) {
        shapeData.corner = _corner;
        return *this;
    }
    ShapeBuilder& scale(float _scale) {
        shapeData.scale = _scale;
        return *this;
    }
    ShapeBuilder& preserveDsScale() {
        shapeData.scale = 0.0;
        return *this;
    }
    ShapeBuilder& fromPosition(int x, int y) {
        if (shapeData.shape == 0) {
            shapeData.square.x = x;
            shapeData.square.y = y;
        }
        return *this;
    }
    ShapeBuilder& withSize(int width, int height) {
        if (shapeData.shape == 0) {
            shapeData.square.z = width;
            shapeData.square.w = height;
        }
        return *this;
    }
    ShapeBuilder& withMargin(float left, float top, float right, float bottom) {
        shapeData.margin.x = left;
        shapeData.margin.y = top;
        shapeData.margin.z = right;
        shapeData.margin.w = bottom;
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

    ShapeData build() {
        if (_fromBottomScreen) {
            shapeData.square.y += 192;
        }
        return shapeData;
    }
private:
    ShapeData shapeData;
    bool _fromBottomScreen;
};

}

#endif
