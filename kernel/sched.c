#include "sched.h"
#include "port.h"
#include "rtos_types.h"
#include "task.h"
#include "task_internal.h"

static task_t *ready_list;
static task_t *sleep_list;
static task_t *current_task;

static void ready_list_add(task_t *task)
{
	task_t *curr = ready_list;
	task_t *prev = NULL;
	task->next = NULL;

	// Add based on priority, lower number = higher priority
	while (curr != NULL && curr->priority <= task->priority)
	{
		prev = curr;
		curr = curr->next;
	}

	if (prev == NULL)
	{
		task->next = ready_list;
		ready_list = task;
		return;
	}

	prev->next = task;
	task->next = curr;
}

static void sleep_list_add(task_t *task)
{
	task_t *curr = sleep_list;
	task_t *prev = NULL;
	task->next = NULL;

	// Add based on wake_time, lower number = earlier wake time 
	while (curr != NULL && curr->wake_time <= task->wake_time)
	{
		prev = curr;
		curr = curr->next;
	}

	if (prev == NULL)
	{
		task->next = sleep_list;
		sleep_list = task;
		return;
	}

	prev->next = task;
	task->next = curr;
}

static task_t *ready_list_pop(void)
{
	task_t *to_remove = ready_list;
	
	if (to_remove == NULL)
	{
		return to_remove;
	}

	ready_list = ready_list->next;
	to_remove->next = NULL;
	return to_remove;
}

static task_t *sched_pick_next(void) 
{
	return ready_list_pop();
}

rtos_status_t sched_init(void) 
{
	ready_list = NULL;
	sleep_list = NULL;
	current_task = NULL;

	return RTOS_OK;
}

rtos_status_t sched_start(void) 
{
	current_task = sched_pick_next();

	if (current_task == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	task_set_state(current_task, TASK_RUNNING);
	port_start_first_task(current_task);

	// Should not normally return
	return RTOS_ERR_INTERNAL;
}

rtos_status_t sched_add_task(task_t *task) 
{
	if (task == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	task_set_state(task, TASK_READY);
	ready_list_add(task);

	return RTOS_OK;
}

task_t *sched_current_task(void)
{
	return current_task;
}

rtos_status_t sched_yield(void)
{

	task_t *old = current_task;
	task_t *new = NULL;

	if (old != NULL)
	{
		task_set_state(old, TASK_READY);
		ready_list_add(old);
	}

	new = sched_pick_next();

	if (new == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	current_task = new;
	task_set_state(current_task, TASK_RUNNING);

	port_context_switch(old, new);

	return RTOS_OK;
}

rtos_status_t sched_block_current(void);

rtos_status_t sched_sleep_current_until(uint64_t ticks)
{

}
