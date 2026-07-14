#pragma once

#include "rtos_types.h"

#define RTOS_NS_PER_US 1000ULL
#define RTOS_NS_PER_MS 1000000ULL
#define RTOS_NS_PER_S  1000000000ULL

#define RTOS_US(value) ((rtos_time_t)(value) * RTOS_NS_PER_US)
#define RTOS_MS(value) ((rtos_time_t)(value) * RTOS_NS_PER_MS)
#define RTOS_S(value)  ((rtos_time_t)(value) * RTOS_NS_PER_S)

#define RTOS_TIME_TO_US(value) ((uint64_t)((value) / RTOS_NS_PER_US))
#define RTOS_TIME_TO_MS(value) ((uint64_t)((value) / RTOS_NS_PER_MS))

rtos_time_t rtos_time_now(void);
