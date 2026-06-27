// raster_viewer.c - консольний переглядач RasterFont іконок
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "glyphmap.h"
#include "glyphs.h"

// Підключаємо згенерований шрифт (myicons.c/myicons.h)
#include "myicons.h"

/*
 * Виводить один гліф у консоль з масштабуванням
 * glyph - вказівник на дані гліфа
 * width - ширина гліфа в пікселях
 * height - висота гліфа в пікселях
 * scale - масштаб (1 = 1 піксель = 1 символ, 2 = 2x2 символи, тощо)
 */
void print_glyph(const uint8_t* glyph, int width, int height, int scale) {
    int bytes_per_row = (width + 7) / 8;
    
    for (int y = 0; y < height; y++) {
        for (int sy = 0; sy < scale; sy++) {  // Вертикальне масштабування
            for (int x = 0; x < width; x++) {
                int byte_idx = y * bytes_per_row + x / 8;
                int bit_idx = 7 - (x % 8);
                int pixel = (glyph[byte_idx] >> bit_idx) & 1;
                
                // Виводимо символ scale разів для горизонтального масштабування
                for (int sx = 0; sx < scale; sx++) {
                    putchar(pixel ? '#' : ' ');
                }
            }
            putchar('\n');
        }
    }
}

/*
 * Виводить метадані гліфа
 */
void print_glyph_info(uint32_t unicode, int width, int height, 
                      int vert_offset, int horiz_offset, int glyph_bytes) {
    printf("Unicode: U+%04X\n", unicode);
    printf("Size: %dx%d pixels\n", width, height);
    printf("Buffer size: %d bytes\n", glyph_bytes);
    printf("Offsets: vertical=%d, horizontal=%d\n", vert_offset, horiz_offset);
    printf("---\n");
}

/*
 * Виводить всі гліфи шрифту
 */
void print_all_glyphs(const RasterFont* font, int scale) {
    printf("Font: %s\n", font->name);
    printf("Base size: %dx%d\n", font->glyph_width, font->glyph_height);
    printf("Glyph count: %d\n", font->glyph_count);
    printf("Buffer size per glyph: %d bytes\n", font->glyph_bytes);
    printf("\n");
    
    for (int i = 0; i < font->glyph_count; i++) {
        uint32_t unicode = font->glyph_map[i].unicode;
        const uint8_t* glyph_data = font->glyph_map[i].glyph;
        
        int width = font->glyph_widths[i];
        int height = font->glyph_heights[i];
        int vert_offset = font->glyph_vertical_offsets[i];
        int horiz_offset = font->glyph_horizontal_offsets[i];
        
        printf("=== Glyph %d/%d ===\n", i + 1, font->glyph_count);
        print_glyph_info(unicode, width, height, vert_offset, horiz_offset, font->glyph_bytes);
        print_glyph(glyph_data, width, height, scale);
        printf("\n\n");
    }
}

/*
 * Виводить один гліф за Unicode кодом
 */
void print_single_glyph(const RasterFont* font, uint32_t unicode, int scale) {
    // Шукаємо гліф у мапі
    const uint8_t* glyph_data = NULL;
    int index = -1;
    
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyph_map[i].unicode == unicode) {
            glyph_data = font->glyph_map[i].glyph;
            index = i;
            break;
        }
    }
    
    if (!glyph_data) {
        printf("Glyph U+%04X not found in font\n", unicode);
        return;
    }
    
    int width = font->glyph_widths[index];
    int height = font->glyph_heights[index];
    int vert_offset = font->glyph_vertical_offsets[index];
    int horiz_offset = font->glyph_horizontal_offsets[index];
    
    printf("Font: %s\n", font->name);
    printf("=== Glyph U+%04X (index %d) ===\n", unicode, index);
    print_glyph_info(unicode, width, height, vert_offset, horiz_offset, font->glyph_bytes);
    print_glyph(glyph_data, width, height, scale);
}

