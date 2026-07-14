#pragma once

#include "rtos_time.h"
#include "rtos_types.h"
#include "rtos_task.h"
#include <stdint.h>

rtos_status_t sched_init(void);
rtos_status_t sched_start(void);

rtos_status_t sched_add_task(task_t *task);

task_t *sched_current_task(void);

rtos_status_t rtos_sched_yield(void);
rtos_status_t sched_block_current(void);
rtos_status_t sched_sleep_current_until(rtos_time_t wake_time);
rtos_status_t sched_terminate_current(void);

void sched_wake_due_tasks(rtos_time_t now);
rtos_status_t sched_preempt(void);
