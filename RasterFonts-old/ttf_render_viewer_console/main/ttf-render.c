// ttf-render.c

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Функція для конвертації UTF-8 послідовності у Unicode кодпоінт
const char* utf8_to_codepoint(const char* str, uint32_t* codepoint) {
    unsigned char c = (unsigned char)*str;
    if (c < 0x80) {
        // 1-байтний символ ASCII (0xxxxxxx)
        *codepoint = c;
        return str + 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-байтний UTF-8 символ (110xxxxx 10xxxxxx)
        *codepoint = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        return str + 2;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-байтний UTF-8 символ (1110xxxx 10xxxxxx 10xxxxxx)
        *codepoint = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return str + 3;
    } else {
        // 4-байтні та інші символи не підтримуються – ставимо 0
        *codepoint = 0;
        return str + 1; // щоб не зациклитись
    }
}

// Функція для «намалювання» (затінення / блітингу) гліфа у бітовому буфері bitmap
void blit_glyph(uint8_t* bitmap, int bitmap_width, int bitmap_height,
                FT_Bitmap* bmp, int pen_x, int pen_y, int bitmap_left, int bitmap_top)
{
    // ttf рендерить у 1-бітному форматі в байтових рядках
    int bytes_per_row = (bitmap_width + 7) / 8;
    // Проходимо по пікселях зображення гліфа
    for (int y = 0; y < bmp->rows; y++) {
        int dest_y = pen_y - bitmap_top + y; // вертикальна позиція в кінцевому бейтампі
        if (dest_y < 0 || dest_y >= bitmap_height) continue; // вихід за межі по висоті
        for (int x = 0; x < bmp->width; x++) {
            unsigned char pixel = bmp->buffer[y * bmp->pitch + x]; // інтенсивність пікселя гліфа
            if (pixel > 128) { // поріг включення (можна регулювати)
                int dest_x = pen_x + bitmap_left + x; // горизонтальна позиція у результаті
                if (dest_x < 0 || dest_x >= bitmap_width) continue; // межі по ширині
                int byte_idx = dest_y * bytes_per_row + (dest_x / 8); // байт у буфері
                int bit_idx = 7 - (dest_x % 8); // позиція біта у байті (big-endian)
                bitmap[byte_idx] |= (1 << bit_idx); // встановлення біта
            }
        }
    }
}

// Функція для підрахунку кількості рядків, які необхідно виділити для рендеру
int count_lines(const char* text, FT_Face face, int max_width_px) {
    int pen_x = 0;   // горизонтальна позиція пера (курсор по X)
    int lines = 1;   // рахунок рядків (починаємо з 1, бо хоч один рядок завжди буде)
    const char* p = text;
    uint32_t codepoint;

    // Ітеруємо текст, обробляємо символи
    while (*p) {
        if (*p == '\n') {
            // Якщо зустрічаємо символ нового рядка - скидаємо X і додаємо рядок
            pen_x = 0;
            lines++;
            p++;
            continue;
        }

        p = utf8_to_codepoint(p, &codepoint); // отримуємо кодпоінт UTF-8

        if (codepoint == 0) continue; // ігноруємо непідтримувані символи безпечним чином

        if (FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT)) continue; // Завантажуємо гліф без рендеру (функція поверт така при помилці)

        int advance = face->glyph->advance.x >> 6; // скільки далі рухаємось по X (пікселі)
        if (pen_x + advance > max_width_px) {
            // Якщо виходимо за межі максимальної ширини - переходимо на новий рядок
            pen_x = 0;
            lines++;
        }
        pen_x += advance;
    }
    return lines;
}

// Функція для виводу bitmap як тексту в консоль (# для чорних пікселів, пробіл для фонів)
void print_bitmap(const uint8_t* bitmap, int bitmap_width, int bitmap_height) {
    int bytes_per_row = (bitmap_width + 7) / 8;
    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            int byte_idx = y * bytes_per_row + (x / 8);
            int bit_idx = 7 - (x % 8);
            putchar((bitmap[byte_idx] & (1 << bit_idx)) ? '#' : ' ');
        }
        putchar('\n'); // Перехід на новий рядок після кожного рядка пікселів
    }
}

int main(int argc, char** argv) {
    if (argc < 6) {
        printf("Usage: %s <font.ttf> <glyph_width> <glyph_height> <max_width_px> <text>\n", argv[0]);
        return 1;
    }

    // Отримуємо параметри командного рядка
    const char* fontfile = argv[1];
    int glyph_width = atoi(argv[2]);
    int glyph_height = atoi(argv[3]);
    int max_width_px = atoi(argv[4]);
    const char* text = argv[5];

    // Ініціалізація FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Failed to initialize FreeType\n");
        return 1;
    }

    // Завантаження шрифту
    FT_Face face;
    if (FT_New_Face(ft, fontfile, 0, &face)) {
        fprintf(stderr, "Failed to load font %s\n", fontfile);
        FT_Done_FreeType(ft);
        return 1;
    }

    // Встановлення розміру гліфів (в пікселях)
    FT_Set_Pixel_Sizes(face, glyph_width, glyph_height);

    // 1. Підрахунок необхідної кількості рядків для тексту
    int lines = count_lines(text, face, max_width_px);

    int bytes_per_row = (max_width_px + 7) / 8; // кількість байтів в одному рядку бітового буфера
    int bitmap_height = lines * glyph_height;   // загальна висота bitmap — висота гліфа помножена на кількість рядків

    // Виділення пам'яті під bitmap (очищена нулями)
    uint8_t* bitmap = calloc(bitmap_height * bytes_per_row, 1);
    if (!bitmap) {
        fprintf(stderr, "Out of memory\n");
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return 1;
    }

    // 2. Другий прохід: рендер символів у bitmap
    int pen_x = 0;
    int pen_y = glyph_height; // починаємо з першого рядка, y — це baseline шрифту

    const char* p = text;
    uint32_t codepoint;

    while (*p) {
        if (*p == '\n') {
            pen_x = 0;           // перехід в початок нового рядка
            pen_y += glyph_height; // зміщення вниз
            p++;
            continue;
        }

        p = utf8_to_codepoint(p, &codepoint);
        if (codepoint == 0) continue;

        if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
            fprintf(stderr, "Failed to load char U+%04X\n", codepoint);
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        // Якщо текст виходить за межі max ширини — переносимо на новий рядок
        if (pen_x + (g->advance.x >> 6) > max_width_px) {
            pen_x = 0;
            pen_y += glyph_height;
        }

        // Відрисовуємо гліф в bitmap у правильній позиції
        blit_glyph(bitmap, max_width_px, bitmap_height,
                   &g->bitmap, pen_x, pen_y, g->bitmap_left, g->bitmap_top);

        // Зрушуємо pen_x на ширину гліфа для наступного символу
        pen_x += g->advance.x >> 6;
    }

    // Виводимо bitmap в консоль як текст
    print_bitmap(bitmap, max_width_px, bitmap_height);

    // Звільняємо ресурси
    free(bitmap);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}

