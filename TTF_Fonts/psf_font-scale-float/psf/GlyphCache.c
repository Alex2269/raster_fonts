// GlyphCache.c
#include "GlyphCache.h"
#include "GlyphToImage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Максимальна кількість одночасно кешованих шрифтів
#define MAX_CACHED_FONTS 16

// Структура для зберігання кешу одного шрифту:
// зберігає копію шрифту (щоб ідентифікувати його) та кеш гліфів
typedef struct {
    PSF_Font font;       // Копія структури шрифту (для порівняння та пошуку)
    GlyphCache cache;    // Кеш гліфів для цього шрифту
} FontCacheEntry;

// Статичний масив кешів для різних шрифтів
static FontCacheEntry g_fontCaches[MAX_CACHED_FONTS];

// Поточна кількість кешованих шрифтів
static int g_fontCacheCount = 0;

// Ініціалізує кеш гліфів: виділяє пам’ять під масив текстур і обнуляє їх
void GlyphCache_Init(GlyphCache* cache, int glyph_count) {
    if (!cache) return;

    cache->glyph_count = glyph_count;

    // Виділяємо пам’ять під масив текстур розміром glyph_count
    cache->glyphTextures = (Texture2D*)calloc(glyph_count, sizeof(Texture2D));
    if (!cache->glyphTextures) {
        fprintf(stderr, "Помилка виділення пам’яті для glyphTextures\n");
        cache->glyph_count = 0;
    }
}

// Звільняє всі текстури гліфів у кеші та очищує пам’ять
void GlyphCache_Unload(GlyphCache* cache) {
    if (!cache || !cache->glyphTextures) return;

    // Проходимо по всіх текстурах і звільняємо їх, якщо вони існують
    for (int i = 0; i < cache->glyph_count; i++) {
        if (cache->glyphTextures[i].id != 0) {
            UnloadTexture(cache->glyphTextures[i]);
            cache->glyphTextures[i].id = 0;
        }
    }

    // Звільняємо масив текстур
    free(cache->glyphTextures);
    cache->glyphTextures = NULL;
    cache->glyph_count = 0;
}

// Повертає текстуру гліфа з кешу, створює її при відсутності
// Текстура створюється у білому кольорі (монохром), колір задається при малюванні
Texture2D GlyphCache_GetTexture(GlyphCache* cache, PSF_Font font, int glyphIndex, float scale) {
    // Якщо індекс гліфа некоректний — замінюємо на пробіл (32)
    if (!cache || glyphIndex < 0 || glyphIndex >= cache->glyph_count) glyphIndex = 32;

    // Якщо текстура для цього гліфа ще не створена — створюємо
    if (cache->glyphTextures[glyphIndex].id == 0) {
        // Створюємо текстуру гліфа з білим кольором (WHITE)
        cache->glyphTextures[glyphIndex] = GlyphToTexture(font, glyphIndex, scale, WHITE);
    }

    // Повертаємо текстуру з кешу
    return cache->glyphTextures[glyphIndex];
}

// Малює текстуру гліфа з масштабуванням і заданим кольором
void DrawPSFCharScaledTexture(Texture2D tex, int x, int y, float scale, Color color) {
    Rectangle sourceRec = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Rectangle destRec = { (float)x, (float)y, tex.width * scale, tex.height * scale };
    Vector2 origin = { 0.0f, 0.0f };

    // Малюємо текстуру з переданим кольором (колір задається тут)
    DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, color);
}

// Внутрішня функція пошуку кешу для конкретного шрифту за унікальним вказівником на glyphBuffer
static GlyphCache* GetCacheForFont(PSF_Font font) {
    // Перевіряємо, чи кеш для цього шрифту вже існує
    for (int i = 0; i < g_fontCacheCount; i++) {
        if (g_fontCaches[i].font.glyphBuffer == font.glyphBuffer) {
            // Знайшли існуючий кеш — повертаємо його
            return &g_fontCaches[i].cache;
        }
    }

    // Якщо кеш не знайдено, створюємо новий, якщо є вільне місце
    if (g_fontCacheCount < MAX_CACHED_FONTS) {
        g_fontCaches[g_fontCacheCount].font = font;
        GlyphCache_Init(&g_fontCaches[g_fontCacheCount].cache, font.glyph_count);
        g_fontCacheCount++;
        return &g_fontCaches[g_fontCacheCount - 1].cache;
    }

    // Якщо кешів забагато — виводимо помилку і повертаємо NULL
    fprintf(stderr, "Перевищено максимальну кількість кешів шрифтів (%d)\n", MAX_CACHED_FONTS);
    return NULL;
}

// Малює UTF-8 текст шрифтом PSF з динамічним кешем гліфів,
// підтримує багатошрифтовість і різні кольори
void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, float scale, Color color) {
    // Отримуємо кеш для заданого шрифту
    GlyphCache* cache = GetCacheForFont(font);
    if (!cache) return; // Якщо кеш не створено — нічого не малюємо

    int xpos = x;
    int ypos = y;

    while (*text) {
        // Обробка символу нового рядка
        if (*text == '\n') {
            xpos = x; // повертаємось у початок рядка
            ypos += (int)((font.glyph_height * scale) + spacing); // переходимо на наступний рядок
            text++;
            continue;
        }

        uint32_t codepoint = 0;
        // Декодуємо один UTF-8 символ і отримуємо Unicode код
        int bytes = utf8_decode(text, &codepoint);

        // Знаходимо індекс гліфа для Unicode коду
        int glyph_index = UnicodeToGlyphIndex(codepoint);

        // Якщо індекс некоректний — замінюємо на пробіл
        if (glyph_index < 0 || glyph_index >= font.glyph_count) glyph_index = 32;

        // Отримуємо текстуру гліфа з кешу (монохромну)
        Texture2D glyphTex = GlyphCache_GetTexture(cache, font, glyph_index, scale);

        // Малюємо текстуру гліфа з потрібним кольором
        DrawPSFCharScaledTexture(glyphTex, xpos, ypos, scale, color);

        // Зсуваємо позицію по горизонталі для наступного символу
        xpos += (int)((font.glyph_width * scale) + spacing);

        // Переходимо до наступного символу у тексті
        text += bytes;
    }
}

// Звільняє всі кеші гліфів для всіх шрифтів
void GlyphCache_ClearAllCaches(void) {
    for (int i = 0; i < g_fontCacheCount; i++) {
        GlyphCache_Unload(&g_fontCaches[i].cache);
    }
    g_fontCacheCount = 0;
}

