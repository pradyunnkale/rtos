#include <stdint.h>
#include <stdio.h>

#include "rtos.h"
#include "rtos_types.h"

#define TASK_STACK_SIZE 16384U

#define CONTROL_PRIORITY  0U
#define GUIDANCE_PRIORITY 1U
#define CONTROL_PERIOD  RTOS_MS(100U)
#define GUIDANCE_PERIOD RTOS_MS(250U)

static task_t control_task;
static task_t guidance_task;
static rtos_time_t test_start;

static uint8_t control_stack[TASK_STACK_SIZE];
static uint8_t guidance_stack[TASK_STACK_SIZE];

static void control_entry(void *arg)
{
    (void)arg;

    rtos_time_t next_release = rtos_time_now();

    for (;;)
    {
        rtos_time_t now = rtos_time_now();

        printf(
            "CONTROL  at %llu ms\n",
            (unsigned long long)RTOS_TIME_TO_MS(now - test_start)
        );
        fflush(stdout);

        next_release += CONTROL_PERIOD;

        if (task_sleep_until(next_release) != RTOS_OK)
        {
            return;
        }
    }
}

static void guidance_entry(void *arg)
{
    (void)arg;

    rtos_time_t next_release = rtos_time_now();

    for (;;)
    {
        rtos_time_t now = rtos_time_now();

        printf(
            "GUIDANCE at %llu ms\n",
            (unsigned long long)RTOS_TIME_TO_MS(now - test_start)
        );
        fflush(stdout);

        next_release += GUIDANCE_PERIOD;

        if (task_sleep_until(next_release) != RTOS_OK)
        {
            return;
        }
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

    test_start = rtos_time_now();

    status = rtos_start();

    fprintf(
        stderr,
        "rtos_start returned unexpectedly: %d\n",
        status
    );

    return 1;
}
