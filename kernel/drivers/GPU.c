#include "GPU.h"
#include "font.h"

static BootInfo* g_binfo;

void gpu_init(BootInfo* binfo) {
    g_binfo = binfo;
}

void put_pixel(unsigned int x, unsigned int y, unsigned int color) {
    if (x >= g_binfo->ScreenWidth || y >= g_binfo->ScreenHeight) return;
    g_binfo->FramebufferBase[y * g_binfo->PixelsPerScanLine + x] = color;
}

unsigned int get_pixel(unsigned int x, unsigned int y) {
    if (x >= g_binfo->ScreenWidth || y >= g_binfo->ScreenHeight) return 0;
    return g_binfo->FramebufferBase[y * g_binfo->PixelsPerScanLine + x];
}

void fill_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color) {
    for (unsigned int cy = y; cy < y + h; cy++) {
        for (unsigned int cx = x; cx < x + w; cx++) {
            put_pixel(cx, cy, color);
        }
    }
}

void draw_ukraine_flag() {
    fill_rect(0, 0, g_binfo->ScreenWidth, g_binfo->ScreenHeight / 2, COLOR_BLUE);
    fill_rect(0, g_binfo->ScreenHeight / 2, g_binfo->ScreenWidth, g_binfo->ScreenHeight / 2, COLOR_YELLOW);
}

void draw_tryzub(unsigned int x, unsigned int y, unsigned int scale, unsigned int color) {
    fill_rect(x, y, scale, scale * 6, color);
    fill_rect(x - scale * 3, y + scale * 2, scale, scale * 3, color);
    fill_rect(x + scale * 3, y + scale * 2, scale, scale * 3, color);
    fill_rect(x, y + scale * 6, scale, scale, color);
}

void draw_char_scaled(unsigned char c, unsigned int x, unsigned int y, unsigned int color, unsigned int scale) {
    if (c >= 128) c = '?';
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_8x16[c][row] >> (7 - col)) & 1) {
                fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void draw_string_scaled(const char* str, unsigned int x, unsigned int y, unsigned int color, unsigned int scale) {
    while (*str) {
        draw_char_scaled((unsigned char)*str, x, y, color, scale);
        x += (8 + 1) * scale;
        str++;
    }
}

void draw_panel() {
    unsigned int h = g_binfo->ScreenHeight;
    fill_rect(0, h - 40, g_binfo->ScreenWidth, 40, 0x001A1A1A);
    fill_rect(5, h - 35, 80, 30, COLOR_BLUE);
    draw_string_scaled("START", 15, h - 28, COLOR_WHITE, 1);
}
