#include "CPU.h"

static char vendor[13]; // 12 символів + 1 на закриваючий нуль

const char* cpu_get_vendor() {
    unsigned int eax, ebx, ecx, edx;
    eax = 0;
    __asm__ volatile ("cpuid" 
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
        : "a"(eax));

    // Копіюємо байти в правильному порядку (ebx -> edx -> ecx)
    *((unsigned int*)&vendor[0]) = ebx;
    *((unsigned int*)&vendor[4]) = edx;
    *((unsigned int*)&vendor[8]) = ecx;
    vendor[12] = '\0';
    
    return vendor;
}

void cpu_halt() {
    while(1) { __asm__ volatile ("hlt"); }
}