/*
 * Виводить сітку іконок (кілька в ряд)
 */
void print_icon_grid(const RasterFont* font, int icons_per_row, int scale) {
    printf("Font: %s\n", font->name);
    printf("Grid: %d icons per row, scale %dx\n", icons_per_row, scale);
    printf("\n");
    
    int max_height = 0;
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyph_heights[i] > max_height) {
            max_height = font->glyph_heights[i];
        }
    }
    
    // Для кожного рядка пікселів
    for (int row = 0; row < max_height; row++) {
        for (int sy = 0; sy < scale; sy++) {  // Вертикальне масштабування
            for (int i = 0; i < font->glyph_count; i++) {
                uint32_t unicode = font->glyph_map[i].unicode;
                const uint8_t* glyph_data = font->glyph_map[i].glyph;
                int width = font->glyph_widths[i];
                int height = font->glyph_heights[i];
                int bytes_per_row = (width + 7) / 8;
                
                // Виводимо один рядок гліфа (або порожнечу якщо рядок виходить за межі)
                if (row < height) {
                    for (int x = 0; x < width; x++) {
                        int byte_idx = row * bytes_per_row + x / 8;
                        int bit_idx = 7 - (x % 8);
                        int pixel = (glyph_data[byte_idx] >> bit_idx) & 1;
                        
                        for (int sx = 0; sx < scale; sx++) {
                            putchar(pixel ? '#' : ' ');
                        }
                    }
                } else {
                    // Порожній рядок
                    for (int x = 0; x < width * scale; x++) {
                        putchar(' ');
                    }
                }
                
                // Розділювач між іконками
                if (i < font->glyph_count - 1) {
                    for (int s = 0; s < 2 * scale; s++) {
                        putchar(' ');
                    }
                }
            }
            putchar('\n');
        }
    }
    
    // Підписи Unicode кодів
    printf("\nUnicode codes:\n");
    for (int i = 0; i < font->glyph_count; i++) {
        printf("U+%04X", font->glyph_map[i].unicode);
        if (i < font->glyph_count - 1) {
            for (int s = 0; s < 2 * scale + 6; s++) {
                putchar(' ');
            }
        }
    }
    printf("\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s [options]\n", argv[0]);
        printf("Options:\n");
        printf("  -a [scale]     Show all glyphs (default scale: 2)\n");
        printf("  -u <code> [scale]  Show single glyph by Unicode code (hex)\n");
        printf("  -g [icons_per_row] [scale]  Show icon grid\n");
        printf("  -h             Show this help\n");
        printf("\nExamples:\n");
        printf("  %s -a 3        Show all glyphs with 3x scale\n", argv[0]);
        printf("  %s -u 0041 2   Show glyph U+0041 with 2x scale\n", argv[0]);
        printf("  %s -g 5 2      Show grid with 5 icons per row, 2x scale\n", argv[0]);
        return 1;
    }
    
    const RasterFont* font = &myicons_font;
    int scale = 2;  // За замовчуванням 2x
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                scale = atoi(argv[++i]);
            }
            print_all_glyphs(font, scale);
        }
        else if (strcmp(argv[i], "-u") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -u requires Unicode code\n");
                return 1;
            }
            uint32_t unicode;
            sscanf(argv[++i], "%x", &unicode);
            
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                scale = atoi(argv[++i]);
            }
            print_single_glyph(font, unicode, scale);
        }
        else if (strcmp(argv[i], "-g") == 0) {
            int icons_per_row = 5;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                icons_per_row = atoi(argv[++i]);
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                scale = atoi(argv[++i]);
            }
            print_icon_grid(font, icons_per_row, scale);
        }
        else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -a [scale]     Show all glyphs (default scale: 2)\n");
            printf("  -u <code> [scale]  Show single glyph by Unicode code (hex)\n");
            printf("  -g [icons_per_row] [scale]  Show icon grid\n");
            printf("  -h             Show this help\n");
            return 0;
        }
    }
    
    return 0;
}
