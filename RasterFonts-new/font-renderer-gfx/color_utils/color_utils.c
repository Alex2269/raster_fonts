// color_utils.c

#include "color_utils.h"

// Допоміжна функція для конвертації RGB (uint32_t) у компоненти float 0..1
void RGBtoFloatComponents(uint32_t color, float *r, float *g, float *b) {
    *r = ((color >> 16) & 0xFF) / 255.0f;
    *g = ((color >> 8) & 0xFF) / 255.0f;
    *b = (color & 0xFF) / 255.0f;
}

// Допоміжна функція для конвертації float-компонент 0..1 у uint32_t RGB
uint32_t FloatComponentsToRGB(float r, float g, float b) {
    uint32_t R = (uint32_t)(r * 255.0f) & 0xFF;
    uint32_t G = (uint32_t)(g * 255.0f) & 0xFF;
    uint32_t B = (uint32_t)(b * 255.0f) & 0xFF;
    return (R << 16) | (G << 8) | B;
}

// Обчислення яскравості кольору (luminance), потрібно для вибору контрасту
float GetLuminance(uint32_t color) {
    // Витягуємо компоненти R, G, B з кольору у форматі 0xRRGGBB
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    // Обчислюємо яскравість (luminance)
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір білого або чорного кольору, щоб текст був контрастним до фону
uint32_t GetContrastColor(uint32_t color) {
    return (GetLuminance(color) > 0.5f) ? BLACK : WHITE;
}

// Зміна насиченості (saturation) кольору у HSV просторі
uint32_t ChangeSaturation(uint32_t color, float saturationScale) {
    float r, g, b;
    RGBtoFloatComponents(color, &r, &g, &b);

    float cMax = fmaxf(r, fmaxf(g, b));
    float cMin = fminf(r, fminf(g, b));
    float delta = cMax - cMin;

    float h = 0.0f;
    if (delta > 0) {
        if (cMax == r)
            h = fmodf((g - b) / delta, 6.0f);
        else if (cMax == g)
            h = (b - r) / delta + 2.0f;
        else
            h = (r - g) / delta + 4.0f;
        h *= 60.0f;
        if (h < 0) h += 360.0f;
    }
    float s = (cMax == 0) ? 0 : delta / cMax;
    float v = cMax;

    s *= saturationScale;
    if (s > 1.0f) s = 1.0f;

    float cVal = v * s;
    float xVal = cVal * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - cVal;

    float nr = 0, ng = 0, nb = 0;

    if (h >= 0 && h < 60) {
        nr = cVal; ng = xVal; nb = 0;
    } else if (h >= 60 && h < 120) {
        nr = xVal; ng = cVal; nb = 0;
    } else if (h >= 120 && h < 180) {
        nr = 0; ng = cVal; nb = xVal;
    } else if (h >= 180 && h < 240) {
        nr = 0; ng = xVal; nb = cVal;
    } else if (h >= 240 && h < 300) {
        nr = xVal; ng = 0; nb = cVal;
    } else if (h >= 300 && h < 360) {
        nr = cVal; ng = 0; nb = xVal;
    }

    // Додаємо m до кожної компоненти і конвертуємо назад в 0..1
    nr = nr + m;
    ng = ng + m;
    nb = nb + m;

    return FloatComponentsToRGB(nr, ng, nb);
}

// Функція інверсії кольору (кольори RGB інвертуємо, альфа залишаємо)
// Перевірка, чи є колір темним (яскравість нижча за 0.5)
// Отримання контрастного інверсного кольору для фону із коригуванням насиченості і яскравості

// Інверсія кольору (інвертуємо R,G,B)
uint32_t InvertColor(uint32_t color) {
    uint8_t r = 255 - ((color >> 16) & 0xFF);
    uint8_t g = 255 - ((color >> 8) & 0xFF);
    uint8_t b = 255 - (color & 0xFF);
    return (r << 16) | (g << 8) | b;
}

// Перевірка, чи є колір темним (яскравість нижча за 0.5)
int IsColorDark(uint32_t color) {
    return GetLuminance(color) < 0.5f;
}

// Отримання контрастного інверсного кольору для фону із коригуванням насиченості і яскравості
uint32_t GetContrastInvertColor(uint32_t color) {
    uint32_t invColor = InvertColor(color);
    if (IsColorDark(color)) {
        // Якщо текст темний, робимо фон світлим і насиченим
        invColor = ChangeSaturation(invColor, 0.35f);

        uint8_t r = (((invColor >> 16) & 0xFF) + 255) / 2;
        uint8_t g = (((invColor >> 8)  & 0xFF) + 255) / 2;
        uint8_t b = ((invColor & 0xFF) + 255) / 2;

        invColor = (r << 16) | (g << 8) | b;
    } else {
        // Якщо текст світлий, фон темніший і менш насичений
        invColor = ChangeSaturation(invColor, 0.65f);

        uint8_t r = ((invColor >> 16) & 0xFF) / 2;
        uint8_t g = ((invColor >> 8)  & 0xFF) / 2;
        uint8_t b = (invColor & 0xFF) / 2;

        invColor = (r << 16) | (g << 8) | b;
    }
    return invColor;
}

