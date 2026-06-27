// main.c
#include "raylib.h"
#include "psf_font.h"
#include "GlyphCache.h"
#include "draw.h"

PSF_Font font12;
PSF_Font font18;
PSF_Font font32;

int main(void) {
    InitWindow(800, 450, "PSF Font with Multi-Font Glyph Cache");

    font12 = LoadPSFFont("fonts/Uni3-Terminus12x6.psf");
    font18 = LoadPSFFont("fonts/Uni3-TerminusBold18x10.psf");
    font32 = LoadPSFFont("fonts/Uni3-TerminusBold32x16.psf");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        drawTxt();

        // DrawPSFText(font12, 40, 20, "12", 1, 3.0f, SKYBLUE);
        // DrawPSFText(font18, 40, 100, "18", 1, 2.0f, YELLOW);
        // DrawPSFText(font32, 40, 140, "32", 1, 1.25f, GREEN);

        EndDrawing();
    }

    // Після виходу з циклу звільняємо пам'ять шрифту
    GlyphCache_ClearAllCaches();
    UnloadPSFFont(font12);
    UnloadPSFFont(font18);
    UnloadPSFFont(font32);

    CloseWindow();
    return 0;
}

