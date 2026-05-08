#ifndef CPU_H
#define CPU_H

#include "../../boot/boot_info.h"

typedef enum {
    PRIORITY_DRIVER = 0, // Найвищий пріоритет (драйвери, системні служби)
    PRIORITY_APP = 1    // Звичайний пріоритет (користувацькі програми)
} TaskPriority;

typedef void (*TaskFunction)(BootInfo*);

typedef struct {
    TaskFunction func;
    TaskPriority priority;
    int active;
    const char* name;
} XOS_Task;

#define MAX_TASKS 16

const char* cpu_get_vendor();
void cpu_halt();

void cpu_init_scheduler();
int cpu_add_task(TaskFunction func, const char* name, TaskPriority priority);
void cpu_run_scheduler(BootInfo* binfo);

#endif
