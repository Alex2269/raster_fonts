// svg_generator/svg_to_raster.c
/*
svg_to_raster.c - конвертер SVG іконок у формат RasterFont
Підтримує:
- Одиночні файли: ./app application.elf icon.svg
- Пакетна обробка: ./app application.elf /path/to/svg/dir
- Сортування в алфавітному порядку (0-9, a-z, A-Z)
- Правильне бітове зміщення даних після обрізки порожніх полів
*/

#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo.h>
#include <rsvg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <glob.h>

// Структури для RasterFont
typedef struct {
    uint32_t unicode;
    const uint8_t* glyph;
} GlyphPointerMap;

typedef struct {
    uint32_t unicode;
    unsigned char* data;
    int width;
    int height;
    int vertical_offset;
    int horizontal_offset;
} GlyphData;

// Глобальні налаштування
static int g_glyph_width = 16;
static int g_glyph_height = 16;
static int g_verbose = 0;

/*
 * Конвертація імені файлу в Unicode код
 */
uint32_t filename_to_unicode(const char* filename, int index) {
    char basename[256];
    strncpy(basename, filename, sizeof(basename) - 1);
    basename[sizeof(basename) - 1] = '\0';
    
    char* dot = strrchr(basename, '.');
    if (dot) *dot = '\0';
    
    if (strncmp(basename, "U+", 2) == 0 || strncmp(basename, "u+", 2) == 0) {
        uint32_t code;
        if (sscanf(basename + 2, "%x", &code) == 1) return code;
    }
    
    if (strlen(basename) == 4) {
        uint32_t code;
        if (sscanf(basename, "%x", &code) == 1) return code;
    }
    
    if (strlen(basename) == 1) {
        return (uint8_t)basename[0];
    }
    
    return 32 + index;
}

/*
 * Завантаження SVG та рендеринг у бітову карту з ПРАВИЛЬНИМ ЗМІЩЕННЯМ БІТІВ
 */
int render_svg_to_bitmap(const char* svg_path,
                         unsigned char* bitmap,
                         int bitmap_width,
                         int bitmap_height,
                         int* out_actual_width,
                         int* out_actual_height) {

    GError* error = NULL;
    RsvgHandle* handle = rsvg_handle_new_from_file(svg_path, &error);
    if (!handle) {
        fprintf(stderr, "Error loading SVG %s: %s\n", svg_path, error ? error->message : "Unknown");
        if (error) g_error_free(error);
        return -1;
    }

    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(handle, &dims);

    printf("  SVG original size: %dx%d\n", dims.width, dims.height);

    // Створюємо Cairo surface у цільовому розмірі
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, bitmap_width, bitmap_height);
    if (!surface) {
        rsvg_handle_free(handle);
        return -1;
    }

    cairo_t* cr = cairo_create(surface);

    // Очищаємо поверхню
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    // ВИПРАВЛЕННЯ: Правильна трансформація для SVG з негативними Y координатами
    // 1. Масштабування з ЗБЕРЕЖЕННЯМ ПРОПОРЦІЙ
    double scale_x = (double)bitmap_width / dims.width;
    double scale_y = (double)bitmap_height / dims.height;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;

    // 2. Центрування
    double offset_x = (bitmap_width - dims.width * scale) / 2.0;
    double offset_y = (bitmap_height - dims.height * scale) / 2.0;

    cairo_translate(cr, offset_x, offset_y);
    cairo_scale(cr, scale, scale);

    // 3. Віддзеркалення Y осі (SVG: Y=0 зверху, Cairo: Y=0 знизу)
    // Переміщуємо початок координат у нижній кут і віддзеркалюємо
    // cairo_translate(cr, 0, dims.height);
    // cairo_scale(cr, 1, -1);

    // Рендеримо SVG
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    rsvg_handle_render_cairo(handle, cr);

    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        rsvg_handle_free(handle);
        return -1;
    }

    cairo_destroy(cr);

    // Копіюємо дані з Cairo surface у bitmap
    int bytes_per_row = (bitmap_width + 7) / 8;
    unsigned char* src_data = cairo_image_surface_get_data(surface);
    int src_stride = cairo_image_surface_get_stride(surface);

    memset(bitmap, 0, bytes_per_row * bitmap_height);

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            unsigned char alpha = src_data[y * src_stride + x];

            if (alpha > 128) {
                int byte_idx = y * bytes_per_row + x / 8;
                int bit_idx = 7 - (x % 8);
                bitmap[byte_idx] |= (1 << bit_idx);
            }
        }
    }

    // Знаходимо bounding box
    int min_x = bitmap_width, max_x = -1;
    int min_y = bitmap_height, max_y = -1;
    int has_content = 0;

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            int byte_idx = y * bytes_per_row + x / 8;
            int bit_idx = 7 - (x % 8);
            if (bitmap[byte_idx] & (1 << bit_idx)) {
                has_content = 1;
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
        }
    }

    if (!has_content) {
        *out_actual_width = 0;
        *out_actual_height = 0;
        cairo_surface_destroy(surface);
        rsvg_handle_free(handle);
        return 0;
    }

    int actual_w = max_x - min_x + 1;
    int actual_h = max_y - min_y + 1;

    printf("  Bounding box: min_x=%d, min_y=%d, max_x=%d, max_y=%d\n", min_x, min_y, max_x, max_y);
    printf("  Actual size: %dx%d\n", actual_w, actual_h);

    // Зміщуємо біти вліво-вгору
    unsigned char* temp_bitmap = calloc(bytes_per_row * bitmap_height, 1);
    if (!temp_bitmap) {
        cairo_surface_destroy(surface);
        rsvg_handle_free(handle);
        return -1;
    }

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int src_byte = y * bytes_per_row + x / 8;
            int src_bit = 7 - (x % 8);
            if (bitmap[src_byte] & (1 << src_bit)) {
                int dest_x = x - min_x;
                int dest_y = y - min_y;
                int dest_byte = dest_y * bytes_per_row + dest_x / 8;
                int dest_bit = 7 - (dest_x % 8);
                temp_bitmap[dest_byte] |= (1 << dest_bit);
            }
        }
    }

    memcpy(bitmap, temp_bitmap, bytes_per_row * bitmap_height);
    free(temp_bitmap);

    *out_actual_width = actual_w;
    *out_actual_height = actual_h;

    cairo_surface_destroy(surface);
    rsvg_handle_free(handle);
    return 0;
}

