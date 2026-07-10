#include "port.h"
#include "rtos_types.h"
#include "task.h"
#include <stdint.h>
#include <ucontext.h>
#include "sched.h"

static void task_trampoline(uintptr_t task_arg)
{
	task_t *task = (task_t *)task_arg;

	task->entry(task->arg);

	/* Tasks do not return */
	for (;;)
	{
		sched_yield();
	}
}

rtos_status_t port_context_init(task_t *task)
{
	if (task == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (getcontext(&task->context.native) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	task->context.native.uc_stack.ss_sp = task->stack_base;
	task->context.native.uc_stack.ss_size = task->stack_size;
	task->context.native.uc_stack.ss_flags = 0;
	task->context.native.uc_link = NULL;

	makecontext(&task->context.native, (void (*)(void))task_trampoline, 1, (uintptr_t)task);

	return RTOS_OK;
}

rtos_status_t port_context_switch(task_t *from, task_t *to)
{
	if (from == NULL || to == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (from == to)
	{
		return RTOS_OK;
	}

	if (swapcontext(&from->context.native, &to->context.native) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_start_first_task(task_t *task)
{
	if (task == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (setcontext(&task->context.native) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_ERR_INTERNAL;
}
