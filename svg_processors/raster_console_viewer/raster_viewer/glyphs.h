#ifndef GLYPHS_H
#define GLYPHS_H
#include <stdint.h>

typedef struct {
    const char* name;
    int glyph_width;
    int glyph_height;
    int glyph_bytes;
    const GlyphPointerMap* glyph_map;
    int glyph_count;
    const int* glyph_widths;
    const int* glyph_heights;
    const int* glyph_vertical_offsets;
    const int* glyph_horizontal_offsets;
} RasterFont;

#endif // GLYPHS_H
