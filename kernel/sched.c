#include "sched.h"
#include "port.h"
#include "rtos_types.h"
#include "rtos_task.h"
#include "task_internal.h"
#include <stdbool.h>

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

	return port_start_first_task(current_task);
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

	return port_context_switch(old, new);
}

rtos_status_t sched_block_current(void)
{
	task_t *blocked = current_task;
	task_t *next;

	if (blocked == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	task_set_state(blocked, TASK_BLOCKED);

	next = sched_pick_next();

	if (next == NULL)
	{
		task_set_state(blocked, TASK_RUNNING);
		return RTOS_ERR_NO_TASKS;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	return port_context_switch(blocked, next);
}

rtos_status_t sched_sleep_current_until(uint64_t wake_time)
{
	task_t *sleeping = current_task;
	task_t *next;

	if (sleeping == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	sleeping->wake_time = wake_time;
	task_set_state(sleeping, TASK_SLEEP);
	sleep_list_add(sleeping);

	next = sched_pick_next();

	if (next == NULL)
	{
		current_task = NULL;
		return RTOS_ERR_NO_TASKS;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	return port_context_switch(sleeping, next);
}

rtos_status_t sched_terminate_current(void)
{
	task_t *terminated = current_task;
	task_t *next;

	if (terminated == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	task_set_state(terminated, TASK_TERMINATED);

	next = sched_pick_next();

	if (next == NULL)
	{
		current_task = NULL;
		return RTOS_ERR_NO_TASKS;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	return port_context_switch(terminated, next);
}

static bool sched_should_preempt(void)
{
	if (current_task == NULL || ready_list == NULL)
	{
		return false;
	}

	return ready_list->priority < current_task->priority;
}

void sched_wake_due_tasks(uint64_t now)
{
	while (sleep_list != NULL && sleep_list->wake_time <= now)
	{
		task_t *task = sleep_list;

		sleep_list = sleep_list->next;
		task->next = NULL;

		task_set_state(task, TASK_READY);
		ready_list_add(task);
	}

	if (sched_should_preempt())
	{
		port_request_context_switch();
	}
}

rtos_status_t sched_preempt(void)
{
	task_t *old;
	task_t *new;

	if (!sched_should_preempt())
	{
		return RTOS_OK;
	}

	old = current_task;

	if (old == NULL)
	{
		return RTOS_ERR_NO_TASKS;
	}

	task_set_state(old, TASK_READY);
	ready_list_add(old);

	new = sched_pick_next();

	if (new == NULL)
	{
		current_task = old;
		task_set_state(old, TASK_RUNNING);
		return RTOS_ERR_NO_TASKS;
	}

	current_task = new;
	task_set_state(new, TASK_RUNNING);

	return port_context_switch(old, new);
}
