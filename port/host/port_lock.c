#include <pthread.h>

#include "port.h"
#include "port_types.h"
#include "rtos_types.h"

rtos_status_t port_shared_lock_init(port_shared_lock_t *lock)
{
	if (lock == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (pthread_mutex_init(&lock->native, NULL) != 0)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_shared_lock_acquire(port_shared_lock_t *lock)
{
	if (lock == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (pthread_mutex_lock(&lock->native) != 0)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}

rtos_status_t port_shared_lock_release(port_shared_lock_t *lock)
{
	if (lock == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (pthread_mutex_unlock(&lock->native) != 0)
	{
		return RTOS_ERR_INTERNAL;
	}

	return RTOS_OK;
}
