#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "slcan.h"
#include <stdio.h>
#include "queue_manager.h"
#include "can.h"
#include "hal/usb_serial_jtag_ll.h"
#include "driver/usb_serial_jtag.h"

void app_main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setbuf(stdout, NULL);
    usb_serial_jtag_driver_config_t usb_serial_jtag_driver_config = {
        .rx_buffer_size = 256,
        .tx_buffer_size = 64,
    };
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_driver_config));

    open_can_interface();

    can_send_queue = xQueueCreate(10, sizeof(twai_message_t));
    can_receive_queue = xQueueCreate(10, sizeof(twai_message_t));
    serial_in_queue = xQueueCreate(10, 20 * sizeof(uint8_t *));
    serial_out_queue = xQueueCreate(10, 20 * sizeof(uint8_t *));
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // start the CAN driver
    vTaskDelay(5 / portTICK_PERIOD_MS);
    // Create the twai and slcan tasks
    xTaskCreate(slcan_task, "slcan_task", 4096, NULL, 1, NULL);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    xTaskCreate(can_task, "can_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    ESP_LOGI("MAIN", "Setup finished");
}
