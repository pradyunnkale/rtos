#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum
{
	RTOS_OK = 0,
	RTOS_ERR_NULL = -1,
	RTOS_ERR_INVALID_ARG = -2,
}
rtos_status_t;

typedef void (*task_entry_t)(void *);


