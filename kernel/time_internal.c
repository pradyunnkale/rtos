#include "rtos_time.h"
#include "time_internal.h"
#include "port_time.h"

rtos_time_t time_now(void)
{
	return port_time_now();
}

rtos_time_t rtos_time_now(void)
{
	return time_now();
}
