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
struct ShapeData {
    int enabled;      // 4 bytes (bool is not std140-safe, so we use int)
    int shape;        // 4 bytes
    int corner;       // 4 bytes
    float scale;      // 4 bytes

    ivec4 square;   // 16 bytes (X, Y, Width, Height)
    ivec2 freeForm[4]; // 4 * 8 bytes = 32 bytes

    vec4 margin;        // 16 bytes (left, top, right, down)
    vec4 fadeBorderSize; // 16 bytes (left fade, top fade, right fade, down fade)

    int invertGrayScaleColors; // 4 bytes (bool -> int for std140)
    ivec3 colorToAlpha;   // 12 bytes (RGB)

    int _pad1; // 4 bytes padding to ensure struct size is multiple of 16 bytes
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

class ShapeBuilder
{
public:
    static ShapeBuilder square() {
        auto shapeBuilder = ShapeBuilder();
        shapeBuilder.shapeData.enabled = 1;
        shapeBuilder.shapeData.shape = 0;
        shapeBuilder.shapeData.scale = 1.0;
        return shapeBuilder;
    }

    ShapeBuilder& corner(int _corner) {
        shapeData.corner = _corner;
        return *this;
    }
    ShapeBuilder& scale(float _scale) {
        shapeData.scale = _scale;
        return *this;
    }
    ShapeBuilder& position(int x, int y) {
        if (shapeData.shape == 0) {
            shapeData.square.x = x;
            shapeData.square.y = y;
        }
        return *this;
    }
    ShapeBuilder& size(int width, int height) {
        if (shapeData.shape == 0) {
            shapeData.square.z = width;
            shapeData.square.w = height;
        }
        return *this;
    }
    ShapeBuilder& margin(float left, float top, float right, float bottom) {
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

    ShapeData build() {
        return shapeData;
    }
private:
    ShapeData shapeData;
};

}

#endif
