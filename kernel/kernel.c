#include "drivers/GPU.h"
#include "drivers/IDT.h"
#include "drivers/Keyboard.h"
#include "drivers/RTC.h"

void _start(BootInfo* binfo) {
    gpu_init(binfo);
    idt_init();
    keyboard_init();
    
    // Вмикаємо переривання на рівні процесора
    __asm__ volatile ("sti");

    draw_ukraine_flag();
    draw_panel();
    draw_tryzub(binfo->ScreenWidth / 2, binfo->ScreenHeight / 6, 12, COLOR_WHITE);

    draw_string_scaled("XOS 4.0 new - UKRAINE", 10, 10, COLOR_WHITE, 1);

    while (1) {
        // Оновлюємо годинник
        const char* time = rtc_time_to_string();
        fill_rect(binfo->ScreenWidth - 80, binfo->ScreenHeight - 30, 75, 20, 0x001A1A1A);
        draw_string_scaled(time, binfo->ScreenWidth - 75, binfo->ScreenHeight - 28, COLOR_WHITE, 1);
        
        // Пауза для стабільності
        for(volatile unsigned long long i = 0; i < 2000000; i++);
    }
}
