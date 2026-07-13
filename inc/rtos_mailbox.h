#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rtos_types.h"

typedef struct
{
  void *storage;
  size_t item_size;
  uint64_t sequence;
  bool valid;
}
rtos_mailbox_t;

rtos_status_t mailbox_init(rtos_mailbox_t *mailbox, void *storage, size_t item_size);
rtos_status_t mailbox_publish(rtos_mailbox_t *mailbox, const void *item);
rtos_status_t mailbox_read_latest(rtos_mailbox_t *mailbox, void *item, uint64_t *sequence);

