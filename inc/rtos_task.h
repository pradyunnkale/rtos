#pragma once

#include <stddef.h>
#include <stdint.h>

#include "rtos_types.h"
#include "port_types.h"

typedef enum
{
    TASK_READY,
    TASK_SLEEP,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_CREATED,
    TASK_TERMINATED
} task_state_t;

typedef struct task
{
    uint8_t pid;
    uint8_t priority;
    task_state_t state;

    task_entry_t entry;
    void *arg;

    uint8_t *stack_base;
    uint8_t *stack_ptr;
    size_t stack_size;

    struct task *next;

    rtos_tick_t wake_time;
    port_context_t context;
} task_t;

rtos_status_t task_create(task_t *task, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack, size_t stack_size);

void task_yield(void);

rtos_status_t task_sleep_until(rtos_tick_t wake_time);
