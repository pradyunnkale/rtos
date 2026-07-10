#include "rtos_time.h"
#include "time_internal.h"
#include "port_time.h"

rtos_status_t time_init(void)
{
	return port_time_init();
}

rtos_tick_t time_now(void)
{
	return port_time_now();
}

rtos_tick_t rtos_time_now(void)
{
    return time_now();
}

rtos_time_us_t rtos_time_now_us(void)
{
    return port_time_now_us();
}
