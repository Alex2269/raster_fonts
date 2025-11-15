/*
 * ttf_generator.c - генератор растрових гліфів TTF-шрифту за допомогою FreeType.
 * Програма завантажує шрифт, генерує бітмапи гліфів для заданого набору Unicode символів,
 * обрізає зайві порожні рядки зверху і знизу, формує щільний буфер гліфа,
 * експортує у C-масиви з метаданими для подальшого використання у рендері.
 */

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef struct {
    uint32_t unicode;        // Код Unicode символу
    const uint8_t *glyph;    // Вказівник на масив байтів гліфа
} GlyphPointerMap;

typedef struct {
    uint32_t unicode;        // Код Unicode символу
    unsigned char *data;     // Щільний бітовий буфер гліфа
    int width;               // Ширина гліфа в пікселях (активна область)
    int height;              // Висота гліфа в пікселях (активна область)
    int vertical_offset;     // Зсув по вертикалі від baseline для точного рендеру
} GlyphData;

/*
 * process_codepoint_compressed - генерує растровий гліф для вказаного Unicode коду.
 * Виконує завантаження та рендеринг гліфа через FreeType,
 * знаходить активні по вертикалі пікселі (обрізаючи порожні рядки зверху і знизу),
 * копіює активну область у щільний буфер без зайвих заповнень,
 * повертає реальні розміри гліфа і вертикальний зсув для коректного рендеру.
 *
 * Параметри:
 *  face - ініціалізований FT_Face (шрифт)
 *  codepoint - Unicode код символу
 *  glyph_width - задана базова ширина (пікселі)
 *  glyph_height - задана базова висота (пікселі)
 *  glyph_data_buffer - буфер для запису згенерованого гліфа (розмір max_charsize байт)
 *  max_charsize - максимально доступний розмір буфера glyph_data_buffer
 *  out_glyph_width - вихідна фактична ширина гліфа
 *  out_glyph_height - вихідна висота активної області гліфа
 *  out_vertical_offset - вихідний вертикальний зсув для рендера
 */
void process_codepoint_compressed(FT_Face face, uint32_t codepoint, int glyph_width, int glyph_height,
                                  unsigned char* glyph_data_buffer, int max_charsize,
                                  int *out_glyph_width, int *out_glyph_height, int *out_vertical_offset)
{
    // Завантаження і рендеринг гліфа (монохромний)
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
        // Якщо гліф не завантажився, очищаємо буфер і повертаємо базові розміри
        memset(glyph_data_buffer, 0, max_charsize);
        *out_glyph_width = glyph_width;
        *out_glyph_height = glyph_height;
        *out_vertical_offset = 0;
        return;
    }

    FT_Bitmap* bmp = &face->glyph->bitmap;

    // Очищаємо буфер на початок для запису нового гліфа
    memset(glyph_data_buffer, 0, max_charsize);

    // Пошук активної області гліфа по вертикалі - визначаємо верхню і нижню межі
    int top_row = -1, bottom_row = -1;
    for (int r = 0; r < bmp->rows; r++) {
        for (int c = 0; c < bmp->width; c++) {
            // Якщо піксель має інтенсивність більшу за 128 (порог для видимого пікселя)
            if (bmp->buffer[r * bmp->width + c] > 128) {
                if (top_row == -1) top_row = r;   // Задаємо верхню активну лінію
                bottom_row = r;                   // Оновлюємо нижню активну лінію
            }
        }
    }

    // Якщо не знайдено жодного активного пікселя, гліф вважаємо порожнім
    if (top_row == -1) {
        *out_glyph_width = glyph_width;
        *out_glyph_height = 0;
        *out_vertical_offset = 0;
        return;
    }

    // Висота активної області - кількість рядків з пікселями
    int active_height = bottom_row - top_row + 1;

    int offset_x = 0;
    // Обчислюємо кількість байтів на рядок у масиві гліфа
    int bytes_per_row = (glyph_width + 7) / 8 / 2;

    unsigned char temp_buffer[max_charsize];
    // Очищаємо тимчасовий буфер для формування щільного гліфа
    memset(temp_buffer, 0, max_charsize);

    // Цикл по активним рядкам обробляє кожен видимий рядок гліфа
    for (int row = top_row; row <= bottom_row; row++) {
        // Цикл по колонках - по кожному пікселю в рядку
        for (int col = 0; col < bmp->width; col++) {
            // Перевірка, чи піксель достатньо яскравий щоб бути видимим
            if (bmp->buffer[row * bmp->width + col] > 128) {
                int target_row = row - top_row;               // Відносний індекс рядка у сформованому гліфі
                int target_col = col + offset_x;              // Відносний індекс колонки (пікселя)
                // Знаходження байта, в якому знаходиться цей піксель
                int byte_index = target_row * bytes_per_row + target_col / 8;
                // Обчислення позиції біта у байті (пікселі в кожному байті читаються справа наліво)
                int bit_index = 7 - (target_col % 8);
                // Встановлення біта у тимчасовому буфері, що відповідає пікселю
                temp_buffer[byte_index] |= (1 << bit_index);
            }
        }
    }

    // Копіюємо сформований щільний буфер у вихідний буфер для гліфа
    memcpy(glyph_data_buffer, temp_buffer, max_charsize);

    // Повертаємо фактичну ширину гліфа (або базову, якщо bmp->width 0)
    *out_glyph_width = bmp->width > 0 ? bmp->width : glyph_width;
    // Повертаємо висоту обробленої активної області
    *out_glyph_height = active_height;

    // Обчислюємо вертикальний зсув гліфа відносно базової лінії для точного рендеру
    int baseline_offset = glyph_height - face->glyph->bitmap_top;

    int shift_up = roundf((float)glyph_height/4.0f); // підняти гліф на 1/4 гліфа вгору, можна регулювати

    *out_vertical_offset = baseline_offset - shift_up;

    // *out_vertical_offset = baseline_offset - top_row;
}

