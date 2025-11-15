// main.c

#include "main.h"
#include "glyphs.h"
#include "all_font.h" // Опис шрифтів як структури

int main(void) {
    const int screenWidth = 420;
    const int screenHeight = 340;

    int osc_width = screenWidth;
    int osc_height = screenHeight - 115;

    gfx_open(screenWidth,screenHeight,"PSF_Font renderer");
    Display_Set_WIDTH(screenWidth);
    Display_Set_HEIGHT(screenHeight);
    gfx_color(128,127,255);

    DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
    DrawRectangle(0, 0, osc_width, osc_height, BLACK);

    int scale = 2; // масштаб 1x
    int spacing = 1; // простір між символами px
    int padding = 5;
    int borderThickness = 1;

    DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 10, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, YELLOW, padding, borderThickness);
    DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 72, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, GREEN, padding, borderThickness);
    DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 134, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, BLUE, padding, borderThickness);
    DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 196, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, RED, padding, borderThickness);

    DrawTextScaled(Terminus12x6_font, 30, 260, "Масштабований текст x2", spacing, 2, BLUE); // масштаб 2x

    while(1) {
        gfx_flush();
        usleep(100);
    }

    // Після виходу з циклу звільняємо пам'ять шрифту

    return 0;
}


