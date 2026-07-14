#include "rtos_sched.h"
#include "port.h"
#include "rtos_types.h"
#include "rtos_task.h"
#include "task_internal.h"
#include <stdbool.h>

static task_t *ready_list;
static task_t *sleep_list;
static task_t *current_task;

static void sched_update_timer(void)
{
	if (sleep_list == NULL)
	{
		(void)port_timer_disarm();
	}
	else
	{
		(void)port_timer_arm(sleep_list->wake_time);
	}
}

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
	port_critical_state_t critical;
	rtos_status_t status;

	if (task == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	task_set_state(task, TASK_READY);
	ready_list_add(task);

	status = port_critical_exit(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	return RTOS_OK;
}

task_t *sched_current_task(void)
{
	return current_task;
}

rtos_status_t rtos_sched_yield(void)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;
	task_t *old;
	task_t *new;

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	old = current_task;

	if (old == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	task_set_state(old, TASK_READY);
	ready_list_add(old);

	new = sched_pick_next();

	if (new == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	current_task = new;
	task_set_state(current_task, TASK_RUNNING);

	status = port_context_switch(old, new);

exit_critical:
	exit_status = port_critical_exit(&critical);

	if (status != RTOS_OK)
	{
		return status;
	}

	return exit_status;
}

rtos_status_t sched_block_current(void)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;
	task_t *blocked;
	task_t *next;

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	blocked = current_task;

	if (blocked == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	task_set_state(blocked, TASK_BLOCKED);

	next = sched_pick_next();

	if (next == NULL)
	{
		task_set_state(blocked, TASK_RUNNING);
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	status = port_context_switch(blocked, next);

exit_critical:
	exit_status = port_critical_exit(&critical);
	return status != RTOS_OK ? status : exit_status;
}

rtos_status_t sched_sleep_current_until(rtos_time_t wake_time)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;
	task_t *sleeping;
	task_t *next;

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	sleeping = current_task;

	if (sleeping == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	if (wake_time <= time_now())
	{
		status = RTOS_OK;
		goto exit_critical;
	}

	if (ready_list == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	sleeping->wake_time = wake_time;

	task_set_state(sleeping, TASK_SLEEP);
	sleep_list_add(sleeping);
	sched_update_timer();

	next = sched_pick_next();

	if (next == NULL)
	{
		status = RTOS_ERR_INTERNAL;
		goto exit_critical;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	status = port_context_switch(sleeping, next);

exit_critical:
	exit_status = port_critical_exit(&critical);
	return status != RTOS_OK ? status : exit_status;
}

rtos_status_t sched_terminate_current(void)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;
	task_t *terminated;
	task_t *next;

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	terminated = current_task;

	if (terminated == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	if (ready_list == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	task_set_state(terminated, TASK_TERMINATED);

	next = sched_pick_next();

	if (next == NULL)
	{
		status = RTOS_ERR_INTERNAL;
		goto exit_critical;
	}

	current_task = next;
	task_set_state(next, TASK_RUNNING);

	status = port_context_switch(terminated, next);

exit_critical:
	exit_status = port_critical_exit(&critical);
	return status != RTOS_OK ? status : exit_status;
}

static bool sched_should_preempt(void)
{
	if (current_task == NULL || ready_list == NULL)
	{
		return false;
	}

	return ready_list->priority < current_task->priority;
}

void sched_wake_due_tasks(rtos_time_t now)
{
	port_critical_state_t critical;

	if (port_critical_enter(&critical) != RTOS_OK)
	{
		return;
	}

	while (sleep_list != NULL && sleep_list->wake_time <= now)
	{
		task_t *task = sleep_list;

		sleep_list = sleep_list->next;
		task->next = NULL;

		task_set_state(task, TASK_READY);
		ready_list_add(task);
	}

	sched_update_timer();

	if (sched_should_preempt())
	{
		port_request_context_switch();
	}

	(void)port_critical_exit(&critical);
}

rtos_status_t sched_preempt(void)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;
	task_t *old;
	task_t *new;

	status = port_critical_enter(&critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	if (!sched_should_preempt())
	{
		status = RTOS_OK;
		goto exit_critical;
	}

	old = current_task;

	if (old == NULL)
	{
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	task_set_state(old, TASK_READY);
	ready_list_add(old);

	new = sched_pick_next();

	if (new == NULL)
	{
		current_task = old;
		task_set_state(old, TASK_RUNNING);
		status = RTOS_ERR_NO_TASKS;
		goto exit_critical;
	}

	current_task = new;
	task_set_state(new, TASK_RUNNING);

	status = port_context_switch(old, new);

exit_critical:
	exit_status = port_critical_exit(&critical);
	return status != RTOS_OK ? status : exit_status;
}
