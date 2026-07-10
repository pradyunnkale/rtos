#include <stdint.h>
#include <stdio.h>

#include "rtos.h"

#define TASK_STACK_SIZE 16384U

#define CONTROL_PRIORITY  0U
#define GUIDANCE_PRIORITY 1U
#define IDLE_PRIORITY     UINT8_MAX

#define CONTROL_PERIOD_MS  100U
#define GUIDANCE_PERIOD_MS 250U

static task_t control_task;
static task_t guidance_task;
static task_t idle_task;

static uint8_t control_stack[TASK_STACK_SIZE];
static uint8_t guidance_stack[TASK_STACK_SIZE];
static uint8_t idle_stack[TASK_STACK_SIZE];

static void control_entry(void *arg)
{
    (void)arg;

    rtos_tick_t next_release = rtos_time_now();

    for (;;)
    {
        printf(
            "CONTROL  at %llu ms\n",
            (unsigned long long)rtos_time_now()
        );
        fflush(stdout);

        next_release += CONTROL_PERIOD_MS;

        if (task_sleep_until(next_release) != RTOS_OK)
        {
            return;
        }
    }
}

static void guidance_entry(void *arg)
{
    (void)arg;

    rtos_tick_t next_release = rtos_time_now();

    for (;;)
    {
        printf(
            "GUIDANCE at %llu ms\n",
            (unsigned long long)rtos_time_now()
        );
        fflush(stdout);

        next_release += GUIDANCE_PERIOD_MS;

        if (task_sleep_until(next_release) != RTOS_OK)
        {
            return;
        }
    }
}

static void idle_entry(void *arg)
{
    (void)arg;

    /*
     * The host timer asynchronously wakes sleeping tasks and requests
     * preemption. The idle task therefore does not manage wakeups.
     */
    for (;;)
    {
        /*
         * This can busy-spin for the initial host implementation.
         * Later, the host port can sleep until the next timer event.
         */
    }
}

int main(void)
{
    rtos_status_t status;

    status = rtos_init();
    if (status != RTOS_OK)
    {
        fprintf(stderr, "rtos_init failed: %d\n", status);
        return 1;
    }

    status = task_create(
        &control_task,
        control_entry,
        NULL,
        CONTROL_PRIORITY,
        control_stack,
        sizeof(control_stack)
    );

    if (status != RTOS_OK)
    {
        fprintf(stderr, "control task creation failed: %d\n", status);
        return 1;
    }

    status = task_create(
        &guidance_task,
        guidance_entry,
        NULL,
        GUIDANCE_PRIORITY,
        guidance_stack,
        sizeof(guidance_stack)
    );

    if (status != RTOS_OK)
    {
        fprintf(stderr, "guidance task creation failed: %d\n", status);
        return 1;
    }

    status = task_create(
        &idle_task,
        idle_entry,
        NULL,
        IDLE_PRIORITY,
        idle_stack,
        sizeof(idle_stack)
    );

    if (status != RTOS_OK)
    {
        fprintf(stderr, "idle task creation failed: %d\n", status);
        return 1;
    }

    status = rtos_start();

    fprintf(
        stderr,
        "rtos_start returned unexpectedly: %d\n",
        status
    );

    return 1;
}
