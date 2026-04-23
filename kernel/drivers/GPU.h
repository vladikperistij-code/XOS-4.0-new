#ifndef GPU_H
#define GPU_H

#include "../../boot/boot_info.h"

#define COLOR_BLUE   0x000057B7
#define COLOR_YELLOW 0x00FFD700
#define COLOR_WHITE  0x00FFFFFF

void gpu_init(BootInfo* binfo);
void put_pixel(unsigned int x, unsigned int y, unsigned int color);
unsigned int get_pixel(unsigned int x, unsigned int y);
void fill_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);
void draw_ukraine_flag();
void draw_tryzub(unsigned int x, unsigned int y, unsigned int scale, unsigned int color);
void draw_char_scaled(unsigned char c, unsigned int x, unsigned int y, unsigned int color, unsigned int scale);
void draw_string_scaled(const char* str, unsigned int x, unsigned int y, unsigned int color, unsigned int scale);
void draw_panel();

#endif
