#include "rtos_kernel.h"

#include "task_internal.h"
#include "sched.h"
#include "time_internal.h"

rtos_status_t rtos_init(void)
{
    rtos_status_t status;

    task_system_init();

    status = sched_init();
    if (status != RTOS_OK)
    {
        return status;
    }

    return RTOS_OK;
}

rtos_status_t rtos_start(void)
{
    return sched_start();
}
