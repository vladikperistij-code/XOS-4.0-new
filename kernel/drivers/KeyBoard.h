#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../boot/boot_info.h"

// Оголошуємо функції, щоб ядро їх бачило
void keyboard_init();
void keyboard_handler();

#endif
