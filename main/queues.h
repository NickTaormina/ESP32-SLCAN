#ifndef QUEUES_H
#define QUEUES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t can_send_queue;
extern QueueHandle_t can_receive_queue;

extern QueueHandle_t serial_in_queue;
extern QueueHandle_t serial_out_queue;

#endif // QUEUES_H