#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <time.h>

#include "port_time.h"
#include "rtos_types.h"

#define NS_PER_SECOND 1000000000ULL
#define NS_PER_MS     1000000ULL
#define NS_PER_US     1000ULL

static struct timespec start_time;

static uint64_t port_elapsed_ns(void)
{
    struct timespec now;
    int64_t elapsed_sec;
    int64_t elapsed_nsec;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
    {
        return 0;
    }

    elapsed_sec =
        (int64_t)now.tv_sec -
        (int64_t)start_time.tv_sec;

    elapsed_nsec =
        (int64_t)now.tv_nsec -
        (int64_t)start_time.tv_nsec;

    if (elapsed_nsec < 0)
    {
        elapsed_sec--;
        elapsed_nsec += (int64_t)NS_PER_SECOND;
    }

    if (elapsed_sec < 0)
    {
        return 0;
    }

    return
        ((uint64_t)elapsed_sec * NS_PER_SECOND) +
        (uint64_t)elapsed_nsec;
}

rtos_status_t port_time_init(void)
{
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
    {
        return RTOS_ERR_INTERNAL;
    }

    return RTOS_OK;
}

rtos_tick_t port_time_now(void)
{
    return (rtos_tick_t)(port_elapsed_ns() / NS_PER_MS);
}

rtos_time_us_t port_time_now_us(void)
{
    return (rtos_time_us_t)(port_elapsed_ns() / NS_PER_US);
}
