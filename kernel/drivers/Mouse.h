#ifndef MOUSE_H
#define MOUSE_H

#include "../../boot/boot_info.h"

void mouse_init();
void mouse_handler();
void draw_cursor(int x, int y);

#endif
