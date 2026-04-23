#include "Keyboard.h"
#include "IDT.h"
#include "GPU.h"

extern void keyboard_asm_handler(); // Наш місток з interrupts.asm

// Мапа скан-кодів (QWERTY)
static char keymap[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void keyboard_handler() {
    uint8_t scancode;
    // Читаємо скан-код з порту
    __asm__ volatile ("inb $0x60, %0" : "=a"(scancode));

    // Якщо натиснуто клавішу (скан-коди < 58 з нашої мапи)
    if (scancode < 58) {
        char key = keymap[scancode];
        if (key != 0) {
            // Очищуємо місце під літеру (малюємо прямокутник кольору фону)
            fill_rect(15, 150, 40, 40, 0x000057B7); 
            
            // Створюємо рядок з одного символу
            char str[2] = {key, '\0'};
            // Малюємо літеру великим шрифтом
            draw_string_scaled(str, 15, 150, COLOR_WHITE, 3);
        }
    }

    // Сигнал закінчення переривання (EOI) для контролера переривань
    outb(0x20, 0x20);
}

void keyboard_init() {
    // Замість uintptr_t використовуємо uint64_t, який є в IDT.h
    idt_set_gate(33, (uint64_t)keyboard_asm_handler, 0x38, 0x8E);
}
