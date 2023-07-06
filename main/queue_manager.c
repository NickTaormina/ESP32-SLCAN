#include "queue_manager.h"

QueueHandle_t create_queue(int length, int item_size)
{
    return xQueueCreate(length, item_size);
}

void push_to_queue(QueueHandle_t queue, const void *item, TickType_t ticks_to_wait)
{
    xQueueSend(queue, item, ticks_to_wait);
}

bool pull_from_queue(QueueHandle_t queue, void *item, TickType_t ticks_to_wait)
{
    return xQueueReceive(queue, item, ticks_to_wait);
}