#include "xinfo.h"
#include "../kernel/drivers/GPU.h"
#include "../kernel/drivers/CPU.h"
#include "../kernel/drivers/RAM.h"
#include "../kernel/drivers/RTC.h"
#include "../kernel/kernel.h"

static XInfoWindow win;

void xinfo_init(void) {
    win.active = 0;
    win.mouse_over_close = 0;
}

void xinfo_draw(BootInfo* binfo) {
    if (!win.active) return;
    
    // Тіло вікна
    fill_rect(INFO_WINDOW_X, INFO_WINDOW_Y, INFO_WINDOW_W, INFO_WINDOW_H, 0x001A1A1A);
    // Заголовок
    fill_rect(INFO_WINDOW_X, INFO_WINDOW_Y, INFO_WINDOW_W, 25, 0x002E3A55);
    // Рамка
    fill_rect(INFO_WINDOW_X, INFO_WINDOW_Y, INFO_WINDOW_W, 1, 0x00FFFFFF);
    fill_rect(INFO_WINDOW_X, INFO_WINDOW_Y + INFO_WINDOW_H - 1, INFO_WINDOW_W, 1, 0x00FFFFFF);
    fill_rect(INFO_WINDOW_X, INFO_WINDOW_Y, 1, INFO_WINDOW_H, 0x00FFFFFF);
    fill_rect(INFO_WINDOW_X + INFO_WINDOW_W - 1, INFO_WINDOW_Y, 1, INFO_WINDOW_H, 0x00FFFFFF);

    draw_string_scaled("System Information", INFO_WINDOW_X + 10, INFO_WINDOW_Y + 5, 0x00FFFFFF, 1);

    // Кнопка закриття
    fill_rect(INFO_WINDOW_X + INFO_WINDOW_W - 25, INFO_WINDOW_Y, 25, 25, 0x00AA0000);
    draw_string_scaled("X", INFO_WINDOW_X + INFO_WINDOW_W - 17, INFO_WINDOW_Y + 5, 0x00FFFFFF, 1);
    
    // Вивід характеристик
    draw_string_scaled("Processor:", INFO_WINDOW_X + 20, INFO_WINDOW_Y + 50, 0x00FFFFFF, 1);
    draw_string_scaled(cpu_get_vendor(), INFO_WINDOW_X + 110, INFO_WINDOW_Y + 50, 0x00FFD700, 1);

    draw_string_scaled("Total RAM:", INFO_WINDOW_X + 20, INFO_WINDOW_Y + 80, 0x00FFFFFF, 1);
    draw_string_scaled(ram_to_string(get_actual_ram()), INFO_WINDOW_X + 110, INFO_WINDOW_Y + 80, 0x00FFD700, 1);
    draw_string_scaled("MB", INFO_WINDOW_X + 150, INFO_WINDOW_Y + 80, 0x00FFFFFF, 1);

    draw_string_scaled("XOS Version: 4.0 New (Kernel x64)", INFO_WINDOW_X + 20, INFO_WINDOW_Y + 120, 0x00808080, 1);
}

void xinfo_toggle(void) {
    win.active = !win.active;
    kernel_trigger_desktop_redraw();
}

int xinfo_is_active(void) {
    return win.active;
}

void xinfo_handle_click(int x, int y) {
    if (!win.active) return;

    // Якщо клік поза межами вікна, нічого не робимо
    if (x < INFO_WINDOW_X || x > INFO_WINDOW_X + INFO_WINDOW_W ||
        y < INFO_WINDOW_Y || y > INFO_WINDOW_Y + INFO_WINDOW_H) {
        return;
    }

    // Перевірка натискання на кнопку закриття
    if (x >= INFO_WINDOW_X + INFO_WINDOW_W - 25 && x <= INFO_WINDOW_X + INFO_WINDOW_W &&
        y >= INFO_WINDOW_Y && y <= INFO_WINDOW_Y + 25) {
        win.active = 0;
        kernel_trigger_desktop_redraw();
    }
}