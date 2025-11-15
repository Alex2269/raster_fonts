// GlyphCache.h
#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include "raylib.h"
#include <stdint.h>
#include "psf_font.h"  // Структура PSF_Font

// Структура кешу текстур гліфів
typedef struct {
    Texture2D* glyphTextures;  // Масив текстур гліфів
    int glyph_count;           // Кількість гліфів (розмір масиву)
} GlyphCache;

// Ініціалізація кешу (виділення пам’яті, обнулення)
void GlyphCache_Init(GlyphCache* cache, int glyph_count);

// Звільнення ресурсів кешу (текстур і пам’яті)
void GlyphCache_Unload(GlyphCache* cache);

// Отримання текстури гліфа з кешу, створення при відсутності
Texture2D GlyphCache_GetTexture(GlyphCache* cache, PSF_Font font, int glyphIndex, float scale);

// Малювання текстури гліфа з масштабуванням і кольором
void DrawPSFCharScaledTexture(Texture2D tex, int x, int y, float scale, Color color);

// Малювання UTF-8 тексту з динамічним кешем текстур гліфів,
// який підтримує одночасну роботу з багатьма шрифтами
void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, float scale, Color color);

// Звільнення всіх кешів, створених для різних шрифтів
void GlyphCache_ClearAllCaches(void);


#endif // GLYPH_CACHE_H

