#include <stdint.h>
#include <stdio.h>

#include "rtos.h"

#define TASK_STACK_SIZE 16384U

#define CONTROL_PRIORITY 0U
#define LOGGING_PRIORITY 3U

#define CONTROL_PERIOD RTOS_MS(100U)
#define LOGGING_PERIOD RTOS_US(50000ULL)

static task_t control_task;
static task_t logging_task;

static rtos_time_t test_start;

static uint8_t control_stack[TASK_STACK_SIZE];
static uint8_t logging_stack[TASK_STACK_SIZE];

static void control_entry(void *arg)
{
    (void)arg;

    rtos_time_t next_release = rtos_time_now();

    for (;;)
    {
        rtos_time_t now = rtos_time_now();

        int64_t jitter_ns = (int64_t)now - (int64_t)next_release;

        printf(
            "CONTROL at %llu us, jitter %+lld us\n",
            (unsigned long long)RTOS_TIME_TO_US(now - test_start),
            (long long)(jitter_ns / (int64_t)RTOS_NS_PER_US)
        );
        fflush(stdout);

        next_release += CONTROL_PERIOD;

        if (task_sleep_until(next_release) != RTOS_OK)
        {
            fprintf(stderr, "control task failed to sleep\n");
            return;
        }
    }
}

static void logging_entry(void *arg)
{
    (void)arg;

    rtos_time_t next_print = rtos_time_now();

    for (;;)
    {
        rtos_time_t now = rtos_time_now();

        if (now >= next_print)
        {
            printf(
                "LOGGING at %llu us\n",
                (unsigned long long)RTOS_TIME_TO_US(now - test_start)
            );
            fflush(stdout);

            /*
             * Advance from the previous deadline to avoid drift,
             * while skipping any missed print periods.
             */
            do
            {
                next_print += LOGGING_PERIOD;
            }
            while (next_print <= now);
        }

        /*
         * Intentionally no yield or sleep.
         * Control must asynchronously preempt this task.
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
        &logging_task,
        logging_entry,
        NULL,
        LOGGING_PRIORITY,
        logging_stack,
        sizeof(logging_stack)
    );

    if (status != RTOS_OK)
    {
        fprintf(stderr, "logging task creation failed: %d\n", status);
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
