// main.c
#include "main.h"

PSF_Font psfFont12;    // Глобальна змінна для PSF шрифту
PSF_Font psfFont20;    // Інший PSF шрифт
PSF_Font psfFont28;
PSF_Font psfFont32;

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

    // Завантаження PSF шрифту (шлях до вашого файлу)
    psfFont12 = LoadPSFFont("fonts/Uni3-Terminus12x6.psf");
    psfFont20 = LoadPSFFont("fonts/Uni3-Terminus20x10.psf");
    psfFont28 = LoadPSFFont("fonts/Uni3-Terminus28x14.psf");
    psfFont32 = LoadPSFFont("fonts/Uni3-Terminus32x16.psf");

    int scale = 2; // масштаб 1x
    int spacing = 1; // простір між символами px
    int padding = 5;
    int borderThickness = 1;

    /*
    DrawPSFTextWithInvertedBackground(psfFont28, 20, 10, "Текст UTF-8", spacing, YELLOW, padding, borderThickness);
    DrawPSFTextWithInvertedBackground(psfFont12, 240,10, "Малий Текст", spacing, CYAN, padding, borderThickness);
    DrawPSFTextWithInvertedBackground(psfFont28, 20, 52, "Текст UTF-8", spacing, GREEN, padding, borderThickness);
    DrawPSFTextWithInvertedBackground(psfFont28, 20, 94, "Текст UTF-8", spacing, BLUE, padding, borderThickness);
    */

    DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 10, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, YELLOW, padding, borderThickness);
    DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 72, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, GREEN, padding, borderThickness);
    DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 134, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, BLUE, padding, borderThickness);
    DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 196, "Масштабований текст\nз інверсним фоном",
                                       spacing, scale, RED, padding, borderThickness);

    DrawPSFTextScaled(psfFont12, 30, 260, "Масштабований текст x2", spacing, 2, BLUE); // масштаб 2x

    while(1) {
        gfx_flush();
        usleep(100);
    }

    // Після виходу з циклу звільняємо пам'ять шрифту
    UnloadPSFFont(psfFont12);
    UnloadPSFFont(psfFont20);
    UnloadPSFFont(psfFont28);
    UnloadPSFFont(psfFont32);

    return 0;
}


