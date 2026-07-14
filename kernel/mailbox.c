#include "port.h"
#include "port_types.h"
#include "rtos_mailbox.h"
#include "rtos_types.h"
#include <stdbool.h>
#include <string.h>

static rtos_status_t mailbox_enter(
	rtos_mailbox_t *mailbox,
	port_critical_state_t *critical)
{
	rtos_status_t status;

	status = port_critical_enter(critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	status = port_shared_lock_acquire(&mailbox->lock);
	if (status != RTOS_OK)
	{
		(void)port_critical_exit(critical);
		return status;
	}

	return RTOS_OK;
}

static rtos_status_t mailbox_exit(
	rtos_mailbox_t *mailbox,
	const port_critical_state_t *critical)
{
	rtos_status_t unlock_status;
	rtos_status_t critical_status;

	unlock_status = port_shared_lock_release(&mailbox->lock);

	critical_status = port_critical_exit(critical);

	if (unlock_status != RTOS_OK)
	{
		return unlock_status;
	}

	return critical_status;
}

rtos_status_t mailbox_init(rtos_mailbox_t *mailbox, void *storage, size_t item_size)
{
	rtos_status_t status;

	if (mailbox == NULL || storage == NULL || item_size == 0U)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	status = port_shared_lock_init(&mailbox->lock);
	if (status != RTOS_OK)
	{
		return status;
	}

	mailbox->storage = storage;
	mailbox->item_size = item_size;
	mailbox->sequence = 0U;
	mailbox->valid = false;

	return RTOS_OK;
}

rtos_status_t mailbox_publish(rtos_mailbox_t *mailbox, const void *item)
{
	port_critical_state_t critical;
	rtos_status_t status;

	if (mailbox == NULL || item == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (mailbox->storage == NULL || mailbox->item_size == 0U)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	status = mailbox_enter(mailbox, &critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	memcpy(mailbox->storage, item, mailbox->item_size);
	mailbox->sequence++;
	mailbox->valid = true;

	return mailbox_exit(mailbox, &critical);
}

rtos_status_t mailbox_read_latest(rtos_mailbox_t *mailbox, void *item, uint64_t *sequence)
{
	port_critical_state_t critical;
	rtos_status_t status;
	rtos_status_t exit_status;

	if (mailbox == NULL || item == NULL)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	if (mailbox->storage == NULL || mailbox->item_size == 0U)
	{
		return RTOS_ERR_INVALID_ARG;
	}

	status = mailbox_enter(mailbox, &critical);
	if (status != RTOS_OK)
	{
		return status;
	}

	if (!mailbox->valid)
	{
		status = RTOS_ERR_EMPTY;
	}
	else
	{
		memcpy(item, mailbox->storage, mailbox->item_size);

		if (sequence != NULL)
		{
			*sequence = mailbox->sequence;
		}

		status = RTOS_OK;
	}

	exit_status = mailbox_exit(mailbox, &critical);

	if (exit_status != RTOS_OK)
	{
		return exit_status;
	}

	return status;
}
