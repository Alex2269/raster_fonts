#ifndef GLYPHS_H
#define GLYPHS_H

#include <stdint.h>
#include "glyphmap.h"

// Структура для опису шрифту повністю
typedef struct {
    const char* name;
    int glyph_width;                     // максимальна ширина гліфа (для сумісності)
    int glyph_height;                    // максимальна висота (фіксована висота шрифту)
    int glyph_bytes;                    // кількість байтів на гліф
    const GlyphPointerMap* glyph_map;   // масив гліфів з їх Unicode кодами
    int glyph_count;                    // кількість гліфів
    const int* glyph_widths;            // масив фактичних ширин кожного гліфа
    const int* glyph_heights;           // масив фактичних висот кожного гліфа
    const int* glyph_vertical_offsets; // масив вертикальних зсувів для вирівнювання по baseline
    const int* glyph_horizontal_offsets; // масив горизонтальних зсувів гліфів
} RasterFont;

// Оголошення функцій
int utf8_strlen(const char* s);
int utf8_decode(const char* str, uint32_t* out_codepoint);
int utf8_encode(uint32_t cp, char* out);

#endif // GLYPHS_H
