#ifndef EXPLORER_H
#define EXPLORER_H

#include "../xfs/xfs.h"
#include "../boot/boot_info.h"

#define EXPLORER_X 120
#define EXPLORER_Y 100
#define EXPLORER_W 420
#define EXPLORER_HEIGHT 260

void explorer_init();
void explorer_open_window();
int explorer_is_active();
void explorer_draw(BootInfo* binfo);
void explorer_on_click(int x, int y);
void explorer_on_key(char key);

#endif