#include "RTC.h"

// Функція для читання байта з порту (низькорівнева)
unsigned char outb_read(unsigned char reg) {
    __asm__ volatile ("outb %0, $0x70" : : "a"(reg));
    unsigned char val;
    __asm__ volatile ("inb $0x71, %0" : "=a"(val));
    return val;
}

void rtc_get_time(int* hour, int* minute, int* second) {
    // Читаємо секунди, хвилини та години (в форматі BCD)
    unsigned char s = outb_read(0x00);
    unsigned char m = outb_read(0x02);
    unsigned char h = outb_read(0x04);

    // Перетворюємо BCD в звичайні числа
    *second = (s & 0x0F) + ((s / 16) * 10);
    *minute = (m & 0x0F) + ((m / 16) * 10);
    *hour   = (h & 0x0F) + ((h / 16) * 10) + 3; // +3 для України (UTC+3)
}

const char* rtc_time_to_string() {
    static char time_buf[9]; // "HH:MM:SS\0"
    int h, m, s;
    rtc_get_time(&h, &m, &s);

    time_buf[0] = (h / 10) + '0';
    time_buf[1] = (h % 10) + '0';
    time_buf[2] = ':';
    time_buf[3] = (m / 10) + '0';
    time_buf[4] = (m % 10) + '0';
    time_buf[5] = ':';
    time_buf[6] = (s / 10) + '0';
    time_buf[7] = (s % 10) + '0';
    time_buf[8] = '\0';

    return time_buf;
}