/*
 * Порівняння файлів для сортування (0-9, a-z, A-Z)
 */
int compare_svg_files(const void* a, const void* b) {
    const char* name_a = *(const char**)a;
    const char* name_b = *(const char**)b;
    
    char base_a[256], base_b[256];
    const char* slash_a = strrchr(name_a, '/');
    const char* slash_b = strrchr(name_b, '/');
    strncpy(base_a, slash_a ? slash_a + 1 : name_a, 255);
    strncpy(base_b, slash_b ? slash_b + 1 : name_b, 255);
    base_a[255] = '\0';
    base_b[255] = '\0';
    
    char* dot_a = strrchr(base_a, '.');
    char* dot_b = strrchr(base_b, '.');
    if (dot_a) *dot_a = '\0';
    if (dot_b) *dot_b = '\0';
    
    int cat_a = 3, cat_b = 3;
    if (strlen(base_a) == 1) {
        if (base_a[0] >= '0' && base_a[0] <= '9') cat_a = 0;
        else if (base_a[0] >= 'a' && base_a[0] <= 'z') cat_a = 1;
        else if (base_a[0] >= 'A' && base_a[0] <= 'Z') cat_a = 2;
    }
    if (strlen(base_b) == 1) {
        if (base_b[0] >= '0' && base_b[0] <= '9') cat_b = 0;
        else if (base_b[0] >= 'a' && base_b[0] <= 'z') cat_b = 1;
        else if (base_b[0] >= 'A' && base_b[0] <= 'Z') cat_b = 2;
    }
    
    if (cat_a != cat_b) return cat_a - cat_b;
    return strcmp(base_a, base_b);
}

/*
 * Отримання списку SVG файлів
 */
