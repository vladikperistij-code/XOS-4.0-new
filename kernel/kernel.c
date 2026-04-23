#include "drivers/GPU.h"
#include "drivers/IDT.h"
#include "drivers/Keyboard.h"
#include "drivers/Mouse.h"
#include "drivers/RAM.h"
#include "drivers/CPU.h"
#include "drivers/RTC.h"

__attribute__((section(".text.entry")))
void _start(BootInfo* binfo) {
    gpu_init(binfo);
    draw_ukraine_flag();
    draw_panel();
    draw_tryzub(binfo->ScreenWidth / 2, binfo->ScreenHeight / 6, 12, 0xFFFFFFFF);

    ram_init(binfo);
    idt_init();
    keyboard_init();
    mouse_init(binfo);

    outb(0x21, 0xF9); // Master: Keyboard + Slave Cascade
    outb(0xA1, 0xEF); // Slave: Mouse
    __asm__ volatile ("sti");
    draw_string_scaled("XOS 4.0 NEW - UKRAINE", 15, 15, 0xFFFFFFFF, 1);
    
    draw_string_scaled("CPU:", 15, 35, 0xFFFFFFFF, 1);
    draw_string_scaled(cpu_get_vendor(), 55, 35, 0x00FFD700, 1);

    draw_string_scaled("RAM:", 15, 55, 0xFFFFFFFF, 1);
    draw_string_scaled(ram_to_string(get_actual_ram()), 55, 55, 0x00FFD700, 1);
    draw_string_scaled("MB", 95, 55, 0xFFFFFFFF, 1);
    while (1) {
        fill_rect(binfo->ScreenWidth - 85, binfo->ScreenHeight - 30, 80, 20, 0x001A1A1A);
        draw_string_scaled(rtc_time_to_string(), binfo->ScreenWidth - 80, binfo->ScreenHeight - 28, 0xFFFFFFFF, 1);
        for (volatile int i = 0; i < 2000000; i++);
        __asm__ volatile ("hlt");
    }
}
