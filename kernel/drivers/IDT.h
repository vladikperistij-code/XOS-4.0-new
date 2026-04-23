#ifndef IDT_H
#define IDT_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  types_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) IDTEntry;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IDTPtr;

extern IDTEntry idt[256];
extern IDTPtr idtp;

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = (uint16_t)(base & 0xFFFF);
    idt[num].offset_mid = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].offset_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].types_attr = flags;
    idt[num].zero = 0;
}

void idt_init();
void pic_remap(); // Додано прототип

#endif
