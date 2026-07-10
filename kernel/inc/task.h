#pragma once

#include <stdint.h>
#include <stddef.h>
#include "rtos_types.h"

typedef enum 
{
	TASK_READY,
	TASK_SLEEP,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DONE,
	TASK_CREATED,
} 
task_state_t;


typedef struct task 
{
	uint8_t pid;
	uint8_t priority;
	task_state_t state;
	uint64_t wake_time;

	task_entry_t entry;
	void *arg;

	uint8_t *stack_base;
	uint8_t *stack_ptr;
	size_t stack_size;

	struct task *next;
} 
task_t;

rtos_status_t task_create(task_t *task, task_entry_t entry, void *arg, uint8_t priority, uint8_t *stack, size_t stack_size);
void task_yield(void);
rtos_status_t task_sleep_until(uint64_t ticks);
