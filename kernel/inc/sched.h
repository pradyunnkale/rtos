#pragma once

#include "rtos_types.h"
#include "task.h"
#include <stdint.h>

rtos_status_t sched_init(void);
rtos_status_t sched_start(void);

rtos_status_t sched_add_task(task_t *task);
rtos_status_t sched_remove_task(task_t *task);

task_t *sched_current_task(void);

rtos_status_t sched_yield(void);
rtos_status_t sched_block_current(void);
rtos_status_t sched_sleep_current_until(uint64_t ticks);

