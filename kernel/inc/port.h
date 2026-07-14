#pragma once
#include "port_types.h"
#include "rtos_types.h"
#include "rtos_task.h"
#include "time_internal.h"
#include <stdint.h>


rtos_status_t port_context_init(task_t *task);
rtos_status_t port_context_switch(task_t *from, task_t *to);
rtos_status_t port_start_first_task(task_t *task);
rtos_status_t port_critical_enter(port_critical_state_t *state);
rtos_status_t port_critical_exit(const port_critical_state_t *state);
void port_request_context_switch(void);

rtos_status_t port_timer_init(void);
rtos_status_t port_timer_arm(rtos_time_t deadline);
rtos_status_t port_timer_disarm(void);
