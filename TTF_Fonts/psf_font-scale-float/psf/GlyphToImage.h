// GlyphToImage.h
#ifndef GLYPH_TO_IMAGE_H
#define GLYPH_TO_IMAGE_H

#include "raylib.h"
#include "psf_font.h"
#include <stdint.h>

// Створює Image з одного гліфа PSF шрифту з заданим кольором
Image GlyphToImage(PSF_Font font, int glyphIndex, Color color);

// Конвертує Image у Texture2D з вибором фільтра залежно від масштабу
Texture2D GlyphToTexture(PSF_Font font, int glyphIndex, float scale, Color color);

#endif // GLYPH_TO_IMAGE_H

