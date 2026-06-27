// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include <stdbool.h>
#include <stdint.h>

#include "raylib.h"
#include "main.h"

#include "psf_font.h"  // заголовок із парсером PSF
PSF_Font psfFont;      // Глобальна змінна для PSF шрифту
PSF_Font psfFont12;    // Інший PSF шрифт

int fontSize = 24;
int LineSpacing = 0;

int main(void) {
    const int screenWidth = 420;
    const int screenHeight = 340;

    int osc_width = screenWidth;
    int osc_height = screenHeight - 115;

    // Встановлюємо прапорець для мультисемплінгу (покращення якості графіки)
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "PSF_Font renderer");

    // Завантаження PSF шрифту (шлях до вашого файлу)
    psfFont = LoadPSFFont("fonts/Uni3-TerminusBold32x16.psf");
    psfFont12 = LoadPSFFont("fonts/Uni3-Terminus12x6.psf");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // frameTime += GetFrameTime();
        // if (frameTime * 1000.0f >= oscData.refresh_rate_ms) {
        //     read_usb_device(&oscData);
        //     update_trigger_indices(&oscData);
        //     frameTime = 0.0f;
        // }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, osc_width, osc_height, BLACK);

        // // DrawPSFText(psfFont, 20, 20, "Текст UTF-8", 0, WHITE);
        // DrawPSFTextScaled(psfFont12, 20, 10, "Масштабований текст x2", 0, 2, RED); // масштаб 2x
        // DrawPSFTextScaled(psfFont12, 20, 45, "Масштабований текст x2", 0, 2, YELLOW); // масштаб 2x
        //
        // DrawPSFText(psfFont, 20, osc_height + 10, "Текст UTF-8", 0, GREEN);
        // DrawPSFText(psfFont12, 20, osc_height + 52,  "Малий Текст UTF-8", 0, BLUE);
        DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 10, "Масштабований текст\nз інверсним фоном",
                                                1, 2, YELLOW, 5, 1);
        DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 72, "Масштабований текст\nз інверсним фоном",
                                                1, 2, GREEN, 5, 1);
        // DrawPSFTextWithInvertedBackground(psfFont, 10, 140, "Текст з інверсним фоном", 1, RED, 4);
        DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 134, "Масштабований текст\nз інверсним фоном",
                                                1, 2, BLUE, 5, 1);
        // DrawPSFTextWithInvertedBackground(psfFont, 10, 140, "Текст з інверсним фоном", 1, RED, 4);
        DrawPSFTextScaledWithInvertedBackground(psfFont12, 20, 196, "Масштабований текст\nз інверсним фоном",
                                                1, 2, RED, 5, 1);

        // DrawPSFTextScaled(psfFont12, 10, 240, "Масштабований текст x2", 0, 2, RED); // масштаб 2x
        DrawPSFTextScaled(psfFont12, 30, 260, "Масштабований текст x2", 0, 2, BLUE); // масштаб 2x

        EndDrawing();
    }

    // Після виходу з циклу звільняємо пам'ять шрифту
    UnloadPSFFont(psfFont);

    CloseWindow();

    return 0;
}


