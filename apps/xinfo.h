#ifndef XINFO_H
#define XINFO_H

#include "../boot/boot_info.h"

#define INFO_WINDOW_X 200
#define INFO_WINDOW_Y 100
#define INFO_WINDOW_W 400
#define INFO_WINDOW_H 250

typedef struct {
    int active;
    int mouse_over_close;
} XInfoWindow;

void xinfo_init(void);
void xinfo_draw(BootInfo* binfo);
void xinfo_handle_click(int x, int y);
void xinfo_toggle(void);
int xinfo_is_active(void);

#endif