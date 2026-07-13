#pragma once

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
