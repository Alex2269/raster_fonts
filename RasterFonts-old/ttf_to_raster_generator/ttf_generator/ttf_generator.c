// ttf_generator.c

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

typedef struct {
    uint32_t unicode;
    const uint8_t *glyph;
} GlyphPointerMap;

typedef struct {
    uint32_t unicode;
    unsigned char *data;
} GlyphData;

// Функція рендерингу гліфа у 1-бітний буфер з центрованим позиціюванням
void process_codepoint(FT_Face face, uint32_t codepoint, int glyph_width, int glyph_height,
                       unsigned char* glyph_data_buffer, int charsize)
{
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
        memset(glyph_data_buffer, 0, charsize);
        return;
    }

    FT_Bitmap* bmp = &face->glyph->bitmap;
    memset(glyph_data_buffer, 0, charsize);

    // Горизонтальне центрування
    int offset_x = (glyph_width - bmp->width) / 2;
    if (offset_x < 0) offset_x = 0;

    // Вертикальне позиціювання по baseline плюс зсув вгору (-shift)
    int baseline_offset = glyph_height - face->glyph->bitmap_top;

    int shift_up = roundf((float)glyph_height/4.0f); // підняти гліф на 1/4 гліфа вгору, можна регулювати

    int offset_y = baseline_offset - shift_up;
    if (offset_y < 0) offset_y = 0; // не виходити за межі

    for (int row = 0; row < bmp->rows; row++) {
        int target_row = row + offset_y;
        if (target_row < 0 || target_row >= glyph_height) continue;

        for (int col = 0; col < bmp->width && (col + offset_x) < glyph_width; col++) {
            unsigned char pixel = bmp->buffer[row * bmp->width + col];
            if (pixel > 128) {
                int byte_index = target_row * ((glyph_width + 7) / 8) + ((col + offset_x) / 8);
                int bit_index = 7 - ((col + offset_x) % 8);
                glyph_data_buffer[byte_index] |= (1 << bit_index);
            }
        }
    }
}

// Функція для створення імені шрифта з вхідного файлу без розширення та шляху
void GetFontName(const char* filepath, char* fontname, size_t max_len)
{
    const char* slash = strrchr(filepath, '/');
    if (!slash)
        slash = strrchr(filepath, '\\');
    const char* start = slash ? slash + 1 : filepath;
    const char* dot = strrchr(start, '.');
    size_t len = dot ? (size_t)(dot - start) : strlen(start);
    if (len >= max_len) len = max_len - 1;
    strncpy(fontname, start, len);
    fontname[len] = '\0';
}

