#ifndef FreePixel_H
#define FreePixel_H

#include <stdint.h>
#include "glyphmap.h"
#include "glyphs.h"

extern const int FreePixel_glyph_height;
extern const int FreePixel_glyph_bytes;

extern const GlyphPointerMap FreePixel_glyph_ptr_map[];
extern const int FreePixel_glyph_ptr_map_count;

extern const int FreePixel_glyph_widths[];
extern const int FreePixel_glyph_heights[];
extern const int FreePixel_glyph_vertical_offsets[];

extern const RasterFont FreePixel_font;

#endif // FreePixel_H
