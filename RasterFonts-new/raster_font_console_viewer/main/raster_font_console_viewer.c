/*
raster_font_console_viewer.c
Консольний переглядач растрових шрифтів (RasterFont)
Підтримує: Terminus12x6, FreePixel, Pixel
Режими: --all, --char <hex>, --text "...", --info
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "glyphmap.h"
#include "glyphs.h"
#include "all_font.h"

/* ============================================================
   ANSI Escape Sequences (кольори для терміналу)
   ============================================================ */
#define ANSI_RESET     "\x1b[0m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_DIM       "\x1b[2m"

#define ANSI_RED       "\x1b[31m"
#define ANSI_GREEN     "\x1b[32m"
#define ANSI_YELLOW    "\x1b[33m"
#define ANSI_BLUE      "\x1b[34m"
#define ANSI_MAGENTA   "\x1b[35m"
#define ANSI_CYAN      "\x1b[36m"
#define ANSI_WHITE     "\x1b[37m"
#define ANSI_GRAY      "\x1b[90m"
#define ANSI_BRIGHT_GREEN "\x1b[92m"
#define ANSI_BRIGHT_CYAN  "\x1b[96m"

/* Символи для малювання */
#define PIXEL_ON       '█'    // Заповнений піксель
#define PIXEL_OFF      ' '    // Порожній піксель
#define PIXEL_DOT      '·'    // Альтернатива (для --dot режиму)

/* ============================================================
   Реєстр доступних шрифтів
   ============================================================ */
typedef struct {
    const char* name;
    const RasterFont* font;
} FontEntry;

static const FontEntry g_fonts[] = {
    { "Terminus12x6",  &Terminus12x6_font  },
    { "FreePixel",     &FreePixel_font     },
    { "Pixel",         &Pixel_font         },
    { NULL, NULL }
};

/* ============================================================
   Пошук гліфа в шрифті
   ============================================================ */
static const GlyphPointerMap* find_glyph(const RasterFont* font, uint32_t unicode) {
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyph_map[i].unicode == unicode)
            return &font->glyph_map[i];
    }
    return NULL;
}

/* ============================================================
   Малювання одного гліфа в консоль
   ============================================================ */
