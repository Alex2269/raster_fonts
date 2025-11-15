// glyphs.c

#include <stdio.h>
#include <string.h>

#include "glyphs.h"    // структури RasterFont та glyph_map
#include "graphics.h"  // DrawRectangle, тощо
#include "gfx.h"       // DrawPixel

/*
 * utf8_strlen - підрахунок кількості Unicode символів у UTF-8 рядку.
 * Стандартна strlen рахує байти, а не символи, що може спотворювати довжину тексту,
 * особливо для кирилиці та інших багатобайтових символів.
 */
int utf8_strlen(const char* s) {
    int len = 0;
    // Проходимо по рядку, декодуючи кожен UTF-8 символ
    while (*s) {
        uint32_t codepoint = 0;
        int bytes = utf8_decode(s, &codepoint); // розпаковуємо один символ
        s += bytes; // переходимо до наступного символу
        len++; // лічильник символів
    }
    return len;
}

/*
 * utf8_decode - декодування одного UTF-8 символу з початку рядка str.
 * Записує decodед Unicode код символу у out_codepoint.
 * Повертає довжину символу у байтах (1-4).
 * Якщо символ некоректний, повертає 1 і код 0.
 */
int utf8_decode(const char* str, uint32_t* out_codepoint) {
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80) {
        // Однобайтовий ASCII символ
        *out_codepoint = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        // Дво-байтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        // Трибайтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        // Чотири байтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) |
        ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        return 4;
    }
    // Некоректний символ
    *out_codepoint = 0;
    return 1;
}


// Малюємо один гліф з урахуванням зсувів і масштабу
// glyph - масив байтів гліфа (монохромний)
// width, height - розміри гліфа (ширина, висота в пікселях)
// bytes_per_glyph - розмір гліфа в байтах (ширина * висота / 8)
// x, y - стартова позиція на екрані
// scale - масштаб (розмір пікселя)
// vert_offset, horiz_offset - вертикальний і горизонтальний зсуви гліфа (пікселі)
// color - колір пікселя
void DrawGlyphWithOffsets(const uint8_t* glyph, int width, int height, int bytes_per_glyph,
                          int x, int y, int scale, int vert_offset, int horiz_offset, uint32_t color)
{
    int bytes_per_row = (width + 7) / 8;
    int draw_y = y + vert_offset * scale;
    int draw_x_base = x + horiz_offset * scale;

    for (int row = 0; row < height; ++row) {
        for (int byte = 0; byte < bytes_per_row; ++byte) {
            uint8_t byte_val = glyph[row * bytes_per_row + byte];
            for (int bit = 0; bit < 8; ++bit) {
                int px = byte * 8 + bit;
                if (px >= width) break;
                if (byte_val & (0x80 >> bit)) {
                    int draw_x = draw_x_base + px * scale;
                    int draw_y_pixel = draw_y + row * scale;
                    for (int dx = 0; dx < scale; ++dx) {
                        for (int dy = 0; dy < scale; ++dy) {
                            DrawPixel(draw_x + dx, draw_y_pixel + dy, color);
                        }
                    }
                }
            }
        }
    }
}

// Пошук гліфа за Unicode кодом із вашого шрифту
const GlyphPointerMap* FindGlyph(const RasterFont font, uint32_t unicode)
{
    for (int i = 0; i < font.glyph_count; ++i) {
        if (font.glyph_map[i].unicode == unicode) {
            return &font.glyph_map[i];
        }
    }
    return NULL;
}

// Малювання символу з урахуванням індивідуальних розмірів і зсувів
void DrawChar(const RasterFont font, int x, int y, uint32_t codepoint,
              uint32_t color, int scale)
{
    const GlyphPointerMap* glyph_ptr = FindGlyph(font, codepoint);
    if (!glyph_ptr) return;

    int glyph_index = (int)(glyph_ptr - font.glyph_map);

    int width = font.glyph_widths[glyph_index];
    int height = font.glyph_heights[glyph_index];
    int vert_offset = font.glyph_vertical_offsets[glyph_index];
    int horiz_offset = font.glyph_horizontal_offsets[glyph_index];  // <-- використовуємо горизонтальний зсув

    DrawGlyphWithOffsets(glyph_ptr->glyph, width, height, font.glyph_bytes,
                         x + horiz_offset * scale, y + vert_offset * scale,
                         scale, 0, 0, color);
}

