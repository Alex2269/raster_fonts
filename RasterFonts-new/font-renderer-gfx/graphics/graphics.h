

#ifndef _LINES_H
#define _LINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "color.h"

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

extern uint16_t BACK_COLOR;
extern uint16_t POINT_COLOR;

// Vector2, 2 components
typedef struct Vector2 {
    float x;                // Vector x component
    float y;                // Vector y component
} Vector2;

// малювання лінії окремими пікселями з передачою кольору в функцію.
void DrawThinLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color);
// малювання лінії окремими квадратиками з передачою кольору в функцію.
void DrawThickLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color);
// малювання лінії окремими квадратиками з передачою кольору в функцію.
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thickness, uint32_t color);

// Малювання заповненого прямокутника кольором color (у форматі 0xRRGGBB)
void DrawRectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t color);
// Малювання не заповненого прямокутника кольором color (у форматі 0xRRGGBB)
void DrawRectangleLines(int16_t x, int16_t y, int16_t width, int16_t height, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* _LINES_H */

