#include "Mouse.h"
#include "IDT.h"
#include "GPU.h"

typedef signed char s8;
typedef unsigned char u8;

static BootInfo* g_m_binfo;
int mouse_x = 400, mouse_y = 300;
u8 mouse_cycle = 0;
s8 mouse_byte[3];           // Масив на 3 байти
unsigned int m_back[144];   // Буфер для фону 12x12 (144 пікселі)

// --- 1. Функції збереження та відновлення фону (МАЮТЬ БУТИ ВГОРІ) ---

void m_save(int x, int y) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            m_back[i * 12 + j] = get_pixel(x + j, y + i);
        }
    }
}

void m_restore(int x, int y) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            put_pixel(x + j, y + i, m_back[i * 12 + j]);
        }
    }
}

// --- 2. Малювання стрілки ---

void draw_mouse_ptr(int x, int y, unsigned int color) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < i && j < 8; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

// --- 3. Робота з залізом ---

void mouse_wait(u8 type) {
    unsigned int t = 100000;
    if (type == 0) {
        while (t--) if ((inb(0x64) & 1)) return;
    } else {
        while (t--) if (!(inb(0x64) & 2)) return;
    }
}

// --- 4. ОБРОБНИК ПЕРЕРИВАНЬ (ТІЛЬКИ ОДИН!) ---

void mouse_handler() {
    u8 status = inb(0x64);
    if ((status & 0x01) && (status & 0x20)) {
        u8 data = inb(0x60);
        
        // Синхронізація
        if (mouse_cycle == 0 && !(data & 0x08)) {
            outb(0xA0, 0x20); outb(0x20, 0x20); return;
        }

        mouse_byte[mouse_cycle++] = (s8)data;

        if (mouse_cycle == 3) {
            mouse_cycle = 0;

            // 1. Повертаємо фон
            m_restore(mouse_x, mouse_y);

            // 2. Оновлюємо координати (X - 2-й байт, Y - 3-й байт)
            mouse_x += (int)mouse_byte[1];
            mouse_y -= (int)mouse_byte[2];

            // Межі екрана
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > g_m_binfo->ScreenWidth - 12) mouse_x = g_m_binfo->ScreenWidth - 12;
            if (mouse_y > g_m_binfo->ScreenHeight - 12) mouse_y = g_m_binfo->ScreenHeight - 12;

            // 3. Зберігаємо новий фон і малюємо стрілку
            m_save(mouse_x, mouse_y);
            draw_mouse_ptr(mouse_x, mouse_y, 0xFFFFFFFF);
        }
    }
    outb(0xA0, 0x20); outb(0x20, 0x20);
}

// --- 5. Ініціалізація ---

extern void mouse_asm_handler();

void mouse_init(BootInfo* binfo) {
    g_m_binfo = binfo;

    mouse_wait(1); outb(0x64, 0xA8); // Enable
    mouse_wait(1); outb(0x64, 0x20); // Get ComByte
    mouse_wait(0); u8 s = (inb(0x60) | 2);
    mouse_wait(1); outb(0x64, 0x60); // Set ComByte
    mouse_wait(1); outb(0x60, s);
    
    mouse_wait(1); outb(0x64, 0xD4);
    mouse_wait(1); outb(0x60, 0xF4); // Enable Data Reporting
    if (inb(0x64) & 1) inb(0x60);    // ACK

    idt_set_gate(44, (uint64_t)mouse_asm_handler, 0x38, 0x8E);

    m_save(mouse_x, mouse_y);
    draw_mouse_ptr(mouse_x, mouse_y, 0xFFFFFFFF);
}
