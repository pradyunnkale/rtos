#include <stdint.h>
#include <stdio.h>

#include "rtos.h"

#define TASK_STACK_SIZE 16384U

#define CONTROL_PRIORITY 0U
#define LOGGING_PRIORITY 3U

#define CONTROL_PERIOD_MS 100U
#define LOGGING_PERIOD_US 50000ULL

static task_t control_task;
static task_t logging_task;

static uint8_t control_stack[TASK_STACK_SIZE];
static uint8_t logging_stack[TASK_STACK_SIZE];

static void control_entry(void *arg)
{
    (void)arg;

    rtos_tick_t next_release_ms = rtos_time_now();
    rtos_time_us_t next_release_us = rtos_time_now_us();

    for (;;)
    {
        rtos_time_us_t now_us = rtos_time_now_us();

        int64_t jitter_us =
            (int64_t)now_us - (int64_t)next_release_us;

        printf(
            "CONTROL at %llu us, jitter %+lld us\n",
            (unsigned long long)now_us,
            (long long)jitter_us
        );
        fflush(stdout);

        next_release_ms += CONTROL_PERIOD_MS;
        next_release_us +=
            (rtos_time_us_t)CONTROL_PERIOD_MS * 1000ULL;

        if (task_sleep_until(next_release_ms) != RTOS_OK)
        {
            fprintf(stderr, "control task failed to sleep\n");
            return;
        }
    }
}

static void logging_entry(void *arg)
{
    (void)arg;

    rtos_time_us_t next_print_us = rtos_time_now_us();

    for (;;)
    {
        rtos_time_us_t now_us = rtos_time_now_us();

        if (now_us >= next_print_us)
        {
            printf(
                "LOGGING at %llu us\n",
                (unsigned long long)now_us
            );
            fflush(stdout);

            /*
             * Advance from the previous deadline to avoid drift,
             * while skipping any missed print periods.
             */
            do
            {
                next_print_us += LOGGING_PERIOD_US;
            }
            while (next_print_us <= now_us);
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

    status = rtos_start();

    fprintf(
        stderr,
        "rtos_start returned unexpectedly: %d\n",
        status
    );

    return 1;
}
