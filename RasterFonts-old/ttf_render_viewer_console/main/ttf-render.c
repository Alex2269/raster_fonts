// ttf-render.c

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char* utf8_to_codepoint(const char* str, uint32_t* codepoint) {
    unsigned char c = (unsigned char)*str;
    if (c < 0x80) {
        *codepoint = c;
        return str + 1;
    } else if ((c & 0xE0) == 0xC0) {
        *codepoint = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        return str + 2;
    } else if ((c & 0xF0) == 0xE0) {
        *codepoint = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return str + 3;
    } else {
        *codepoint = 0; // не підтримуємо 4-байтні символи
        return str + 1;
    }
}

void blit_glyph(uint8_t *bitmap, int bitmap_width, int bitmap_height,
                FT_Bitmap *bmp, int pen_x, int pen_y, int bitmap_left, int bitmap_top)
{
    int bytes_per_row = (bitmap_width + 7) / 8;
    for (int y = 0; y < bmp->rows; y++) {
        int dest_y = pen_y - bitmap_top + y;
        if (dest_y < 0 || dest_y >= bitmap_height) continue;
        for (int x = 0; x < bmp->width; x++) {
            unsigned char pixel = bmp->buffer[y * bmp->pitch + x];
            if (pixel > 128) {
                int dest_x = pen_x + bitmap_left + x;
                if (dest_x < 0 || dest_x >= bitmap_width) continue;
                int byte_idx = dest_y * bytes_per_row + (dest_x / 8);
                int bit_idx = 7 - (dest_x % 8);
                bitmap[byte_idx] |= (1 << bit_idx);
            }
        }
    }
}

void print_bitmap(const uint8_t *bitmap, int bitmap_width, int bitmap_height)
{
    int bytes_per_row = (bitmap_width + 7) / 8;
    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            int byte_idx = y * bytes_per_row + (x / 8);
            int bit_idx = 7 - (x % 8);
            putchar((bitmap[byte_idx] & (1 << bit_idx)) ? '#' : ' ');
        }
        putchar('\n');
    }
}

int main(int argc, char **argv) {
    if (argc < 6) {
        printf("Usage: %s <font.ttf> <glyph_width> <glyph_height> <max_width_px> <text>\n", argv[0]);
        return 1;
    }

    const char *fontfile = argv[1];
    int glyph_width = atoi(argv[2]);
    int glyph_height = atoi(argv[3]);
    int max_width_px = atoi(argv[4]);
    const char *text = argv[5];

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Failed to initialize FreeType\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontfile, 0, &face)) {
        fprintf(stderr, "Failed to load font %s\n", fontfile);
        FT_Done_FreeType(ft);
        return 1;
    }

    FT_Set_Pixel_Sizes(face, glyph_width, glyph_height);
    int bytes_per_row = (max_width_px + 7) / 8;
    int bitmap_height = glyph_height * 10; // достатньо для ~10 рядків
    uint8_t *bitmap = calloc(bitmap_height * bytes_per_row, 1);
    if (!bitmap) {
        fprintf(stderr, "Out of memory\n");
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return 1;
    }

    int pen_x = 0;
    int pen_y = glyph_height;
    const char *p = text;
    uint32_t codepoint;

    while (*p) {
        if (*p == '\n') {
            pen_x = 0;
            pen_y += glyph_height;
            p++;
            continue;
        }

        p = utf8_to_codepoint(p, &codepoint);
        if (codepoint == 0)
            continue;

        if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
            fprintf(stderr, "Failed to load char U+%04X\n", codepoint);
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        if (pen_x + (g->advance.x >> 6) > max_width_px) {
            pen_x = 0;
            pen_y += glyph_height;
        }
        if (pen_y >= bitmap_height) {
            fprintf(stderr, "Bitmap height limit reached\n");
            break;
        }

        blit_glyph(bitmap, max_width_px, bitmap_height,
                   &g->bitmap, pen_x, pen_y, g->bitmap_left, g->bitmap_top);

        pen_x += g->advance.x >> 6;
    }

    print_bitmap(bitmap, max_width_px, bitmap_height);

    free(bitmap);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}

