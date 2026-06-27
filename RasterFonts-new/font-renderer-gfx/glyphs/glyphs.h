#ifndef GLYPHS_H
#define GLYPHS_H

#include <stdint.h>
#include "glyphmap.h"
#include "color_utils.h"

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

const GlyphPointerMap* FindGlyph(const RasterFont font, uint32_t unicode);

void DrawGlyph(const uint8_t* glyph, int charsize, int width, int height,
               int x, int y, uint32_t color);

void DrawGlyphScaled(const uint8_t* glyph, int width, int height, int bytes_per_glyph,
                     int x, int y, int scale, uint32_t color);

void DrawChar(const RasterFont font, int x, int y, uint32_t codepoint, uint32_t color, int scale);

void DrawTextScaled(const RasterFont font, int x, int y, const char* text,
                    int spacing, int scale, uint32_t color);

void DrawTextWithBackground(const RasterFont font, int x, int y, const char* text,
                            int spacing, int scale, uint32_t textColor,
                            uint32_t bgColor, uint32_t borderColor,
                            int padding, int borderThickness);

void DrawTextWithAutoInvertedBackground(const RasterFont font, int x, int y, const char* text,
                                        int spacing, int scale, uint32_t textColor,
                                        int padding, int borderThickness);

#endif // GLYPHS_H