int get_svg_files_from_dir(const char* dir_path, char*** files_out) {
    DIR* dir = opendir(dir_path);
    if (!dir) return 0;
    
    char** files = malloc(1024 * sizeof(char*));
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            const char* name = entry->d_name;
            const char* ext = strrchr(name, '.');
            if (ext && strcasecmp(ext, ".svg") == 0) {
                char* full_path = malloc(strlen(dir_path) + strlen(name) + 2);
                sprintf(full_path, "%s/%s", dir_path, name);
                files[count++] = full_path;
            }
        }
    }
    closedir(dir);
    
    qsort(files, count, sizeof(char*), compare_svg_files);
    *files_out = files;
    return count;
}

/*
 * Експорт у C файли
 */
void ExportGlyphsToC(GlyphData* glyphs, int glyph_count, 
                     int glyph_width, int glyph_height,
                     int glyph_bytes, const char* font_name) {
    
    char filename_c[300], filename_h[300];
    snprintf(filename_c, sizeof(filename_c), "%s.c", font_name);
    snprintf(filename_h, sizeof(filename_h), "%s.h", font_name);
    
    FILE* out_c = fopen(filename_c, "w");
    FILE* out_h = fopen(filename_h, "w");
    if (!out_c || !out_h) {
        fprintf(stderr, "Failed to open output files\n");
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
        font_name, font_name,
        font_name, font_name, font_name,
        font_name, font_name,
        font_name, font_name, font_name, font_name,
        font_name, font_name);
    
    fprintf(out_c, "#include \"%s.h\"\n\n", font_name);
    fprintf(out_c, "const int %s_glyph_width = %d;\n", font_name, glyph_width);
    fprintf(out_c, "const int %s_glyph_height = %d;\n", font_name, glyph_height);
    fprintf(out_c, "const int %s_glyph_bytes = %d;\n\n", font_name, glyph_bytes);
    
    for (int i = 0; i < glyph_count; i++) {
        fprintf(out_c, "static const uint8_t %s_glyph_%04X[%d] = {", 
                font_name, glyphs[i].unicode, glyph_bytes);
        for (int b = 0; b < glyph_bytes; b++) {
            if (b % 12 == 0) fprintf(out_c, "\n    ");
            fprintf(out_c, "0x%02X", glyphs[i].data[b]);
            if (b < glyph_bytes - 1) fprintf(out_c, ", ");
        }
        fprintf(out_c, "\n};\n\n");
    }
    
    fprintf(out_c, "const GlyphPointerMap %s_glyph_ptr_map[] = {\n", font_name);
    for (int i = 0; i < glyph_count; i++) {
        fprintf(out_c, "    {0x%04X, %s_glyph_%04X},\n", 
                glyphs[i].unicode, font_name, glyphs[i].unicode);
    }
    fprintf(out_c, "};\n\n");
    
    fprintf(out_c, "const int %s_glyph_widths[%d] = {\n", font_name, glyph_count);
    for (int i = 0; i < glyph_count; i++)
        fprintf(out_c, "    %d%s\n", glyphs[i].width, (i < glyph_count - 1) ? "," : "");
    fprintf(out_c, "};\n\n");
    
    fprintf(out_c, "const int %s_glyph_heights[%d] = {\n", font_name, glyph_count);
    for (int i = 0; i < glyph_count; i++)
        fprintf(out_c, "    %d%s\n", glyphs[i].height, (i < glyph_count - 1) ? "," : "");
    fprintf(out_c, "};\n\n");
    
    fprintf(out_c, "const int %s_glyph_vertical_offsets[%d] = {\n", font_name, glyph_count);
    for (int i = 0; i < glyph_count; i++)
        fprintf(out_c, "    %d%s\n", glyphs[i].vertical_offset, (i < glyph_count - 1) ? "," : "");
    fprintf(out_c, "};\n\n");
    
    fprintf(out_c, "const int %s_glyph_horizontal_offsets[%d] = {\n", font_name, glyph_count);
    for (int i = 0; i < glyph_count; i++)
        fprintf(out_c, "    %d%s\n", glyphs[i].horizontal_offset, (i < glyph_count - 1) ? "," : "");
    fprintf(out_c, "};\n\n");
    
    fprintf(out_c, "const int %s_glyph_ptr_map_count = %d;\n\n", font_name, glyph_count);
    
    fprintf(out_c,
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
        font_name, font_name,
        font_name, font_name, font_name,
        font_name, font_name,
        font_name, font_name, font_name, font_name);
    
    fclose(out_c);
    fclose(out_h);
    printf("Generated %d glyphs for font %s\n", glyph_count, font_name);
}

