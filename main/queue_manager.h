// queue_manager.h
#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "queues.h"

QueueHandle_t create_queue(int length, int item_size);
void push_to_queue(QueueHandle_t queue, const void *item, TickType_t ticks_to_wait);
bool pull_from_queue(QueueHandle_t queue, void *item, TickType_t ticks_to_wait);
#endif