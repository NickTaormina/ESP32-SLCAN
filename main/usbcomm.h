#ifndef USBCOMM_H
#define USBCOMM_H

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "queues.h"

xQueueHandle *usbcomm_tx_queue;
xQueueHandle *usbcomm_rx_queue;

void init_usbcomm(void);
void usbcomm_tx_task(void *pvParameter);
void usbcomm_rx_task(void *pvParameter);

void flush_output();
#endif