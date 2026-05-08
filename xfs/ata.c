#include "ata.h"

// Безпечне очікування статусу
static int ata_wait_status(unsigned char mask, unsigned char value) {
    for (int i = 0; i < 100000; i++) {
        if ((inb(0x1F7) & mask) == value) return 1;
    }
    return 0; // Тайм-аут
}

void ata_read_sector(int lba, unsigned short* buffer) {
    if (!ata_wait_status(0x80, 0)) return; // Чекаємо BSY=0

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20); // Read

    if (!ata_wait_status(0x08, 0x08)) return; // Чекаємо DRQ=1

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0);
    }
}

static void ata_flush_cache() {
    if (!ata_wait_status(0x80, 0)) return;
    outb(0x1F7, 0xE7); // Flush cache
    ata_wait_status(0x80, 0);
}

void ata_write_sector(int lba, unsigned short* buffer) {
    if (!ata_wait_status(0x80, 0)) return;

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x30); // Write

    if (!ata_wait_status(0x08, 0x08)) return;

    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]);
    }

    ata_flush_cache();
}
