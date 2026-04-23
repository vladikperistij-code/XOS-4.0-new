#include "IDT.h"

IDTEntry idt[256];
IDTPtr idtp;

extern void common_stub_handler();
extern void keyboard_asm_handler();

void pic_remap() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // Дозволяємо тільки IRQ1 (клавіатура)
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}

void idt_init() {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint64_t)common_stub_handler, 0x38, 0x8E);
    }

    pic_remap();
    
    idt_set_gate(33, (uint64_t)keyboard_asm_handler, 0x38, 0x8E);

    idtp.limit = (uint16_t)(sizeof(IDTEntry) * 256) - 1;
    idtp.base = (uint64_t)&idt;
    
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}