/*
 * GetFontName - отримання назви шрифту з шляху до файлу,
 * виключаючи шлях та розширення.
 *
 * filepath - повний шлях/ім'я файлу шрифту
 * fontname - буфер для збереження імені шрифту
 * max_len - розмір буфера fontname
 */
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

/*
 * ExportGlyphsToC - основна функція експорту гліфів з FT_Face у C файли.
 * Завантажує гліфи для заданих діапазонів Unicode,
 * зберігає їх у щільних масивах,
 * експортує метадані і C масиви у .c і .h файли.
 *
 * face - ідентифікатор FreeType шрифту
 * glyph_width - базова ширина гліфа (пікселів)
 * glyph_height - базова висота гліфа (пікселів)
 * input_filename - шлях до файлу шрифту, для іменування вихідних файлів
 */
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

    int max_charsize = ((glyph_width + 7) / 8) * glyph_height / 2;

    // Запис заголовного файлу
    fprintf(out_h,
            "#ifndef %s_H\n#define %s_H\n\n"
            "#include <stdint.h>\n"
            "#include \"glyphmap.h\"\n"
            "#include \"glyphs.h\"\n\n"
            "extern const int %s_glyph_height;\n"
            "extern const int %s_glyph_bytes;\n\n"
            "extern const GlyphPointerMap %s_glyph_ptr_map[];\n"
            "extern const int %s_glyph_ptr_map_count;\n\n"
            "extern const int %s_glyph_widths[];\n"
            "extern const int %s_glyph_heights[];\n"
            "extern const int %s_glyph_vertical_offsets[];\n\n"
            "extern const RasterFont %s_font;\n\n"
            "#endif // %s_H\n",
            fontname, fontname,
            fontname, fontname,
            fontname, fontname,
            fontname, fontname, fontname,
            fontname, fontname);

    // Запис c-файлу
    fprintf(out_c, "#include \"%s.h\"\n\n", fontname);
    fprintf(out_c, "const int %s_glyph_height = %d;\n", fontname, glyph_height);
    fprintf(out_c, "const int %s_glyph_bytes = %d;\n\n", fontname, max_charsize);

    GlyphData* glyphs = malloc(512 * sizeof(GlyphData));
    if (!glyphs) { fprintf(stderr, "Out of memory\n"); exit(1); }
    unsigned char* glyph_buffer = malloc(max_charsize);
    if (!glyph_buffer) { free(glyphs); fprintf(stderr, "Out of memory\n"); exit(1); }

    int glyph_count = 0;

    for (uint32_t cp = 32; cp <= 126; ++cp) {
        int w, h, vo;
        process_codepoint_compressed(face, cp, glyph_width, glyph_height, glyph_buffer, max_charsize, &w, &h, &vo);
        glyphs[glyph_count].unicode = cp;
        glyphs[glyph_count].data = malloc(max_charsize);
        if (!glyphs[glyph_count].data) { fprintf(stderr, "Out of memory\n"); exit(1); }
        memcpy(glyphs[glyph_count].data, glyph_buffer, max_charsize);
        glyphs[glyph_count].width = w;
        glyphs[glyph_count].height = h;
        glyphs[glyph_count].vertical_offset = vo;
        glyph_count++;
    }

    for (uint32_t cp = 0x0400; cp <= 0x04FF; ++cp) {
        int w, h, vo;
        process_codepoint_compressed(face, cp, glyph_width, glyph_height, glyph_buffer, max_charsize, &w, &h, &vo);
        glyphs[glyph_count].unicode = cp;
        glyphs[glyph_count].data = malloc(max_charsize);
        if (!glyphs[glyph_count].data) { fprintf(stderr, "Out of memory\n"); exit(1); }
        memcpy(glyphs[glyph_count].data, glyph_buffer, max_charsize);
        glyphs[glyph_count].width = w;
        glyphs[glyph_count].height = h;
        glyphs[glyph_count].vertical_offset = vo;
        glyph_count++;
    }

    free(glyph_buffer);

    // Запис масивів гліфів як статичних C-масивів
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "static const uint8_t %s_glyph_%04X[%d] = {", fontname, glyphs[i].unicode, max_charsize);
        for (int b = 0; b < max_charsize; ++b) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyphs[i].data[b]);
            if (b < max_charsize - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
    }

    // Map Unicode кодів на гліфи
    fprintf(out_c, "const GlyphPointerMap %s_glyph_ptr_map[] = {\n", fontname);
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "    {0x%04X, %s_glyph_%04X},\n", glyphs[i].unicode, fontname, glyphs[i].unicode);
    }
    fprintf(out_c, "};\n\n");

    // Метадані по ширинах, висотах, зсувам
    fprintf(out_c, "const int %s_glyph_widths[%d] = {\n", fontname, glyph_count);
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "    %d%s\n", glyphs[i].width, (i < glyph_count - 1) ? "," : "");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_heights[%d] = {\n", fontname, glyph_count);
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "    %d%s\n", glyphs[i].height, (i < glyph_count - 1) ? "," : "");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_vertical_offsets[%d] = {\n", fontname, glyph_count);
    for (int i = 0; i < glyph_count; ++i) {
        fprintf(out_c, "    %d%s\n", glyphs[i].vertical_offset, (i < glyph_count - 1) ? "," : "");
    }
    fprintf(out_c, "};\n\n");

    fprintf(out_c, "const int %s_glyph_ptr_map_count = %d;\n\n", fontname, glyph_count);

    // Ініціалізація структури RasterFont
    fprintf(out_c,
            "const RasterFont %s_font = {\n"
            "    .name = \"%s\",\n"
            "    .glyph_height = %s_glyph_height,\n"
            "    .glyph_bytes = %s_glyph_bytes,\n"
            "    .glyph_map = %s_glyph_ptr_map,\n"
            "    .glyph_count = %s_glyph_ptr_map_count,\n"
            "    .glyph_widths = %s_glyph_widths,\n"
            "    .glyph_heights = %s_glyph_heights,\n"
            "    .glyph_vertical_offsets = %s_glyph_vertical_offsets\n"
            "};\n",
            fontname, fontname,
            fontname, fontname,
            fontname, fontname,
            fontname, fontname, fontname);

    for (int i = 0; i < glyph_count; ++i) {
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