// Генератор файлів
void ExportGlyphsToC(FT_Face face, int glyph_width, int glyph_height, const char* input_filename) {
    char fontname[256];
    GetFontName(input_filename, fontname, sizeof(fontname));

    char filename_c[300], filename_h[300];
    snprintf(filename_c, sizeof(filename_c), "%s.c", fontname);
    snprintf(filename_h, sizeof(filename_h), "%s.h", fontname);

    FILE* out_c = fopen(filename_c, "w");
    FILE* out_h = fopen(filename_h, "w");
    if (!out_c || !out_h) {
        fprintf(stderr, "Failed to open output files.\n");
        exit(1);
    }

    int charsize = ((glyph_width + 7) / 8) * glyph_height;

    // Заголовочний файл
    fprintf(out_h,
            "#ifndef %s_H\n#define %s_H\n\n"
            "#include <stdint.h>\n"
            "#include \"glyphmap.h\"\n"
            "#include \"glyphs.h\"\n\n"
            "extern const int %s_glyph_width;\n"
            "extern const int %s_glyph_height;\n"
            "extern const int %s_glyph_bytes;\n\n"
            "extern const GlyphPointerMap %s_glyph_ptr_map[];\n"
            "extern const int %s_glyph_ptr_map_count;\n\n"
            "extern const RasterFont %s_font;\n\n"
            "#endif // %s_H\n",
            fontname, fontname,
            fontname, fontname, fontname,
            fontname, fontname,
            fontname,
            fontname);

    // Початок C файлу з константами
    fprintf(out_c, "#include \"%s.h\"\n\n", fontname);
    fprintf(out_c, "const int %s_glyph_width = %d;\n", fontname, glyph_width);
    fprintf(out_c, "const int %s_glyph_height = %d;\n", fontname, glyph_height);
    fprintf(out_c, "const int %s_glyph_bytes = %d;\n\n", fontname, charsize);

    GlyphData* glyphs = malloc(512 * sizeof(GlyphData));
    if (!glyphs) { fprintf(stderr, "Out of memory\n"); exit(1); }
    unsigned char* glyph_buffer = malloc(charsize);
    if (!glyph_buffer) { free(glyphs); fprintf(stderr, "Out of memory\n"); exit(1); }

    int glyph_count = 0;

    // Обробка ASCII (32..126)
    for (uint32_t cp = 32; cp <= 126; ++cp) {
        process_codepoint(face, cp, glyph_width, glyph_height, glyph_buffer, charsize);
        glyphs[glyph_count].unicode = cp;
        glyphs[glyph_count].data = malloc(charsize);
        if (!glyphs[glyph_count].data) { fprintf(stderr, "Out of memory\n"); exit(1); }
        memcpy(glyphs[glyph_count].data, glyph_buffer, charsize);
        glyph_count++;
    }

    // Обробка кирилиці (U+0400..U+04FF)
    for (uint32_t cp = 0x0400; cp <= 0x04FF; ++cp) {
        process_codepoint(face, cp, glyph_width, glyph_height, glyph_buffer, charsize);
        glyphs[glyph_count].unicode = cp;
        glyphs[glyph_count].data = malloc(charsize);
        if (!glyphs[glyph_count].data) { fprintf(stderr, "Out of memory\n"); exit(1); }
        memcpy(glyphs[glyph_count].data, glyph_buffer, charsize);
        glyph_count++;
    }

    free(glyph_buffer);

    // Запис гліфів у C-масиви
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "static const uint8_t %s_glyph_%04X[%d] = {", fontname, glyphs[i].unicode, charsize);
        for (int b = 0; b < charsize; ++b) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyphs[i].data[b]);
            if (b < charsize - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
    }

    // Таблиця GlyphPointerMap
    fprintf(out_c, "const GlyphPointerMap %s_glyph_ptr_map[] = {\n", fontname);
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "    {0x%04X, %s_glyph_%04X},\n", glyphs[i].unicode, fontname, glyphs[i].unicode);
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_ptr_map_count = %d;\n\n", fontname, glyph_count);

    // Структура RasterFont
    fprintf(out_c,
            "const RasterFont %s_font = {\n"
            "    .name = \"%s\",\n"
            "    .glyph_width = %s_glyph_width,\n"
            "    .glyph_height = %s_glyph_height,\n"
            "    .glyph_bytes = %s_glyph_bytes,\n"
            "    .glyph_map = %s_glyph_ptr_map,\n"
            "    .glyph_count = %s_glyph_ptr_map_count\n"
            "};\n",
            fontname, fontname,
            fontname, fontname, fontname,
            fontname, fontname);

    for (int i=0; i < glyph_count; ++i) {
        free(glyphs[i].data);
    }
    free(glyphs);

    fclose(out_c);
    fclose(out_h);

    printf("Generated %d glyphs for font %s in %s and %s\n",
           glyph_count, fontname, filename_c, filename_h);
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: %s <ttf_font_file> <glyph_width> <glyph_height>\n", argv[0]);
        return 1;
    }

    const char* font_file = argv[1];
    int glyph_width = atoi(argv[2]);
    int glyph_height = atoi(argv[3]);

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Failed to init FreeType library\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_file, 0, &face)) {
        fprintf(stderr, "Failed to load font: %s\n", font_file);
        FT_Done_FreeType(ft);
        return 1;
    }

    FT_Set_Pixel_Sizes(face, glyph_width, glyph_height);

    ExportGlyphsToC(face, glyph_width, glyph_height, font_file);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}

