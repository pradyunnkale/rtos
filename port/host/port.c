#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <ucontext.h>

#include "port.h"
#include "port_types.h"
#include "rtos_task.h"
#include "rtos_types.h"
#include "sched.h"
#include "task_internal.h"
#include "time_internal.h"

static volatile sig_atomic_t context_switch_pending;
static volatile sig_atomic_t in_context_switch;


static void context_switch_handler(int signal_number)
{
	(void)signal_number;

	if (in_context_switch)
	{
		context_switch_pending = 1;
		return;
	}

	in_context_switch = 1;

	do
	{
		context_switch_pending = 0;
		(void)sched_preempt();
	}
	while(context_switch_pending);

	in_context_switch = 0;
}

static rtos_status_t port_context_switch_init(void)
{
	struct sigaction action = {0};

	action.sa_handler = context_switch_handler;
	if (sigemptyset(&action.sa_mask) == -1 ||
		sigaddset(&action.sa_mask, SIGALRM) == -1 ||
		sigaddset(&action.sa_mask, SIGUSR1) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	action.sa_flags = 0;
	if (sigaction(SIGUSR1, &action, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	context_switch_pending = 0;
	in_context_switch = 0;

	return RTOS_OK;
}

static void task_trampoline(uintptr_t task_arg)
{
	task_t *task = (task_t *)task_arg;

	task->entry(task->arg);

	sched_terminate_current();

	/* Tasks do not return */
	for (;;) {}
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

static void tick_handler(int signal_number)
{
	(void)signal_number;

	sched_wake_due_tasks(time_now());
}

static rtos_status_t port_tick_init(void)
{
	struct sigaction action = {0};
	struct itimerval timer = {0};

	action.sa_handler = tick_handler;

	if (sigemptyset(&action.sa_mask) == -1 ||
		sigaddset(&action.sa_mask, SIGALRM) == -1 ||
		sigaddset(&action.sa_mask, SIGUSR1) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	if (sigaction(SIGALRM, &action, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000;

	timer.it_value = timer.it_interval;

	if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
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

	rtos_status_t status = port_context_switch_init();

	if (status != RTOS_OK)
	{
		return status;
	}

	status = port_tick_init();

	if (status != RTOS_OK)
	{
		return status;
	}

	if (setcontext(&task->context.native) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_ERR_INTERNAL;
}

void port_request_context_switch(void)
{
	context_switch_pending = 1;

	if (!in_context_switch)
	{
		(void)raise(SIGUSR1);
	}
}

rtos_status_t port_critical_enter(port_critical_state_t *state)
{
	sigset_t blocked;

	if (state == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (sigemptyset(&blocked) == -1 ||
		sigaddset(&blocked, SIGALRM) == -1 ||
		sigaddset(&blocked, SIGUSR1) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	if (sigprocmask(SIG_BLOCK, &blocked, &state->previous_mask) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_critical_exit(const port_critical_state_t *state)
{
	if (state == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (sigprocmask(SIG_SETMASK, &state->previous_mask, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}
