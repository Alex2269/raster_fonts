#ifndef myicons_H
#define myicons_H

#include <stdint.h>
#include "glyphmap.h"
#include "glyphs.h"

extern const int myicons_glyph_width;
extern const int myicons_glyph_height;
extern const int myicons_glyph_bytes;

extern const GlyphPointerMap myicons_glyph_ptr_map[];
extern const int myicons_glyph_ptr_map_count;

extern const int myicons_glyph_widths[];
extern const int myicons_glyph_heights[];
extern const int myicons_glyph_vertical_offsets[];
extern const int myicons_glyph_horizontal_offsets[];

extern const RasterFont myicons_font;

#endif // myicons_H
