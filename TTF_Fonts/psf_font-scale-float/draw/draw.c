#include "draw.h"
#include "psf_font.h"
#include "GlyphCache.h"

// PSF_Font font12;
// PSF_Font font18;
// PSF_Font font32;

extern PSF_Font font12;
extern PSF_Font font18;
extern PSF_Font font32;

void drawTxt(void)
{
    // font12 = LoadPSFFont("fonts/Uni3-Terminus12x6.psf");
    // font18 = LoadPSFFont("fonts/Uni3-TerminusBold18x10.psf");
    // font32 = LoadPSFFont("fonts/Uni3-TerminusBold32x16.psf");

    DrawPSFText(font12, 40, 20, "Привіт Світ! Terminus12x6", 1, 3.0f, SKYBLUE);
    DrawPSFText(font18, 40, 100, "Привіт Світ! TerminusBold18", 1, 2.0f, YELLOW);
    DrawPSFText(font32, 40, 140, "Привіт Світ! TerminusBold32", 1, 1.25f, GREEN);
}

