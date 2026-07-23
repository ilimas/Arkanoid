#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

// Minimal geometry/color value types, replacing SDL_Rect/SDL_Color/SDL_Point
// now that SDL2 is gone - same fields/semantics (top-left origin, y-down),
// so nearly every call site is an unchanged brace-init.
struct Rect
{
    int x = 0, y = 0, w = 0, h = 0;
};

struct Color
{
    uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct Point
{
    int x = 0, y = 0;
};

#endif
