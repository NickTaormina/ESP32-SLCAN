#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "string.h"
#include "stdlib.h"

#define SL_CAN_TX_GPIO GPIO_NUM_9
#define SL_CAN_RX_GPIO GPIO_NUM_8

#define SLCAN_CR '\r'
#define SLCAN_BEL '\a'
#define SLCAN_ALL 'A'
#define SLCAN_FLAG 'F'
#define SLCAN_TR11 't'
#define SLCAN_TR29 'T'

#define SLCAN_FRAME_LEN_OFFSET 4
#define SLCAN_FRAME_DATA_OFFSET 5

extern TaskHandle_t readHandle;

extern QueueHandle_t canQueue;
extern QueueHandle_t commandQueue;

bool busIsRunning;

void slcan_init(void);
void slcan_ack();
void slcan_nack();
void slcan_receiveFrame(twai_message_t message);
void processSlCommand(uint8_t *bytes);
