// psf_generator.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psf_font.h"
#include "UnicodeGlyphMap.h"

// Визначення імені файлу без розширення
void GetFontName(const char* filepath, char* fontname, size_t max_len) {
    const char* slash = strrchr(filepath, '/');
    if (!slash) slash = strrchr(filepath, '\\');
    const char* start = slash ? slash + 1 : filepath;
    const char* dot = strrchr(start, '.');
    size_t len = dot ? (size_t)(dot - start) : strlen(start);
    if (len >= max_len) len = max_len - 1;
    strncpy(fontname, start, len);
    fontname[len] = '\0';
}

// Обчислення ширини гліфа і горизонтального зсуву
// width - ширина гліфа (наприклад 6)
// height - висота гліфа (наприклад 12)
// bytes_per_row - кількість байтів на рядок (наприклад для 6 пікселів = 1 байт)
// glyph - масив байтів гліфа
// out_width - фактична ширина активних пікселів
// out_horz_offset - горизонтальний зсув (ліва активна межа)
static void GetGlyphMetrics(const uint8_t* glyph, int width, int height, int bytes_per_row,
                            int* out_width, int* out_horz_offset)
{
    int left_col = width;
    int right_col = -1;

    for (int row = 0; row < height; ++row) {
        uint8_t b = glyph[row * bytes_per_row];
        for (int bit = 0; bit < width; ++bit) { // перевіряємо тільки ширину (6)
            if (b & (0x80 >> bit)) {
                if (bit < left_col) left_col = bit + width;
                if (bit > right_col) right_col = bit;
            }
        }
    }
    if (right_col < left_col) {
        *out_width = 0;
        *out_horz_offset = 0;
    } else {
        *out_width = right_col - left_col + 1;
        *out_horz_offset = left_col;
    }
}