static void draw_glyph(const RasterFont* font, uint32_t unicode,
                       int scale, bool color, bool show_frame, char dot_char)
{
    const GlyphPointerMap* gm = find_glyph(font, unicode);
    if (!gm) {
        if (color) printf(ANSI_RED ANSI_BOLD);
        printf("  [U+%04X not found]", unicode);
        if (color) printf(ANSI_RESET);
        printf("\n");
        return;
    }

    int idx = (int)(gm - font->glyph_map);
    int w    = font->glyph_widths  ? font->glyph_widths[idx]  : font->glyph_width;
    int h    = font->glyph_heights ? font->glyph_heights[idx] : font->glyph_height;
    int voff = font->glyph_vertical_offsets   ? font->glyph_vertical_offsets[idx]   : 0;
    int hoff = font->glyph_horizontal_offsets ? font->glyph_horizontal_offsets[idx] : 0;
    int bpr  = (w + 7) / 8;

    /* Заголовок */
    char utf8_buf[5] = {0};
    utf8_encode(unicode, utf8_buf);

    if (color) printf(ANSI_BRIGHT_CYAN ANSI_BOLD);
    printf("  U+%04X ", unicode);
    if (color) printf(ANSI_YELLOW);
    printf("'%s' ", utf8_buf);
    if (color) printf(ANSI_DIM);
    printf("(%dx%d", w, h);
    if (hoff != 0 || voff != 0) printf(", off=%d,%d", hoff, voff);
    printf(")");
    if (color) printf(ANSI_RESET);
    printf("\n");

    if (!show_frame) {
        /* Без рамки — просто пікселі */
        for (int row = 0; row < h; row++) {
            for (int s = 0; s < scale; s++) {
                for (int byte = 0; byte < bpr; byte++) {
                    uint8_t bv = gm->glyph[row * bpr + byte];
                    for (int bit = 0; bit < 8; bit++) {
                        int px = byte * 8 + bit;
                        if (px >= w) break;
                        bool on = (bv & (0x80 >> bit)) != 0;
                        char ch = on ? PIXEL_ON : dot_char;
                        if (color) {
                            printf(on ? ANSI_GREEN : ANSI_GRAY);
                        }
                        for (int sx = 0; sx < scale; sx++) putchar(ch);
                        if (color) printf(ANSI_RESET);
                    }
                }
                putchar('\n');
            }
        }
        return;
    }

    /* З рамкою (box-drawing) */
    int scaled_w = w * scale;

    /* Верхня рамка */
    if (color) printf(ANSI_GRAY);
    printf("  ┌");
    for (int x = 0; x < scaled_w; x++) printf("─");
    printf("┐");
    if (color) printf(ANSI_RESET);
    printf("\n");

    /* Рядки гліфа */
    for (int row = 0; row < h; row++) {
        for (int s = 0; s < scale; s++) {
            if (color) printf(ANSI_GRAY);
            printf("  │");
            if (color) printf(ANSI_RESET);

            for (int byte = 0; byte < bpr; byte++) {
                uint8_t bv = gm->glyph[row * bpr + byte];
                for (int bit = 0; bit < 8; bit++) {
                    int px = byte * 8 + bit;
                    if (px >= w) break;
                    bool on = (bv & (0x80 >> bit)) != 0;
                    char ch = on ? PIXEL_ON : dot_char;
                    if (color) {
                        printf(on ? ANSI_BRIGHT_GREEN : ANSI_GRAY);
                    }
                    for (int sx = 0; sx < scale; sx++) putchar(ch);
                    if (color) printf(ANSI_RESET);
                }
            }

            if (color) printf(ANSI_GRAY);
            printf("│");
            if (color) printf(ANSI_RESET);
            printf("\n");
        }
    }

    /* Нижня рамка */
    if (color) printf(ANSI_GRAY);
    printf("  └");
    for (int x = 0; x < scaled_w; x++) printf("─");
    printf("┘");
    if (color) printf(ANSI_RESET);
    printf("\n");
}

/* ============================================================
   Режим: --all (показати всі гліфи шрифту)
   ============================================================ */
