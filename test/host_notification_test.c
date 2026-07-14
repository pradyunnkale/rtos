#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "rtos.h"

#define TASK_STACK_SIZE 16384U

static task_t waiter_task;
static task_t starter_task;
static uint8_t waiter_stack[TASK_STACK_SIZE];
static uint8_t starter_stack[TASK_STACK_SIZE];

static void *native_notifier(void *arg)
{
    const struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 10000000L,
    };

    (void)arg;
    (void)nanosleep(&delay, NULL);

    if (task_notify_give(&waiter_task) != RTOS_OK)
    {
        _Exit(2);
    }

    return NULL;
}

static void waiter_entry(void *arg)
{
    (void)arg;

    if (task_notify_take() != RTOS_OK)
    {
        _Exit(3);
    }

    _Exit(0);
}

static void starter_entry(void *arg)
{
    pthread_t notifier;

    (void)arg;

    if (pthread_create(&notifier, NULL, native_notifier, NULL) != 0)
    {
        _Exit(4);
    }
    (void)pthread_detach(notifier);

    for (;;)
    {
        if (task_notify_take() != RTOS_OK)
        {
            _Exit(5);
        }
    }
}

int main(void)
{
    if (rtos_init() != RTOS_OK)
    {
        return 1;
    }

    if (task_create(
            &waiter_task,
            waiter_entry,
            NULL,
            0U,
            waiter_stack,
            sizeof(waiter_stack)) != RTOS_OK)
    {
        return 1;
    }

    if (task_create(
            &starter_task,
            starter_entry,
            NULL,
            1U,
            starter_stack,
            sizeof(starter_stack)) != RTOS_OK)
    {
        return 1;
    }

    return rtos_start();
}
