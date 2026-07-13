#include "port.h"
#include "port_types.h"
#include "rtos_mailbox.h"
#include "rtos_types.h"
#include <stdbool.h>
#include <string.h>

rtos_status_t mailbox_init(rtos_mailbox_t *mailbox, void *storage, size_t item_size)
{
  if (mailbox == NULL || storage == NULL)
  {
    return RTOS_ERR_INVALID_ARG;
  }

  if (item_size == 0U)
  {
    return RTOS_ERR_INVALID_ARG;
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

  status = port_critical_enter(&critical);
  if (status != RTOS_OK)
  {
    return status;
  }

  memcpy(mailbox->storage, item, mailbox->item_size);
  mailbox->sequence++;
  mailbox->valid = true;

  return port_critical_exit(&critical);
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

  status = port_critical_enter(&critical);
  if (status != RTOS_OK)
  {
    return status;
  }

  if (!mailbox->valid)
  {
    status = RTOS_ERR_EMPTY;
    goto exit_critical;
  }

  memcpy(item, mailbox->storage, mailbox->item_size);

  if (sequence != NULL)
  {
    *sequence = mailbox->sequence;
  }

  status = RTOS_OK;

exit_critical:
  exit_status = port_critical_exit(&critical);
  return status != RTOS_OK ? status : exit_status;

}