#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "UnicodeGlyphMap.h" // таблиця Unicode->гліф

typedef struct {
    uint8_t magic[4];
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t length;
    uint32_t glyph_bytes;
    uint32_t glyph_height;
    uint32_t glyph_width;
} PSF2_Header;

typedef struct {
    PSF2_Header header;
    unsigned char* glyphs;
} PSF2_Font;

// Читаємо 4-байтне little-endian число з файлу
static uint32_t ReadLE32(FILE* f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

int LoadPSF2Font(const char* filename, PSF2_Font* font) {
    FILE* f = fopen(filename, "rb");
    if (!f) { perror("Open PSF2 font"); return 0; }

    fread(font->header.magic, 1, 4, f);
    if (font->header.magic[0] != 0x72 || font->header.magic[1] != 0xB5 ||
        font->header.magic[2] != 0x4A || font->header.magic[3] != 0x86) {
        printf("Not a PSF2 font file\n");
    fclose(f);
    return 0;
        }

        font->header.version = ReadLE32(f);
        font->header.headersize = ReadLE32(f);
        font->header.flags = ReadLE32(f);
        font->header.length = ReadLE32(f);
        font->header.glyph_bytes = ReadLE32(f);
        font->header.glyph_height = ReadLE32(f);
        font->header.glyph_width = ReadLE32(f);

        fseek(f, font->header.headersize, SEEK_SET);

        font->glyphs = malloc(font->header.length * font->header.glyph_bytes);
        if (!font->glyphs) {
            fclose(f);
            printf("Memory allocation failed\n");
            return 0;
        }
        fread(font->glyphs, 1, font->header.length * font->header.glyph_bytes, f);
        fclose(f);
        return 1;
}

// Функція UTF-8 декодування символу у Unicode кодпоінт
static int utf8_decode(const char* str, uint32_t* out_codepoint) {
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80) {
        *out_codepoint = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        *out_codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        *out_codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        *out_codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        return 4;
    }
    *out_codepoint = 0;
    return 1;
}

static int UnicodeToGlyphIndex(uint32_t codepoint) {
    if (codepoint >= 32 && codepoint <= 126) return codepoint;
    for (int i = 0; i < sizeof(cyr_map)/sizeof(cyr_map[0]); i++) {
        if (cyr_map[i].unicode == codepoint) return cyr_map[i].glyph_index;
    }
    return 32; // замінити на пробіл якщо не знайдено
}

void PrintTextLine(PSF2_Font* font, const char* text) {
    int bytes_per_row = (font->header.glyph_width + 7) / 8;

    uint32_t codepoints[256];
    int len = 0;
    const char* p = text;

    // Декодуємо UTF-8 у масив кодпоінтів
    while (*p && len < 256) {
        uint32_t cp;
        int bytes = utf8_decode(p, &cp);
        codepoints[len++] = cp;
        p += bytes;
    }

    // Виводимо пікселі рядок за рядком для всіх символів
    for (int row = 0; row < font->header.glyph_height; row++) {
        for (int i = 0; i < len; i++) {
            int glyph_idx = UnicodeToGlyphIndex(codepoints[i]);

            if (glyph_idx < 0) glyph_idx = 32; // завжди заміна на пробіл, щоб не припиняти вивід
            unsigned char* glyph = font->glyphs + glyph_idx * font->header.glyph_bytes;

            for (int bit = 0; bit < font->header.glyph_width; bit++) {
                int byte_idx = row * bytes_per_row + bit / 8;
                int bit_idx = 7 - (bit % 8);
                if (glyph[byte_idx] & (1 << bit_idx)) putchar('#');
                else putchar(' ');
            }
            putchar(' ');
        }
        putchar('\n');
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s font.psf \"text\"\n", argv[0]);
        return 1;
    }

    PSF2_Font font;
    if (!LoadPSF2Font(argv[1], &font)) return 1;

    PrintTextLine(&font, argv[2]);

    free(font.glyphs);
    return 0;
}

