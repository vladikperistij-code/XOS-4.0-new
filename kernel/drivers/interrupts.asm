[bits 64]
extern keyboard_handler
extern pic_eoi             ; Додамо функцію в Сі для скидання PIC
global keyboard_asm_handler
global common_stub_handler

keyboard_asm_handler:
    push rbp
    mov rbp, rsp
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    call keyboard_handler

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    pop rbp
    iretq

common_stub_handler:
    push rax
    mov al, 0x20
    out 0x20, al        ; Посилаємо EOI в Master PIC
    out 0xA0, al        ; Посилаємо EOI в Slave PIC
    pop rax
    iretq
