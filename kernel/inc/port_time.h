#pragma once

#include "rtos_types.h"

rtos_status_t port_time_init(void);
rtos_tick_t port_time_now(void);
rtos_time_us_t port_time_now_us(void);
