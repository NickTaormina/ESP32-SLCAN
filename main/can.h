#ifndef CAN_H
#define CAN_H

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define CAN_TX_GPIO GPIO_NUM_9
#define CAN_RX_GPIO GPIO_NUM_8

// interface
twai_timing_config_t bus_speed; // variable to store the bus speed
bool speed_set;                 // Boolean to track if speed has been set
bool setup_speed(char speed_code);
bool open_can_interface();

// local functions
bool write_can_message(twai_message_t message);

#endif // CAN_H