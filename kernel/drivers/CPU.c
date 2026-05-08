#include "CPU.h"

static char vendor[13];
static XOS_Task task_list[MAX_TASKS];
static int task_count = 0;

const char* cpu_get_vendor() {
    unsigned int eax, ebx, ecx, edx;
    eax = 0;
    __asm__ volatile ("cpuid" 
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
        : "a"(eax));

    *((unsigned int*)&vendor[0]) = ebx;
    *((unsigned int*)&vendor[4]) = edx;
    *((unsigned int*)&vendor[8]) = ecx;
    vendor[12] = '\0';
    
    return vendor;
}

void cpu_halt() {
    while(1) { __asm__ volatile ("hlt"); }
}

void cpu_init_scheduler() {
    task_count = 0;
    for(int i = 0; i < MAX_TASKS; i++) {
        task_list[i].active = 0;
    }
}

int cpu_add_task(TaskFunction func, const char* name, TaskPriority priority) {
    if (task_count >= MAX_TASKS) return -1;
    
    task_list[task_count].func = func;
    task_list[task_count].name = name;
    task_list[task_count].priority = priority;
    task_list[task_count].active = 1;
    
    return task_count++;
}

void cpu_run_scheduler(BootInfo* binfo) {
    // 1. Спочатку виконуємо драйвери (Windows-like priority)
    for (int i = 0; i < task_count; i++) {
        if (task_list[i].active && task_list[i].priority == PRIORITY_DRIVER) {
            if (task_list[i].func) {
                task_list[i].func(binfo);
            }
        }
    }

    // 2. Потім виконуємо прикладні програми
    for (int i = 0; i < task_count; i++) {
        if (task_list[i].active && task_list[i].priority == PRIORITY_APP) {
            if (task_list[i].func) {
                task_list[i].func(binfo);
            }
        }
    }
}
