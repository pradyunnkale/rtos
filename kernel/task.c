#include <stddef.h>
#include <stdint.h>
#include "task.h"
#include "task_internal.h"
#include "rtos.h"
#include "rtos_types.h"

static task_t *ready_list;
static task_t *sleep_list;
static task_t *current_task;
static uint8_t next_pid;

void task_system_init(void)
{
	ready_list = NULL;
	sleep_list = NULL;
	current_task = NULL;
	next_pid = 0;
}

rtos_status_t task_init(task_t* task, uint8_t pid, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack_base, size_t stack_size)
{
	if (task == NULL || arg == NULL || stack_base == NULL || stack_size == 0) {
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

	next_pid++;

	return RTOS_OK;
}

void task_yield(void)
{
	// Use the sched.c version because this should just call a scheduler alg
}

rtos_status_t task_sleep(uint64_t ticks)
{
	// Use the sched.c version because this is also dependent on the sched alg
	return RTOS_OK;
}