/*
 * Обробка одного файлу
 */
int process_svg_file(const char* svg_path, int index, GlyphData* glyphs) {
    char basename[256];
    const char* name = strrchr(svg_path, '/');
    name = name ? name + 1 : svg_path;
    strncpy(basename, name, sizeof(basename) - 1);
    
    if (g_verbose) printf("Processing: %s (index: %d)\n", basename, index);
    
    uint32_t unicode = filename_to_unicode(basename, index);
    
    int bytes_per_row = (g_glyph_width + 7) / 8;
    int bitmap_size = bytes_per_row * g_glyph_height;
    unsigned char* bitmap = calloc(bitmap_size, 1);
    if (!bitmap) return -1;
    
    int actual_width, actual_height;
    if (render_svg_to_bitmap(svg_path, bitmap, g_glyph_width, g_glyph_height,
                            &actual_width, &actual_height) != 0) {
        free(bitmap);
        return -1;
    }
    
    glyphs[index].unicode = unicode;
    glyphs[index].data = bitmap;
    glyphs[index].width = actual_width;
    glyphs[index].height = actual_height;
    glyphs[index].vertical_offset = 0;
    glyphs[index].horizontal_offset = 0;
    
    if (g_verbose) printf("  Unicode: U+%04X, Size: %dx%d\n", unicode, actual_width, actual_height);
    return 0;
}

/*
 * Головна функція
 */
int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s [options] <output_name> <svg_file_or_dir>\n", argv[0]);
        printf("Options:\n");
        printf("  -w <width>   Glyph width (default: 16)\n");
        printf("  -h <height>  Glyph height (default: 16)\n");
        printf("  -v           Verbose output\n");
        return 1;
    }
    
    int arg_idx = 1;
    while (arg_idx < argc) {
        if (strcmp(argv[arg_idx], "-w") == 0 && arg_idx + 1 < argc) g_glyph_width = atoi(argv[++arg_idx]);
        else if (strcmp(argv[arg_idx], "-h") == 0 && arg_idx + 1 < argc) g_glyph_height = atoi(argv[++arg_idx]);
        else if (strcmp(argv[arg_idx], "-v") == 0) g_verbose = 1;
        else if (argv[arg_idx][0] != '-') break;
        arg_idx++;
    }
    
    if (arg_idx + 2 > argc) {
        fprintf(stderr, "Missing output name or input file/directory\n");
        return 1;
    }
    
    const char* output_name = argv[arg_idx];
    const char* input_path = argv[arg_idx + 1];
    
    printf("SVG to RasterFont Converter\n");
    printf("Glyph size: %dx%d\n", g_glyph_width, g_glyph_height);
    printf("Input: %s\n\n", input_path);
    
    struct stat path_stat;
    stat(input_path, &path_stat);
    
    char** svg_files = NULL;
    int file_count = 0;
    
    if (S_ISDIR(path_stat.st_mode)) {
        file_count = get_svg_files_from_dir(input_path, &svg_files);
        if (file_count == 0) {
            fprintf(stderr, "No SVG files found in %s\n", input_path);
            return 1;
        }
        printf("Found %d SVG files\n", file_count);
    } else {
        svg_files = malloc(sizeof(char*));
        svg_files[0] = strdup(input_path);
        file_count = 1;
    }
    
    GlyphData* glyphs = calloc(file_count, sizeof(GlyphData));
    if (!glyphs) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    
    int success_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (process_svg_file(svg_files[i], i, glyphs) == 0) success_count++;
    }
    
    printf("\nSuccessfully processed %d/%d files\n", success_count, file_count);
    
    int bytes_per_glyph = ((g_glyph_width + 7) / 8) * g_glyph_height;
    ExportGlyphsToC(glyphs, success_count, g_glyph_width, g_glyph_height, bytes_per_glyph, output_name);
    
    for (int i = 0; i < file_count; i++) {
        if (glyphs[i].data) free(glyphs[i].data);
        if (svg_files[i]) free(svg_files[i]);
    }
    free(glyphs);
    free(svg_files);
    
    printf("Done!\n");
    return 0;
}
