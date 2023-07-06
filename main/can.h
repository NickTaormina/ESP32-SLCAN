#ifndef CAN_H
#define CAN_H

#include "queue_manager.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define CAN_TX_GPIO GPIO_NUM_9
#define CAN_RX_GPIO GPIO_NUM_8

// rtos
void can_task(void *pvParameters);
void can_init(int bitrate);

// local functions
bool write_can_message(twai_message_t message);

#endif // CAN_H