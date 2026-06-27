// lines.c

#include "graphics.h"
#include "gfx.h"

// малювання лінії окремими пікселями з передачою кольору в функцію.
void DrawThinLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color)
{
    // Розрахунок напрямку лінії
    int delta_x = x2 - x1;
    int delta_y = y2 - y1;
    float length = sqrt(delta_x * delta_x + delta_y * delta_y); // довжина лінії

    // Нормалізуємо напрямок лінії
    float unit_x = delta_x / length;
    float unit_y = delta_y / length;

    // Розрахунок напрямку перпендикулярного вектора для товщини лінії
    float perp_x = -unit_y;  // Перпендикулярний вектор
    float perp_y = unit_x;

    // Малюємо товсту лінію
    for (int i = -(thickness / 2); i <= thickness / 2; i++) {
        // Зміщення для товщини лінії
        int offsetX = (int)(perp_x * i);
        int offsetY = (int)(perp_y * i);

        // Розрахунок точок для малювання товстої лінії
        int x = x1;
        int y = y1;
        int xerr = 0, yerr = 0;
        int distance = (abs(delta_x) > abs(delta_y)) ? abs(delta_x) : abs(delta_y);

        for (int t = 0; t <= distance; t++) {
            // Малюємо пікселі з зміщенням для товщини
            DrawPixel(x + offsetX, y + offsetY,color);
            // DrawRectangle(x + offsetX, y + offsetY, 1, 1, color);

            xerr += abs(delta_x);
            yerr += abs(delta_y);

            if (xerr > distance) {
                xerr -= distance;
                x += (delta_x > 0) ? 1 : -1;
            }
            if (yerr > distance) {
                yerr -= distance;
                y += (delta_y > 0) ? 1 : -1;
            }
        }
    }
}

// малювання лінії окремими квадратиками з передачою кольору в функцію.
void DrawThickLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color)
{
    // Розрахунок напрямку лінії
    int delta_x = x2 - x1;
    int delta_y = y2 - y1;
    float length = sqrt(delta_x * delta_x + delta_y * delta_y); // довжина лінії

    // Нормалізуємо напрямок лінії
    float unit_x = delta_x / length;
    float unit_y = delta_y / length;

    // Розрахунок напрямку перпендикулярного вектора для товщини лінії
    float perp_x = -unit_y;  // Перпендикулярний вектор
    float perp_y = unit_x;

    // Множимо на товщину, щоб збільшити відстань для товщини
    float offset_perp_x = perp_x * thickness / 2.0f;
    float offset_perp_y = perp_y * thickness / 2.0f;

    // Малюємо товсту лінію
    for (int i = -(thickness / 2); i <= thickness / 2; i++) {
        // Зміщення для товщини лінії
        int offsetX = (int)(offset_perp_x * i);
        int offsetY = (int)(offset_perp_y * i);

        // Розрахунок точок для малювання товстої лінії
        int x = x1;
        int y = y1;
        int xerr = 0, yerr = 0;
        int distance = (abs(delta_x) > abs(delta_y)) ? abs(delta_x) : abs(delta_y);

        for (int t = 0; t <= distance; t++) {
            // Малюємо квадрат (thickness) пікселя з зміщенням для товщини
            // DrawPixel(x + offsetX, y + offsetY,color);
            DrawRectangle(x + offsetX, y + offsetY, thickness, thickness, color);

            xerr += abs(delta_x);
            yerr += abs(delta_y);

            if (xerr > distance) {
                xerr -= distance;
                x += (delta_x > 0) ? 1 : -1;
            }
            if (yerr > distance) {
                yerr -= distance;
                y += (delta_y > 0) ? 1 : -1;
            }
        }
    }
}

// малювання лінії окремими квадратиками з передачою кольору в функцію.
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thickness, uint32_t color)
{
    int x1 = startPos.x;
    int y1 = startPos.y;
    int x2 = endPos.x;
    int y2 = endPos.y;

    // Розрахунок напрямку лінії
    int delta_x = x2 - x1;
    int delta_y = y2 - y1;
    float length = sqrt(delta_x * delta_x + delta_y * delta_y); // довжина лінії

    // Нормалізуємо напрямок лінії
    float unit_x = delta_x / length;
    float unit_y = delta_y / length;

    // Розрахунок напрямку перпендикулярного вектора для товщини лінії
    float perp_x = -unit_y;  // Перпендикулярний вектор
    float perp_y = unit_x;

    // Множимо на товщину, щоб збільшити відстань для товщини
    float offset_perp_x = perp_x * thickness / 2.0f;
    float offset_perp_y = perp_y * thickness / 2.0f;

    // Малюємо товсту лінію
    for (int i = -(thickness / 2); i <= thickness / 2; i++) {
        // Зміщення для товщини лінії
        int offsetX = (int)(offset_perp_x * i);
        int offsetY = (int)(offset_perp_y * i);

        // Розрахунок точок для малювання товстої лінії
        int x = x1;
        int y = y1;
        int xerr = 0, yerr = 0;
        int distance = (abs(delta_x) > abs(delta_y)) ? abs(delta_x) : abs(delta_y);

        for (int t = 0; t <= distance; t++) {
            // Малюємо квадрат (thickness) пікселя з зміщенням для товщини
            // DrawPixel(x + offsetX, y + offsetY,color);
            DrawRectangle(x + offsetX, y + offsetY, thickness, thickness, color);

            xerr += abs(delta_x);
            yerr += abs(delta_y);

            if (xerr > distance) {
                xerr -= distance;
                x += (delta_x > 0) ? 1 : -1;
            }
            if (yerr > distance) {
                yerr -= distance;
                y += (delta_y > 0) ? 1 : -1;
            }
        }
    }
}

// Малювання заповненого прямокутника кольором color (у форматі 0xRRGGBB)
void DrawRectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t color)
{
    for (int16_t py = y; py < y + height; py++) {
        for (int16_t px = x; px < x + width; px++) {
            DrawPixel(px, py, color);
        }
    }
}

// Малювання не заповненого прямокутника кольором color (у форматі 0xRRGGBB)
void DrawRectangleLines(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t color)
{
    // Верхня горизонтальна лінія
    for (int16_t px = x; px < x + width; px++) {
        DrawPixel(px, y, color);
    }
    // Нижня горизонтальна лінія
    for (int16_t px = x; px < x + width; px++) {
        DrawPixel(px, y + height - 1, color);
    }
    // Ліва вертикальна лінія
    for (int16_t py = y; py < y + height; py++) {
        DrawPixel(x, py, color);
    }
    // Права вертикальна лінія
    for (int16_t py = y; py < y + height; py++) {
        DrawPixel(x + width - 1, py, color);
    }
}
