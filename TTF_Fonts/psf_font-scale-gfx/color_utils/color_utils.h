// color_utils.h

#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include "color.h"
#include <stdint.h>
#include <math.h>

// Допоміжна функція для конвертації RGB (uint32_t) у компоненти float 0..1
void RGBtoFloatComponents(uint32_t color, float *r, float *g, float *b);

// Допоміжна функція для конвертації float-компонент 0..1 у uint32_t RGB
uint32_t FloatComponentsToRGB(float r, float g, float b);

// Обчислення яскравості кольору (luminance), потрібно для вибору контрасту
float GetLuminance(uint32_t color);

// Вибір білого або чорного кольору, щоб текст був контрастним до фону
uint32_t GetContrastColor(uint32_t color);

// Зміна насиченості (saturation) кольору у HSV просторі
uint32_t ChangeSaturation(uint32_t color, float saturationScale);

// Інверсія кольору (інвертуємо R,G,B)
uint32_t InvertColor(uint32_t color);

// Перевірка, чи є колір темним (яскравість нижча за 0.5)
int IsColorDark(uint32_t color);

// Отримання контрастного інверсного кольору для фону із коригуванням насиченості і яскравості
uint32_t GetContrastInvertColor(uint32_t color);


#endif // COLOR_UTILS_H

