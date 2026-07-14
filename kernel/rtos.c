#include "rtos_kernel.h"

#include <stdint.h>

#include "port.h"
#include "task_internal.h"
#include "rtos_sched.h"
#include "time_internal.h"

#ifndef RTOS_IDLE_STACK_SIZE
#define RTOS_IDLE_STACK_SIZE 16384U
#endif

static task_t idle_task;
static uint8_t idle_stack[RTOS_IDLE_STACK_SIZE];

static void idle_entry(void *arg)
{
    (void)arg;

    for (;;)
    {
        port_idle_wait();
    }
}

rtos_status_t rtos_init(void)
{
    rtos_status_t status;

    task_system_init();

    status = sched_init();
    if (status != RTOS_OK)
    {
        return status;
    }

    return task_create(
        &idle_task,
        idle_entry,
        NULL,
        UINT8_MAX,
        idle_stack,
        sizeof(idle_stack));
}

rtos_status_t rtos_start(void)
{
    return sched_start();
}
