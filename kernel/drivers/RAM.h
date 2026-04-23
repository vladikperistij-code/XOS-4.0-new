#ifndef RAM_H
#define RAM_H

#include "../../boot/boot_info.h"

void ram_init(BootInfo* binfo);
unsigned long long get_actual_ram();
const char* ram_to_string(unsigned long long value);

#endif
