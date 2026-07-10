#pragma once
#include <stdint.h>
#include <sys/ucontext.h>

#define RTOS_PORT_CONTEXT_WORDS 1024

typedef struct 
{
	ucontext_t native;	
}
port_context_t;