static void show_all(const RasterFont* font, int scale, bool color, int cols) {
    if (color) printf(ANSI_BOLD ANSI_CYAN);
    printf("Font: %s  |  glyphs: %d  |  size: %dx%d  |  bytes/glyph: %d\n",
           font->name, font->glyph_count, font->glyph_width,
           font->glyph_height, font->glyph_bytes);
    if (color) printf(ANSI_RESET);
    printf("\n");

    int rows_per_glyph = font->glyph_height * scale + 3; /* +рамка+заголовок */
    int col_w = font->glyph_width * scale + 6;           /* відступи */

    for (int start = 0; start < font->glyph_count; start += cols) {
        int end = start + cols;
        if (end > font->glyph_count) end = font->glyph_count;

        /* Заголовки стовпців */
        for (int c = start; c < end; c++) {
            if (color) printf(ANSI_YELLOW);
            printf("U+%04X ", font->glyph_map[c].unicode);
            if (color) printf(ANSI_RESET);
            int pad = col_w - 7;
            for (int i = 0; i < pad; i++) putchar(' ');
        }
        printf("\n");

        /* Рядки гліфів */
        for (int row = 0; row < font->glyph_height; row++) {
            for (int s = 0; s < scale; s++) {
                for (int c = start; c < end; c++) {
                    const GlyphPointerMap* gm = &font->glyph_map[c];
                    int idx = c;
                    int w = font->glyph_widths ? font->glyph_widths[idx] : font->glyph_width;
                    int bpr = (w + 7) / 8;
                    int h = font->glyph_heights ? font->glyph_heights[idx] : font->glyph_height;

                    if (row >= h) {
                        /* Порожній рядок для коротких гліфів */
                        for (int x = 0; x < w * scale + 2; x++) putchar(' ');
                    } else {
                        if (color) printf(ANSI_GRAY);
                        printf("│");
                        if (color) printf(ANSI_RESET);

                        for (int byte = 0; byte < bpr; byte++) {
                            uint8_t bv = gm->glyph[row * bpr + byte];
                            for (int bit = 0; bit < 8; bit++) {
                                int px = byte * 8 + bit;
                                if (px >= w) break;
                                bool on = (bv & (0x80 >> bit)) != 0;
                                if (color) {
                                    printf(on ? ANSI_GREEN : ANSI_GRAY);
                                }
                                for (int sx = 0; sx < scale; sx++) {
                                    putchar(on ? PIXEL_ON : PIXEL_OFF);
                                }
                                if (color) printf(ANSI_RESET);
                            }
                        }
                        if (color) printf(ANSI_GRAY);
                        printf("│");
                        if (color) printf(ANSI_RESET);
                    }
                    putchar(' ');
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

/* ============================================================
   Режим: --text (рендер тексту)
   ============================================================ */
static void show_text(const RasterFont* font, const char* text,
                      int scale, bool color) {
    if (color) printf(ANSI_BOLD ANSI_CYAN);
    printf("Rendering text with font: %s\n", font->name);
    if (color) printf(ANSI_RESET);
    printf("\"%s\"\n\n", text);

    /* Рахуємо кількість символів */
    uint32_t cps[512];
    int len = 0;
    const char* p = text;
    while (*p && len < 512) {
        uint32_t cp;
        int bytes = utf8_decode(p, &cp);
        cps[len++] = cp;
        p += bytes;
    }

    /* Рендеримо рядок за рядком */
    int h = font->glyph_height;
    for (int row = 0; row < h; row++) {
        for (int s = 0; s < scale; s++) {
            for (int i = 0; i < len; i++) {
                const GlyphPointerMap* gm = find_glyph(font, cps[i]);
                if (!gm) {
                    /* Пробіл замість відсутнього */
                    int w = font->glyph_width;
                    for (int x = 0; x < w * scale; x++) putchar(' ');
                    putchar(' ');
                    continue;
                }
                int idx = (int)(gm - font->glyph_map);
                int w = font->glyph_widths ? font->glyph_widths[idx] : font->glyph_width;
                int gh = font->glyph_heights ? font->glyph_heights[idx] : font->glyph_height;
                int bpr = (w + 7) / 8;

                if (row >= gh) {
                    for (int x = 0; x < w * scale; x++) putchar(' ');
                } else {
                    for (int byte = 0; byte < bpr; byte++) {
                        uint8_t bv = gm->glyph[row * bpr + byte];
                        for (int bit = 0; bit < 8; bit++) {
                            int px = byte * 8 + bit;
                            if (px >= w) break;
                            bool on = (bv & (0x80 >> bit)) != 0;
                            if (color) {
                                printf(on ? ANSI_GREEN : ANSI_GRAY);
                            }
                            for (int sx = 0; sx < scale; sx++) {
                                putchar(on ? PIXEL_ON : PIXEL_OFF);
                            }
                            if (color) printf(ANSI_RESET);
                        }
                    }
                }
                putchar(' ');
            }
            printf("\n");
        }
    }
}

/* ============================================================
 *  Режим: --info (інформація про шрифт)
 *  ============================================================ */
static void show_info(const RasterFont* font, bool color) {
    if (color) printf(ANSI_BOLD ANSI_CYAN);
    printf("+----------------------------------------------+\n");
    printf("|          RASTER FONT INFO                    |\n");
    printf("+----------------------------------------------+\n");
    if (color) printf(ANSI_RESET);

    if (color) printf(ANSI_YELLOW);
    printf("| Name:          %-29s|\n", font->name ? font->name : "(unnamed)");
    if (color) printf(ANSI_GREEN);
    printf("| Glyph size:    %dx%-25d|\n", font->glyph_width, font->glyph_height);
    if (color) printf(ANSI_GREEN);
    printf("| Bytes/glyph:   %-29d|\n", font->glyph_bytes);
    if (color) printf(ANSI_MAGENTA);
    printf("| Glyph count:   %-29d|\n", font->glyph_count);
    if (color) printf(ANSI_BLUE);
    printf("| Has widths:    %-29s|\n", font->glyph_widths ? "yes" : "no");
    printf("| Has heights:   %-29s|\n", font->glyph_heights ? "yes" : "no");
    printf("| Has v-offsets: %-29s|\n", font->glyph_vertical_offsets ? "yes" : "no");
    printf("| Has h-offsets: %-29s|\n", font->glyph_horizontal_offsets ? "yes" : "no");
    if (color) printf(ANSI_CYAN);
    printf("+----------------------------------------------+\n");
    if (color) printf(ANSI_RESET);

    /* Список перших 16 гліфів */
    printf("\nFirst 16 glyphs:\n");
    int n = font->glyph_count < 16 ? font->glyph_count : 16;
    for (int i = 0; i < n; i++) {
        char utf8_buf[5] = {0};
        utf8_encode(font->glyph_map[i].unicode, utf8_buf);
        if (color) printf(ANSI_YELLOW);
        printf("  [%2d] U+%04X ", i, font->glyph_map[i].unicode);
        if (color) printf(ANSI_WHITE);
        printf("'%s'", utf8_buf);
        if (color) printf(ANSI_RESET);
        printf("\n");
    }
}

/* ============================================================
   Help
   ============================================================ */
static void print_help(const char* prog) {
    printf("raster_font_console_viewer — перегляд растрових шрифтів\n\n");
    printf("Usage: %s [options]\n\n", prog);
    printf("Options:\n");
    printf("  --font <name>      Font name: Terminus12x6, FreePixel, Pixel\n");
    printf("                     (default: Terminus12x6)\n");
    printf("  --all              Show all glyphs in table mode\n");
    printf("  --char <hex>       Show single glyph (e.g. 0x0410, 0x41, 65)\n");
    printf("  --text \"...\"       Render UTF-8 text\n");
    printf("  --info             Show font metadata\n");
    printf("  --scale <N>        Scale factor (default: 1)\n");
    printf("  --cols <N>         Columns for --all mode (default: 8)\n");
    printf("  --color            Enable ANSI colors\n");
    printf("  --no-frame         Disable frame around glyph\n");
    printf("  --list-fonts       List available fonts\n");
    printf("  --help             Show this help\n");
    printf("\nExamples:\n");
    printf("  %s --font Terminus12x6 --char 0x0410 --color\n", prog);
    printf("  %s --font FreePixel --text \"Привіт!\" --scale 2 --color\n", prog);
    printf("  %s --font Pixel --all --cols 10 --color\n", prog);
    printf("  %s --font Terminus12x6 --info\n", prog);
}

static void list_fonts(void) {
    printf("Available fonts:\n");
    for (int i = 0; g_fonts[i].name != NULL; i++) {
        const RasterFont* f = g_fonts[i].font;
        printf("  %-15s  %dx%d  %d glyphs  %d bytes/glyph\n",
               g_fonts[i].name, f->glyph_width, f->glyph_height,
               f->glyph_count, f->glyph_bytes);
    }
}

/* ============================================================
   Парсинг аргументів
   ============================================================ */
typedef struct {
    const char* font_name;
    const char* text;
    uint32_t    char_code;
    int         scale;
    int         cols;
    bool        color;
    bool        show_frame;
    bool        mode_all;
    bool        mode_char;
    bool        mode_text;
    bool        mode_info;
    bool        mode_list;
    bool        mode_help;
} Options;

static uint32_t parse_code(const char* s) {
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        return (uint32_t)strtoul(s, NULL, 16);
    }
    /* Перевіряємо, чи всі цифри */
    bool all_digits = true;
    for (const char* p = s; *p; p++) {
        if (!isdigit((unsigned char)*p)) { all_digits = false; break; }
    }
    if (all_digits) return (uint32_t)strtoul(s, NULL, 10);
    /* Інакше — hex */
    return (uint32_t)strtoul(s, NULL, 16);
}

static const RasterFont* select_font(const char* name) {
    for (int i = 0; g_fonts[i].name != NULL; i++) {
        if (strcasecmp(g_fonts[i].name, name) == 0)
            return g_fonts[i].font;
    }
    return NULL;
}

int main(int argc, char** argv) {
    Options opt = {
        .font_name  = "Terminus12x6",
        .text       = NULL,
        .char_code  = 0,
        .scale      = 1,
        .cols       = 8,
        .color      = false,
        .show_frame = true,
        .mode_all   = false,
        .mode_char  = false,
        .mode_text  = false,
        .mode_info  = false,
        .mode_list  = false,
        .mode_help  = false
    };

    /* Парсимо аргументи */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--font") == 0 && i + 1 < argc) {
            opt.font_name = argv[++i];
        } else if (strcmp(argv[i], "--char") == 0 && i + 1 < argc) {
            opt.char_code = parse_code(argv[++i]);
            opt.mode_char = true;
        } else if (strcmp(argv[i], "--text") == 0 && i + 1 < argc) {
            opt.text = argv[++i];
            opt.mode_text = true;
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            opt.scale = atoi(argv[++i]);
            if (opt.scale < 1) opt.scale = 1;
            if (opt.scale > 8) opt.scale = 8;
        } else if (strcmp(argv[i], "--cols") == 0 && i + 1 < argc) {
            opt.cols = atoi(argv[++i]);
            if (opt.cols < 1) opt.cols = 1;
            if (opt.cols > 20) opt.cols = 20;
        } else if (strcmp(argv[i], "--all") == 0) {
            opt.mode_all = true;
        } else if (strcmp(argv[i], "--info") == 0) {
            opt.mode_info = true;
        } else if (strcmp(argv[i], "--list-fonts") == 0) {
            opt.mode_list = true;
        } else if (strcmp(argv[i], "--color") == 0) {
            opt.color = true;
        } else if (strcmp(argv[i], "--no-frame") == 0) {
            opt.show_frame = false;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            opt.mode_help = true;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_help(argv[0]);
            return 1;
        }
    }

    /* --list-fonts */
    if (opt.mode_list) {
        list_fonts();
        return 0;
    }

    /* --help */
    if (opt.mode_help) {
        print_help(argv[0]);
        return 0;
    }

    /* Вибір шрифту */
    const RasterFont* font = select_font(opt.font_name);
    if (!font) {
        fprintf(stderr, "Font not found: %s\n", opt.font_name);
        fprintf(stderr, "Available fonts:\n");
        for (int i = 0; g_fonts[i].name != NULL; i++) {
            fprintf(stderr, "  - %s\n", g_fonts[i].name);
        }
        return 1;
    }

    /* Виконуємо режим */
    if (opt.mode_info) {
        show_info(font, opt.color);
    } else if (opt.mode_all) {
        show_all(font, opt.scale, opt.color, opt.cols);
    } else if (opt.mode_char) {
        draw_glyph(font, opt.char_code, opt.scale, opt.color, opt.show_frame, PIXEL_OFF);
    } else if (opt.mode_text) {
        show_text(font, opt.text, opt.scale, opt.color);
    } else {
        /* За замовчуванням — info */
        printf("No mode specified. Use --help for usage.\n\n");
        show_info(font, opt.color);
    }

    return 0;
}
