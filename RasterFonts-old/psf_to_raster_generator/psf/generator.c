// generator.c

#include <stdio.h>
#include <string.h>
#include "psf_font.h"    // Ваша структура PSF шрифту
#include "UnicodeGlyphMap.h" // Ваша таблиця відповідності кирилиці

// Функція для вилучення чистого імені шрифту з файлу (без розширення і шляху)
void GetFontName(const char* filepath, char* fontname, size_t max_len) {
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

// Головна функція експорту шрифту в C і H файли
void ExportGlyphsToC(PSF_Font font, const char* input_filename) {
    char fontname[256];
    GetFontName(input_filename, fontname, sizeof(fontname));

    // Формування імен файлів .c і .h на базі назви шрифту
    char filename_c[300];
    char filename_h[300];
    snprintf(filename_c, sizeof(filename_c), "%s.c", fontname);
    snprintf(filename_h, sizeof(filename_h), "%s.h", fontname);

    FILE* out_c = fopen(filename_c, "w");
    FILE* out_h = fopen(filename_h, "w");
    if (!out_c || !out_h) {
        printf("Не вдалося відкрити файл(и) для запису: %s, %s\n", filename_c, filename_h);
        return;
    }

    // ------------ Генерація заголовочного файлу ------------

    // Початок include guard
    fprintf(out_h, "#ifndef %s_H\n#define %s_H\n\n", fontname, fontname);

    // Необхідні інклуди, в т.ч. glyphs.h для оголошення структури RasterFont
    fprintf(out_h, "#include <stdint.h>\n");
    fprintf(out_h, "#include \"glyphmap.h\"\n");
    fprintf(out_h, "#include \"glyphs.h\"\n\n");  // Важливо: щоб визначався тип RasterFont

    // Оголошення констант ширини, висоти і розміру гліфа в байтах
    fprintf(out_h, "extern const int %s_glyph_width;\n", fontname);
    fprintf(out_h, "extern const int %s_glyph_height;\n", fontname);
    fprintf(out_h, "extern const int %s_glyph_bytes;\n\n", fontname);

    // Оголошення масиву відповідності Unicode → glyph data
    fprintf(out_h, "extern const GlyphPointerMap %s_glyph_ptr_map[];\n", fontname);
    fprintf(out_h, "extern const int %s_glyph_ptr_map_count;\n\n", fontname);

    // Оголошення екземпляру шрифту (структури RasterFont)
    fprintf(out_h, "extern const RasterFont %s_font;\n\n", fontname);

    // Кінець include guard
    fprintf(out_h, "#endif // %s_H\n", fontname);

    // ------------ Генерація C-файлу ------------

    // Підключення відповідного заголовка
    fprintf(out_c, "#include \"%s.h\"\n\n", fontname);

    // Визначення констант шрифту
    fprintf(out_c, "const int %s_glyph_width = %d;\n", fontname, font.width);
    fprintf(out_c, "const int %s_glyph_height = %d;\n", fontname, font.height);
    fprintf(out_c, "const int %s_glyph_bytes = %d;\n\n", fontname, font.charsize);

    int bytes_per_glyph = font.charsize;
    int glyphs_exported = 0;

    // Експорт гліфів ASCII від 32 до 126 включно
    for (uint32_t codepoint = 32; codepoint <= 126; ++codepoint) {
        int glyph_index = codepoint; // ASCII індекс гліфа зазвичай співпадає з кодом символу
        if (glyph_index < 0 || glyph_index >= font.charcount) continue;

        unsigned char* glyph_data = font.glyphBuffer + glyph_index * bytes_per_glyph;
        fprintf(out_c, "static const uint8_t %s_glyph_%04X[%d] = {", fontname, codepoint, bytes_per_glyph);

        for (int b = 0; b < bytes_per_glyph; b++) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyph_data[b]);
            if (b < bytes_per_glyph - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
        glyphs_exported++;
    }

    // Експорт гліфів кирилиці за таблицею cyr_map, пропускаючи дублі ASCII
    int cyr_count = sizeof(cyr_map) / sizeof(cyr_map[0]);
    for (int i = 0; i < cyr_count; i++) {
        uint32_t unicode = cyr_map[i].unicode;
        int glyph_index = cyr_map[i].glyph_index;

        if (glyph_index < 0 || glyph_index >= font.charcount) continue;
        if (unicode >= 32 && unicode <= 126) continue; // пропускаємо ASCII

        unsigned char* glyph_data = font.glyphBuffer + glyph_index * bytes_per_glyph;
        fprintf(out_c, "static const uint8_t %s_glyph_%04X[%d] = {", fontname, unicode, bytes_per_glyph);

        for (int b = 0; b < bytes_per_glyph; b++) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyph_data[b]);
            if (b < bytes_per_glyph - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
        glyphs_exported++;
    }

    // Створення масиву GlyphPointerMap — відповідність Unicode → гліф
    fprintf(out_c, "const GlyphPointerMap %s_glyph_ptr_map[] = {\n", fontname);

    // Додаємо записи для ASCII
    for (uint32_t codepoint = 32; codepoint <= 126; ++codepoint) {
        int glyph_index = codepoint;
        if (glyph_index < 0 || glyph_index >= font.charcount) continue;
        fprintf(out_c, "    {0x%04X, %s_glyph_%04X},\n", codepoint, fontname, codepoint);
    }

    // Додаємо записи для кирилиці
    for (int i = 0; i < cyr_count; i++) {
        uint32_t unicode = cyr_map[i].unicode;
        int glyph_index = cyr_map[i].glyph_index;
        if (glyph_index < 0 || glyph_index >= font.charcount) continue;
        if (unicode >= 32 && unicode <= 126) continue;
        fprintf(out_c, "    {0x%04X, %s_glyph_%04X},\n", unicode, fontname, unicode);
    }
    fprintf(out_c, "};\n\n");

    // Оголошення загальної кількості гліфів у масиві
    fprintf(out_c, "const int %s_glyph_ptr_map_count = sizeof(%s_glyph_ptr_map) / sizeof(GlyphPointerMap);\n\n",
            fontname, fontname);

    // Ініціалізація екземпляра структури RasterFont для зручного використання
    fprintf(out_c, "const RasterFont %s_font = {\n", fontname);
    fprintf(out_c, "    .name = \"%s\",\n", fontname);
    fprintf(out_c, "    .glyph_width = %s_glyph_width,\n", fontname);
    fprintf(out_c, "    .glyph_height = %s_glyph_height,\n", fontname);
    fprintf(out_c, "    .glyph_bytes = %s_glyph_bytes,\n", fontname);
    fprintf(out_c, "    .glyph_map = %s_glyph_ptr_map,\n", fontname);
    fprintf(out_c, "    .glyph_count = %s_glyph_ptr_map_count\n", fontname);
    fprintf(out_c, "};\n\n");

    fclose(out_c);
    fclose(out_h);

    printf("Успішно згенеровано %d гліфів шрифту \"%s\" в файли:\n  %s\n  %s\n",
           glyphs_exported, fontname, filename_c, filename_h);
}

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
