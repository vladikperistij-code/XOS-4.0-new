#ifndef MOUSE_H
#define MOUSE_H

#include "../../boot/boot_info.h"

void mouse_init(BootInfo* binfo);
void mouse_handler();

int mouse_get_x();
int mouse_get_y();
int mouse_left_pressed();

void mouse_restore_cursor();
void mouse_draw_cursor();

#endif
