#include "drivers/GPU.h"
#include "drivers/IDT.h"
#include "drivers/Keyboard.h"
#include "drivers/Mouse.h"
#include "drivers/RAM.h"
#include "drivers/CPU.h"
#include "drivers/RTC.h"
#include "../xfs/xfs.h"
#include "kernel.h"
#include "../apps/explorer.h"
#include "../apps/xinfo.h"

static int desktop_needs_redraw = 1;
static int last_mouse_btn = 0;

// Окремі обгортки для задач (майбутні "потоки")
void task_clock(BootInfo* binfo) {
    fill_rect(binfo->ScreenWidth - 85, binfo->ScreenHeight - 30, 80, 20, 0x001A1A1A);
    draw_string_scaled(rtc_time_to_string(), binfo->ScreenWidth - 80, binfo->ScreenHeight - 28, 0xFFFFFFFF, 1);
}

void task_explorer(BootInfo* binfo) {
    if (explorer_is_active()) {
        explorer_draw(binfo);
    }
}

void kernel_trigger_desktop_redraw() {
    desktop_needs_redraw = 1;
}

static void draw_explorer_icon() {
    int x = 20, y = 80, w = 80, h = 80;
    fill_rect(x, y, w, h, 0x002E3A55);
    fill_rect(x + 8, y + 12, 64, 40, 0x00FFD700);
    fill_rect(x + 8, y + 44, 46, 18, 0x00808080);
    draw_string_scaled("Explorer", x + 5, y + 64, 0x00FFFFFF, 1);
    fill_rect(x - 2, y - 2, w + 4, 2, 0x00FFFFFF);
    fill_rect(x - 2, y + h, w + 4, 2, 0x00FFFFFF);
    fill_rect(x - 2, y, 2, h, 0x00FFFFFF);
    fill_rect(x + w, y, 2, h, 0x00FFFFFF);
}

void draw_desktop(BootInfo* binfo) {
    draw_ukraine_flag();
    draw_panel();
    draw_explorer_icon();
    draw_tryzub(binfo->ScreenWidth / 2, binfo->ScreenHeight / 6, 12, 0xFFFFFFFF);
}

void desktop_on_click(int x, int y) {
    // Клік по інформації про систему (верхній лівий кут) активує xinfo
    if (x >= 15 && x < 150 && y >= 35 && y < 75) {
        xinfo_toggle();
        return;
    }

    if (xinfo_is_active()) {
        xinfo_handle_click(x, y);
        // Якщо після кліку вікно закрилося або клік був поза ним, дозволяємо обробку далі
        if (xinfo_is_active() && 
            x >= INFO_WINDOW_X && x < INFO_WINDOW_X + INFO_WINDOW_W && 
            y >= INFO_WINDOW_Y && y < INFO_WINDOW_Y + INFO_WINDOW_H) 
            return;
    }

    if (!explorer_is_active() && x >= 20 && x < 100 && y >= 80 && y < 160) {
        explorer_open_window();
        return;
    }
    if (explorer_is_active()) {
        explorer_on_click(x, y);
        if (x >= EXPLORER_X && x < EXPLORER_X + EXPLORER_W && 
            y >= EXPLORER_Y && y < EXPLORER_Y + EXPLORER_HEIGHT) {
            return;
        }
    }
}

__attribute__((section(".text.entry")))
void _start(BootInfo* binfo) {
    gpu_init(binfo);
    draw_desktop(binfo);

    cpu_init_scheduler();
    ram_init(binfo);
    xfs_init();
    xfs_load();
    explorer_init();
    xinfo_init();
    idt_init();
    keyboard_init();
    mouse_init(binfo);

    outb(0x21, 0xF9); // Master: Keyboard + Slave Cascade
    outb(0xA1, 0xEF); // Slave: Mouse
    __asm__ volatile ("sti");

    // Реєструємо процеси в менеджері
    cpu_add_task(task_clock, "System Clock", PRIORITY_DRIVER);
    cpu_add_task(xinfo_draw, "System Info", PRIORITY_APP);
    cpu_add_task(task_explorer, "File Explorer", PRIORITY_APP);

    kernel_trigger_desktop_redraw();
    xfs_print_info();
    while (1) {
        // 1. Прибираємо мишу зі старої позиції
        mouse_restore_cursor();

        // Малюємо фон ТІЛЬКИ якщо це потрібно (прибирає блимання)
        if (desktop_needs_redraw) {
            draw_desktop(binfo);
            desktop_needs_redraw = 0;
        }

        // 2. Обробляємо кліки
        int current_btn = mouse_left_pressed();
        if (current_btn && !last_mouse_btn) {
            desktop_on_click(mouse_get_x(), mouse_get_y());
        }
        last_mouse_btn = current_btn;

        // Запускаємо менеджер процесів
        cpu_run_scheduler(binfo);

        // 3. Малюємо мишу поверх всього заново
        mouse_draw_cursor();

        __asm__ volatile ("hlt");
    }
}
