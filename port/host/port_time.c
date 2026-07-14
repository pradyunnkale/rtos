#include "port.h"
#include "rtos_time.h"
#include "sched.h"
#include <stddef.h>
#include <signal.h>

#include <stdint.h>
#include <time.h>

#include "port_time.h"
#include "rtos_types.h"

static timer_t scheduler_timer;

rtos_time_t port_time_now(void)
{
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
	{
		return 0;
	}

	return ((rtos_time_t)now.tv_sec * RTOS_NS_PER_S) + (rtos_time_t)now.tv_nsec;
}

static void timer_handler(int signal_number)
{
	(void)signal_number;
	sched_wake_due_tasks(time_now());
}

rtos_status_t port_timer_init(void)
{
	struct sigaction action = {0};
	struct sigevent event = {0};

	action.sa_handler = timer_handler;
	action.sa_flags = 0;

	if (sigemptyset(&action.sa_mask) == -1 || sigaddset(&action.sa_mask, SIGALRM) == -1 || sigaddset(&action.sa_mask, SIGUSR1) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	if (sigaction(SIGALRM, &action, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGALRM;

	if (timer_create(CLOCK_MONOTONIC, &event, &scheduler_timer) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_timer_arm(rtos_time_t deadline)
{
	struct itimerspec timer = {0};

	timer.it_value.tv_sec = (time_t)(deadline / RTOS_NS_PER_S);
	timer.it_value.tv_nsec = (long)(deadline % RTOS_NS_PER_S);

	if (timer_settime(scheduler_timer, TIMER_ABSTIME, &timer, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_timer_disarm(void)
{
	struct itimerspec timer = {0};

	if (timer_settime(scheduler_timer, 0, &timer, NULL) == -1)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}
