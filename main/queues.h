#ifndef QUEUES_H
#define QUEUES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t can_send_queue;
QueueHandle_t can_receive_queue;

QueueHandle_t serial_in_queue;
QueueHandle_t serial_out_queue;

#endif // QUEUES_H