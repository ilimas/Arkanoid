// Single translation unit providing the implementation for the vendored
// single-header stb libraries (each header may only have its IMPLEMENTATION
// macro defined once across the whole program).
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
