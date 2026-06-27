// psf_console_viewer.c

/*
Цей код містить детальні коментарі українською мовою,
які пояснюють кожний ключовий крок: від читання шрифту,
через декодування UTF-8, пошук індекса гліфа, до виводу
символів зі зсувом для вертикального вирівнювання малих
літер кирилиці і латиниці по нижньому краю гліфа.*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "UnicodeGlyphMap.h" // підключення таблиці відповідності Unicode->гліфів для кирилиці

// Структура заголовка PSF2 шрифту
typedef struct {
    uint8_t magic[4];        // "магічні" байти формату PSF2
    uint32_t version;        // версія формату
    uint32_t headersize;     // розмір заголовка шрифту
    uint32_t flags;          // прапори формату
    uint32_t length;         // кількість гліфів у шрифті
    uint32_t glyph_bytes;    // розмір одного гліфа в байтах
    uint32_t glyph_height;   // висота гліфа в пікселях
    uint32_t glyph_width;    // ширина гліфа в пікселях
} PSF2_Header;

// Структура шрифту з інформацією та масивом гліфів
typedef struct {
    PSF2_Header header;
    unsigned char* glyphs;   // вказівник на масив гліфів
} PSF2_Font;

// Функція читання 4-х байтів у форматі little-endian з файлу
static uint32_t ReadLE32(FILE* f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

// Завантаження PSF2 шрифту з файлу
int LoadPSF2Font(const char* filename, PSF2_Font* font) {
    FILE* f = fopen(filename, "rb");
    if (!f) { perror("Open PSF2 font"); return 0; }

    fread(font->header.magic, 1, 4, f);
    // Перевірка магічних байтів PSF2
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

        // Виділення пам'яті під всі гліфи
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

// Декодування одного Unicode коду з UTF-8 рядка
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

// Функція для пошуку індексу гліфа у шрифті по Unicode коду символу
static int UnicodeToGlyphIndex(uint32_t codepoint) {
    // Якщо символ у діапазоні ASCII — повертаємо прямий індекс
    if (codepoint >= 32 && codepoint <= 126) {
        return (int)codepoint;
    }
    // Для кирилиці шукаємо індекс гліфа у таблиці відповідностей cyr_map
    for (int i = 0; i < sizeof(cyr_map)/sizeof(cyr_map[0]); i++) {
        if (cyr_map[i].unicode == codepoint) {
            return cyr_map[i].glyph_index;
        }
    }
    // Якщо символ не знайдено — замінюємо на пробіл (індекс 32)
    return 32;
}

// Функція для обчислення вертикального зсуву (offsetY) для вирівнювання символів нижнього регістру по нижньому краю гліфа
int CalculateLowercaseOffset(unsigned char* glyph, int glyphHeight, int bytesPerRow) {
    // Шукаємо найближчий до низу активний рядок з ненульовими пікселями
    for (int row = glyphHeight - 1; row >= 0; row--) {
        unsigned char* rowData = glyph + row * bytesPerRow;
        for (int byte = 0; byte < bytesPerRow; byte++) {
            if (rowData[byte] != 0) {
                // Визначаємо зсув: скільки рядків підняти символ вверх
                return glyphHeight - 1 - row;
            }
        }
    }
    // Якщо гліф порожній, зсув 0
    return 0;
}

// Активувати вертикальне підняття для малих літер 1, якщо 0 ні
#define ENABLE_RAISE 0

// Функція виводу рядка тексту з урахуванням Unicode та вертикального вирівнювання
void PrintTextLine(PSF2_Font* font, const char* text) {
    int bytes_per_row = (font->header.glyph_width + 7) / 8; // Кількість байт на рядок гліфа
    uint32_t codepoints[256];  // Масив Unicode кодів
    int len = 0;               // Кількість символів у тексті
    const char* p = text;

    // Декодуємо UTF-8 текст в масив Unicode кодів
    while (*p && len < 256) {
        uint32_t cp;
        int bytes = utf8_decode(p, &cp);
        codepoints[len++] = cp;
        p += bytes;
    }

    int maxGlyphHeight = font->header.glyph_height;

    // По рядках з верху до низу
    for (int row = 0; row < maxGlyphHeight; row++) {
        // Для кожного символу в рядку
        for (int i = 0; i < len; i++) {
            int glyph_idx = UnicodeToGlyphIndex(codepoints[i]);
            if (glyph_idx < 0) glyph_idx = 32;    // Якщо символ не знайдено, заміщуємо пробілом

            unsigned char* glyph = font->glyphs + glyph_idx * font->header.glyph_bytes;
            int glyphHeight = font->header.glyph_height;

            int offsetY = 0;
            #if ENABLE_RAISE
            // Визначаємо, чи символ є малою літерою латиниці або кирилиці
            int is_lowercase_latin = (codepoints[i] >= 'a' && codepoints[i] <= 'z');
            int is_lowercase_cyr = (codepoints[i] >= 0x0430 && codepoints[i] <= 0x044F) ||   // діапазон а-я
            codepoints[i] == 0x0451 ||  // ё
            codepoints[i] == 0x0456 ||  // і
            codepoints[i] == 0x0457;    // ї
            // Якщо це мала літера, обчислюємо offsetY для підняття символа по нижньому краю гліфа
            if (is_lowercase_latin || is_lowercase_cyr) {
                offsetY = CalculateLowercaseOffset(glyph, glyphHeight, bytes_per_row);
            }
            #endif

            int drawRow = row - offsetY;
            // Якщо рядок виходить за межі гліфа, виводимо пробіли
            if (drawRow < 0 || drawRow >= glyphHeight) {
                for (int bit = 0; bit < font->header.glyph_width; bit++) putchar(' ');
            } else {
                unsigned char* rowData = glyph + drawRow * bytes_per_row;
                // Вивід кожного пікселя рядка: # якщо піксель встановлений, інакше пробіл
                for (int bit = 0; bit < font->header.glyph_width; bit++) {
                    int byte_idx = bit / 8;
                    int bit_idx = 7 - (bit % 8);
                    if ((rowData[byte_idx] & (1 << bit_idx)) != 0) putchar('#');
                    else putchar(' ');
                }
            }
            putchar(' '); // Відступ між символами
        }
        putchar('\n');
    }
}

// Головна функція
int main(int argc, char* argv[]) {
    // Вивід підказки при недостатній кількості аргументів
    if (argc < 3) {
        printf("Usage: %s font.psf \"text\"\n", argv[0]);
        return 1;
    }

    PSF2_Font font;
    // Завантаження шрифту
    if (!LoadPSF2Font(argv[1], &font)) {
        printf("Failed to load font\n");
        return 1;
    }

    // Вивід рядка тексту
    PrintTextLine(&font, argv[2]);

    // Звільнення пам'яті
    free(font.glyphs);
    return 0;
}

