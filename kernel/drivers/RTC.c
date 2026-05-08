#include "RTC.h"
#include "IDT.h" // Використовуємо уніфіковані outb/inb

void rtc_get_time(int* hour, int* minute, int* second) {
    outb(0x70, 0x00); unsigned char s = inb(0x71);
    outb(0x70, 0x02); unsigned char m = inb(0x71);
    outb(0x70, 0x04); unsigned char h = inb(0x71);

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
