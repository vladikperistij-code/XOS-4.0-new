#include "RAM.h"

// Структура дескриптора UEFI (x86_64 стандарт)
typedef struct {
    unsigned int type;
    unsigned int pad;
    unsigned long long physical_start;
    unsigned long long virtual_start;
    unsigned long long number_of_pages;
    unsigned long long attribute;
} EFI_MEMORY_DESCRIPTOR;

static unsigned long long total_mem_mb = 0;

void ram_init(BootInfo* binfo) {
    total_mem_mb = 0;
    
    unsigned char* map_addr = (unsigned char*)binfo->MemoryMap;
    unsigned long long map_size = binfo->MemoryMapSize;
    
    // Крок дескриптора (48 байт - стандарт gnu-efi)
    unsigned long long descriptor_size = 48; 
    
    for (unsigned long long i = 0; i < map_size; i += descriptor_size) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)(map_addr + i);
        
        // Тип 7 — це вільна пам'ять (EfiConventionalMemory)
        if (desc->type == 7) {
            total_mem_mb += (desc->number_of_pages * 4096);
        }
    }
    
    total_mem_mb /= (1024 * 1024); // Конвертуємо в мегабайти
}

unsigned long long get_actual_ram() {
    return total_mem_mb;
}

// ОСЬ ЦЯ ФУНКЦІЯ, ЯКОЇ НЕ ВИСТАЧАЛО:
const char* ram_to_string(unsigned long long value) {
    static char buf[32]; // Буфер для тексту
    
    // Очищуємо буфер
    for(int i = 0; i < 32; i++) buf[i] = 0;

    int i = 30;
    if (value == 0) {
        buf[i] = '0';
        return &buf[i];
    }

    while (value > 0 && i > 0) {
        buf[--i] = (value % 10) + '0';
        value /= 10;
    }
    
    return &buf[i];
}
