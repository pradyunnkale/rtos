#include <stdio.h>

#include "rtos.h"

#define STACK_SIZE 16384

static task_t task_a;
static task_t task_b;

static uint8_t stack_a[STACK_SIZE];
static uint8_t stack_b[STACK_SIZE];

static void task_a_entry(void *arg)
{
    (void)arg;

    unsigned counter = 0;

    for (;;)
    {
        printf("A: %u\n", counter++);
        task_yield();
    }
}

static void task_b_entry(void *arg)
{
    (void)arg;

    unsigned counter = 0;

    for (;;)
    {
        printf("B: %u\n", counter++);
        task_yield();
    }
}

int main(void)
{
    if (rtos_init() != RTOS_OK)
    {
        return 1;
    }

    if (task_create(
            &task_a,
            task_a_entry,
            NULL,
            1,
            stack_a,
            sizeof(stack_a)) != RTOS_OK)
    {
        return 1;
    }

    if (task_create(
            &task_b,
            task_b_entry,
            NULL,
            1,
            stack_b,
            sizeof(stack_b)) != RTOS_OK)
    {
        return 1;
    }

    return rtos_start();
}
