#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum
{
	RTOS_OK = 0,
	RTOS_ERR_NULL = -1,
	RTOS_ERR_INVALID_ARG = -2,
	RTOS_ERR_NO_TASKS = -3,
	RTOS_ERR_INTERNAL = -4,
	RTOS_ERR_EMPTY = -5,
}
rtos_status_t;

typedef uint64_t rtos_tick_t;
typedef uint64_t rtos_time_us_t;

typedef void (*task_entry_t)(void *);

