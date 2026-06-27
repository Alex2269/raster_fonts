// main.c

#include "raylib.h"

#include "all_font.h" // Опис шрифтів як структури RasterFont
#include "glyphs.h"

int main(void) {
    const int screenWidth = 420;
    const int screenHeight = 340;

    int osc_width = screenWidth;
    int osc_height = screenHeight - 115;

    // Встановлюємо прапорець для мультисемплінгу (покращення якості графіки)
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "RasterFont renderer");

    SetTargetFPS(60);

    int scale = 2; // масштаб 1x
    int spacing = 1; // простір між символами px
    int padding = 5;
    int borderThickness = 1;

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

        DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 10,
                                           "Масштабований текст\nз інверсним фоном",
                                           spacing, scale, YELLOW, padding, borderThickness);
        DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 72,
                                           "Масштабований текст\nз інверсним фоном",
                                           spacing, scale, GREEN, padding, borderThickness);
        DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 134,
                                           "Масштабований текст\nз інверсним фоном",
                                           spacing, scale, BLUE, padding, borderThickness);
        DrawTextWithAutoInvertedBackground(Terminus12x6_font, 20, 196,
                                           "Масштабований текст\nз інверсним фоном",
                                           spacing, scale, RED, padding, borderThickness);

        DrawTextScaled(Pixel_font, 30, 260, "Масштабований текст x2", spacing, scale, BLUE); // масштаб 2x
        DrawTextScaled(FreePixel_font, 30, 290, "Масштабований текст x2", spacing, scale, RED); // масштаб 2x

        EndDrawing();
    }

    // Після виходу з циклу звільняємо пам'ять шрифту

    CloseWindow();

    return 0;
}


