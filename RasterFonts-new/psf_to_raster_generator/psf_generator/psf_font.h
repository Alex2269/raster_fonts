#ifndef PSF_FONT_H
#define PSF_FONT_H

#include "raylib.h"
#include <stdint.h>
#include <stddef.h>

// Структура шрифту PSF1/PSF2
typedef struct {
    int isPSF2;               // 0 - PSF1, 1 - PSF2
    int width;                // ширина символу в пікселях
    int height;               // висота символу в пікселях
    int charcount;            // кількість символів (гліфів) у шрифті
    int charsize;             // розмір одного гліфа в байтах
    unsigned char* glyphBuffer;  // вказівник на буфер з бінарними гліфами

    // Додаткові поля для підтримки офсетів (за бажанням)
    int* glyph_horizontal_offsets; // масив горизонтальних зсувів гліфів
    int* glyph_vertical_offsets;   // масив вертикальних зсувів гліфів (якщо потрібен)
} PSF_Font;

// Функції завантаження/звільнення шрифту
PSF_Font LoadPSFFont(const char* filename);
void UnloadPSFFont(PSF_Font font);

// Функції малювання у різних масштабах з урахуванням офсетів
void DrawPSFChar(PSF_Font font, int x, int y, int c, Color color);
void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, Color color);
void DrawPSFCharScaled(PSF_Font font, int x, int y, int c, int scale, Color color);
void DrawPSFTextScaled(PSF_Font font, int x, int y, const char* text, int spacing, int scale, Color color);

// Підрахунок довжини UTF-8 рядка в символах
int utf8_strlen(const char* s);

#endif // PSF_FONT_H
