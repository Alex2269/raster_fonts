#ifndef Pixel_H
#define Pixel_H

#include <stdint.h>
#include "glyphmap.h"
#include "glyphs.h"

extern const int Pixel_glyph_width;
extern const int Pixel_glyph_height;
extern const int Pixel_glyph_bytes;

extern const GlyphPointerMap Pixel_glyph_ptr_map[];
extern const int Pixel_glyph_ptr_map_count;

extern const RasterFont Pixel_font;

#endif // Pixel_H
