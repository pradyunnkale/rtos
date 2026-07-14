#pragma once

#include <pthread.h>
#include <signal.h>
#include <ucontext.h>

typedef struct
{
	ucontext_t native;
}
port_context_t;

typedef struct
{
	sigset_t previous_mask;
}
port_critical_state_t;

typedef struct
{
	pthread_mutex_t native;
}
port_shared_lock_t;
