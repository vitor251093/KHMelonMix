#ifndef PLUGIN_SHAPES_H
#define PLUGIN_SHAPES_H

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

#endif
