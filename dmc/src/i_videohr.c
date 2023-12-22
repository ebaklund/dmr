//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     SDL emulation of VGA 640x480x4 planar video mode
//

#include "SDL.h"
#include "string.h"

#include "doomtype.h"
#include "i_timer.h"
#include "i_video.h"

// Palette fade-in takes two seconds

#define FADE_TIME 2000

#define HR_SCREENWIDTH 640
#define HR_SCREENHEIGHT 480

static SDL_Window *hr_screen = NULL;
static SDL_Surface *hr_surface = NULL;
static const char *window_title = "";

void I_SlamBlockHR(int x, int y, int w, int h, const byte *src)
{
    SDL_Rect blit_rect;
    const byte *srcptrs[4];
    byte srcbits[4];
    byte *dest;
    int x1, y1;
    int i;
    int bit;

    // Set up source pointers to read from source buffer - each 4-bit
    // pixel has its bits split into four sub-buffers

    for (i=0; i<4; ++i)
    {
        srcptrs[i] = src + (i * w * h / 8);
    }

    if (SDL_LockSurface(hr_surface) < 0)
    {
        return;
    }

    // Draw each pixel

    bit = 0;

    for (y1=y; y1<y+h; ++y1)
    {
        dest = ((byte *) hr_surface->pixels) + y1 * hr_surface->pitch + x;

        for (x1=x; x1<x+w; ++x1)
        {
            // Get the bits for this pixel
            // For each bit, find the byte containing it, shift down
            // and mask out the specific bit wanted.

            for (i=0; i<4; ++i)
            {
                srcbits[i] = (srcptrs[i][bit / 8] >> (7 - (bit % 8))) & 0x1;
            }

            // Reassemble the pixel value

            *dest = (srcbits[0] << 0)
                  | (srcbits[1] << 1)
                  | (srcbits[2] << 2)
                  | (srcbits[3] << 3);

            // Next pixel!

            ++dest;
            ++bit;
        }
    }

    SDL_UnlockSurface(hr_surface);

    // Update the region we drew.
    blit_rect.x = x;
    blit_rect.y = y;
    blit_rect.w = w;
    blit_rect.h = h;
    SDL_BlitSurface(hr_surface, &blit_rect,
                    SDL_GetWindowSurface(hr_screen), &blit_rect);
    SDL_UpdateWindowSurfaceRects(hr_screen, &blit_rect, 1);
}

void I_SetPaletteHR(const byte *palette)
{
    SDL_Rect screen_rect = {0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT};
    SDL_Color sdlpal[16];
    int i;

    for (i=0; i<16; ++i)
    {
        sdlpal[i].r = palette[i * 3 + 0] * 4;
        sdlpal[i].g = palette[i * 3 + 1] * 4;
        sdlpal[i].b = palette[i * 3 + 2] * 4;
    }

    // After setting colors, update the screen.
    SDL_SetPaletteColors(hr_surface->format->palette, sdlpal, 0, 16);
    SDL_BlitSurface(hr_surface, &screen_rect,
                    SDL_GetWindowSurface(hr_screen), &screen_rect);
    SDL_UpdateWindowSurfaceRects(hr_screen, &screen_rect, 1);
}
