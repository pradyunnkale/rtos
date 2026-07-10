#pragma once
#include "rtos_types.h"
#include "rtos_task.h"
#include "time_internal.h"
#include <stdint.h>


rtos_status_t port_context_init(task_t *task);
rtos_status_t port_context_switch(task_t *from, task_t *to);
rtos_status_t port_start_first_task(task_t *task);
void port_request_context_switch(void);