void ExportGlyphsToC(PSF_Font font, const char* input_filename)
{
    char fontname[256];
    GetFontName(input_filename, fontname, sizeof(fontname));

    char filename_c[300], filename_h[300];
    snprintf(filename_c, sizeof(filename_c), "%s.c", fontname);
    snprintf(filename_h, sizeof(filename_h), "%s.h", fontname);

    FILE* out_c = fopen(filename_c, "w");
    FILE* out_h = fopen(filename_h, "w");
    if (!out_c || !out_h) {
        fprintf(stderr, "Не вдалося відкрити файли для запису\n");
        exit(1);
    }

    int bytes_per_glyph = font.charsize;
    int bytes_per_row = (font.width + 7) / 8;
    int max_glyphs = font.charcount;

    int* widths = malloc(max_glyphs * sizeof(int));
    int* heights = malloc(max_glyphs * sizeof(int));
    int* horizontal_offsets = malloc(max_glyphs * sizeof(int));
    int* vertical_offsets = calloc(max_glyphs, sizeof(int));

    if (!widths || !heights || !horizontal_offsets || !vertical_offsets) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

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
            "extern const int %s_glyph_widths[];\n"
            "extern const int %s_glyph_heights[];\n"
            "extern const int %s_glyph_vertical_offsets[];\n"
            "extern const int %s_glyph_horizontal_offsets[];\n\n"
            "extern const RasterFont %s_font;\n\n"
            "#endif // %s_H\n",
            fontname,fontname,fontname,fontname,fontname,
            fontname,fontname,fontname,fontname,fontname,
            fontname,fontname);

    fprintf(out_c, "#include \"%s.h\"\n\n", fontname);
    fprintf(out_c, "const int %s_glyph_width = %d;\n", fontname, font.width);
    fprintf(out_c, "const int %s_glyph_height = %d;\n", fontname, font.height);
    fprintf(out_c, "const int %s_glyph_bytes = %d;\n\n", fontname, bytes_per_glyph);

    int glyphs_exported = 0;
    for (int i = 0; i < max_glyphs; ++i) {
        unsigned char* glyph_data = font.glyphBuffer + i * bytes_per_glyph;

        int w = 0, ho = 0;
        GetGlyphMetrics(glyph_data, font.width, font.height, bytes_per_row, &w, &ho);
        if (w == 0) w = font.width;

        widths[i] = w;
        heights[i] = font.height;
        horizontal_offsets[i] = ho;
        vertical_offsets[i] = 0;

        fprintf(out_c, "static const uint8_t %s_glyph_%03X[%d] = {", fontname, i, bytes_per_glyph);
        for (int b = 0; b < bytes_per_glyph; ++b) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyph_data[b]);
            if (b < bytes_per_glyph - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
        glyphs_exported++;
    }

    fprintf(out_c, "const GlyphPointerMap %s_glyph_ptr_map[] = {\n", fontname);
    for (int i = 32; i < 127; ++i) {
        fprintf(out_c, "    {0x%04X, %s_glyph_%03X},\n", i, fontname, i);
    }
    int cyr_count = sizeof(cyr_map)/sizeof(cyr_map[0]);
    for (int i = 0; i < cyr_count; ++i) {
        uint32_t uc = cyr_map[i].unicode;
        if (uc >= 32 && uc <= 126) continue;
        fprintf(out_c, "    {0x%04X, %s_glyph_%03X},\n", uc, fontname, cyr_map[i].glyph_index);
    }
    fprintf(out_c, "};\n\n");

    // Вивід метаданих
    fprintf(out_c, "const int %s_glyph_widths[%d] = {\n", fontname, glyphs_exported);
    for (int i = 0; i < glyphs_exported; ++i) {
        fprintf(out_c, "    %d%s\n", widths[i], (i == glyphs_exported - 1) ? "" : ",");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_heights[%d] = {\n", fontname, glyphs_exported);
    for (int i = 0; i < glyphs_exported; ++i) {
        fprintf(out_c, "    %d%s\n", heights[i], (i == glyphs_exported - 1) ? "" : ",");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_vertical_offsets[%d] = {\n", fontname, glyphs_exported);
    for (int i = 0; i < glyphs_exported; ++i) {
        fprintf(out_c, "    %d%s\n", vertical_offsets[i], (i == glyphs_exported - 1) ? "" : ",");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_horizontal_offsets[%d] = {\n", fontname, glyphs_exported);
    for (int i = 0; i < glyphs_exported; ++i) {
        fprintf(out_c, "    %d%s\n", horizontal_offsets[i], (i == glyphs_exported - 1) ? "" : ",");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c,
            "const int %s_glyph_ptr_map_count = %d;\n\n"
            "const RasterFont %s_font = {\n"
            "    .name = \"%s\",\n"
            "    .glyph_width = %s_glyph_width,\n"
            "    .glyph_height = %s_glyph_height,\n"
            "    .glyph_bytes = %s_glyph_bytes,\n"
            "    .glyph_map = %s_glyph_ptr_map,\n"
            "    .glyph_count = %s_glyph_ptr_map_count,\n"
            "    .glyph_widths = %s_glyph_widths,\n"
            "    .glyph_heights = %s_glyph_heights,\n"
            "    .glyph_vertical_offsets = %s_glyph_vertical_offsets,\n"
            "    .glyph_horizontal_offsets = %s_glyph_horizontal_offsets\n"
            "};\n",
            fontname, glyphs_exported, fontname,
            fontname,
            fontname, fontname, fontname,
            fontname, fontname,
            fontname, fontname, fontname, fontname, fontname);

    free(widths);
    free(heights);
    free(horizontal_offsets);
    free(vertical_offsets);

    fclose(out_c);
    fclose(out_h);

    printf("Успішно згенеровано %d гліфів шрифту \"%s\"\n", glyphs_exported, fontname);
}

// -----

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Використання: %s <psf-шрифт>\n", argv[0]);
        return 1;
    }

    PSF_Font font = LoadPSFFont(argv[1]);
    ExportGlyphsToC(font, argv[1]);
    UnloadPSFFont(font);
    return 0;
}