void DrawTextScaled(const RasterFont font, int x, int y, const char* text,
                    int spacing, int scale, uint32_t color)
{
    int xpos = x;
    int ypos = y;
    while (*text) {
        if (*text == '\n') {
            xpos = x;
            ypos += font.glyph_height * scale + spacing;
            text++;
            continue;
        }
        uint32_t codepoint = 0;
        int bytes = utf8_decode(text, &codepoint);

        const GlyphPointerMap* glyph = FindGlyph(font, codepoint);
        if (!glyph) glyph = FindGlyph(font, 32); // заміна на пробіл, якщо не знайдено

        if (glyph) {
            int glyph_index = (int)(glyph - font.glyph_map);
            int w = font.glyph_widths[glyph_index];
            int h = font.glyph_heights[glyph_index];
            int vo = font.glyph_vertical_offsets[glyph_index];
            int ho = 0; // Додайте горизонтальний зсув, якщо є

            DrawGlyphWithOffsets(glyph->glyph, w, h, font.glyph_bytes,
                                 xpos, ypos, scale, vo, ho, color);

            xpos += (w * scale) + spacing;
        }
        text += bytes;
    }
}

/*
 * DrawTextWithBackground - малює текст із фоновим прямокутником та рамкою.
 * Параметри:
 * - font: шрифт для малювання
 * - x, y: позиція початку тексту
 * - text: текст для малювання (підтримує \n)
 * - spacing: відступ між символами
 * - scale: масштаб символів
 * - textColor, bgColor, borderColor: кольори тексту, фону і рамки
 * - padding: внутрішній відступ між текстом і рамкою
 * - borderThickness: товщина рамки (число проходів малювання прямокутника)
 */
void DrawTextWithBackground(const RasterFont font, int x, int y, const char* text,
                            int spacing, int scale, uint32_t textColor,
                            uint32_t bgColor, uint32_t borderColor,
                            int padding, int borderThickness)
{
    // Розбиваємо текст на рядки для посторочного малювання
    const char* lines[20];
    int lineCount = 0;
    char tempText[512];
    strncpy(tempText, text, sizeof(tempText) - 1);
    tempText[sizeof(tempText) - 1] = '\0';

    char* line = strtok(tempText, "\n");
    while (line != NULL && lineCount < 20) {
        lines[lineCount++] = line;
        line = strtok(NULL, "\n");
    }

    // Обчислюємо максимальну ширину рядка в пікселях з урахуванням ширини кожного гліфа
    int maxLineWidth = 0;
    for (int i = 0; i < lineCount; i++) {
        int lineWidth = 0;
        const char* ptr = lines[i];
        while (*ptr) {
            uint32_t cp = 0;
            int bytes = utf8_decode(ptr, &cp);
            const GlyphPointerMap* glyph = FindGlyph(font, cp);
            if (!glyph) glyph = FindGlyph(font, 32); // пропуск, якщо не знайдений

            if (glyph) {
                int glyph_index = glyph - font.glyph_map;
                int gw = font.glyph_widths[glyph_index];
                lineWidth += (gw * scale) + spacing;
            }
            ptr += bytes;
        }
        if (lineWidth > 0) lineWidth -= spacing; // прибираємо зайвий інтервал у кінці
        if (lineWidth > maxLineWidth) maxLineWidth = lineWidth;
    }

    // Обчислюємо висоту фону із урахуванням рядків, масштабу, паддінгу, отступів і товщини рамки
    int lineHeight = font.glyph_height * scale;
    int totalHeight = (lineHeight * lineCount) + (spacing * (lineCount - 1));
    int bgWidth = maxLineWidth + 2 * padding + 2 * borderThickness;
    int bgHeight = totalHeight + 2 * padding + 2 * borderThickness;

    // Малюємо залитий фон
    DrawRectangle(x - padding - borderThickness, y - padding - borderThickness, bgWidth, bgHeight, bgColor);

    // Малюємо рамку товщиною borderThickness
    for (int i = 0; i < borderThickness; i++) {
        DrawRectangleLines(x - padding - borderThickness + i, y - padding - borderThickness + i,
                           bgWidth - 2 * i, bgHeight - 2 * i, borderColor);
    }

    // Малюємо текст поверх фону і рамки по рядках
    int ypos = y;
    for (int i = 0; i < lineCount; i++) {
        DrawTextScaled(font, x, ypos, lines[i], spacing, scale, textColor);
        ypos += lineHeight + spacing;
    }
}

/*
 * DrawTextWithAutoInvertedBackground - малює текст з фоном,
 * колір якого обирається автоматично як контрастна інверсія кольору тексту.
 * Використовує DrawTextWithBackground, але фон вибирається автоматично.
 */
void DrawTextWithAutoInvertedBackground(const RasterFont font, int x, int y, const char* text,
                                        int spacing, int scale, uint32_t textColor,
                                        int padding, int borderThickness) {
    // Підбираємо фон як інверсний і контрастний до textColor
    uint32_t bgColor = GetContrastInvertColor(textColor);
    // Викликаємо основну функцію з автоматичним фоном і рамкою кольору textColor
    DrawTextWithBackground(font, x, y, text, spacing, scale, textColor, bgColor, textColor, padding, borderThickness);
}

