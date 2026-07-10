#pragma once
#include "rtos_task.h"

void task_system_init(void);
rtos_status_t task_init(task_t* task, uint8_t pid, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack_base, size_t stack_size);
void task_set_state(task_t *task, task_state_t state);
