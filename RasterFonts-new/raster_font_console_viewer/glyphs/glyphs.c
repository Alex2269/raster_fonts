// glyphs.c

#include <stdio.h>
#include <string.h>

#include "glyphs.h"    // структури RasterFont та glyph_map

/*
 * utf8_strlen - підрахунок кількості Unicode символів у UTF-8 рядку.
 * Стандартна strlen рахує байти, а не символи, що може спотворювати довжину тексту,
 * особливо для кирилиці та інших багатобайтових символів.
 */
int utf8_strlen(const char* s) {
    int len = 0;
    // Проходимо по рядку, декодуючи кожен UTF-8 символ
    while (*s) {
        uint32_t codepoint = 0;
        int bytes = utf8_decode(s, &codepoint); // розпаковуємо один символ
        s += bytes; // переходимо до наступного символу
        len++; // лічильник символів
    }
    return len;
}

/*
 * utf8_decode - декодування одного UTF-8 символу з початку рядка str.
 * Записує decodед Unicode код символу у out_codepoint.
 * Повертає довжину символу у байтах (1-4).
 * Якщо символ некоректний, повертає 1 і код 0.
 */
/* ============================================================
 *   UTF-8 utilities
 *   ============================================================ */
int utf8_decode(const char* str, uint32_t* out) {
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80) {
        *out = c; return 1;
    } else if ((c & 0xE0) == 0xC0) {
        *out = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        *out = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        *out = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) |
        ((str[2] & 0x3F) << 6)  | (str[3] & 0x3F);
        return 4;
    }
    *out = 0;
    return 1;
}

int utf8_encode(uint32_t cp, char* out) {
    if (cp < 0x80) {
        out[0] = (char)cp; return 1;
    } else if (cp < 0x800) {
        out[0] = 0xC0 | (cp >> 6);
        out[1] = 0x80 | (cp & 0x3F);
        return 2;
    } else if (cp < 0x10000) {
        out[0] = 0xE0 | (cp >> 12);
        out[1] = 0x80 | ((cp >> 6) & 0x3F);
        out[2] = 0x80 | (cp & 0x3F);
        return 3;
    } else {
        out[0] = 0xF0 | (cp >> 18);
        out[1] = 0x80 | ((cp >> 12) & 0x3F);
        out[2] = 0x80 | ((cp >> 6) & 0x3F);
        out[3] = 0x80 | (cp & 0x3F);
        return 4;
    }
}

