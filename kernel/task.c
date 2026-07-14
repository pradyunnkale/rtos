#include <stddef.h>
#include <stdint.h>

#include "rtos_task.h"
#include "port.h"
#include "rtos_sched.h"
#include "task_internal.h"
#include "rtos_types.h"


static uint8_t next_pid;

void task_system_init(void)
{
	next_pid = 0;
}

rtos_status_t task_init(task_t* task, uint8_t pid, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack_base, size_t stack_size)
{
	if (task == NULL || entry == NULL || stack_base == NULL || stack_size == 0) {
		return RTOS_ERR_INVALID_ARG;
	}

	task->pid = pid;
	task->entry = entry;
	task->arg = arg;
	task->priority = priority;
	task->stack_base = stack_base;
	task->stack_size = stack_size;

	task->next = NULL;
	task->stack_ptr = stack_base + stack_size;
	task->state = TASK_CREATED; 
	task->wake_time = 0;
	
	return RTOS_OK;
}

void task_set_state(task_t *task, task_state_t state)
{
	task->state = state;
}

rtos_status_t task_create(task_t *task, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack, size_t stack_size)
{
	rtos_status_t status = task_init(task, next_pid, entry, arg, priority, stack, stack_size);

	if (status != RTOS_OK)
	{
		return status;
	}

	status = port_context_init(task);

	if (status != RTOS_OK)
	{
		return status;
	}

	status = sched_add_task(task);

	if (status != RTOS_OK)
	{
		return status;
	}

	next_pid++;

	return RTOS_OK;
}

void task_yield(void)
{
	// Use the sched.c version because this should just call a scheduler alg
	(void)rtos_sched_yield();
}

rtos_status_t task_sleep_until(rtos_time_t wake_time)
{
	// Use the sched.c version because this is also dependent on the sched alg
	return sched_sleep_current_until(wake_time);
}
