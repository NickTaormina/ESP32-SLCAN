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

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "stdio.h"
#include "string.h"

#include "nvs_flash.h"
#include "nvs.h"

// receives CAN messages and places them into the queue
void twai_receive_task(void *arg)
{
    // Buffer for a received message
    twai_message_t message;
    ESP_LOGI("MAIN", "CAN receive task started");
    char can_frame_buffer[32];
    int counter = 0;
    while (1)
    {
        // Receive twai message and wait for a maximum of 1 second until a message is received
        if (twai_receive(&message, 100) == ESP_OK)
        {
            if (message.identifier == 0x7E8)
            {
                ESP_LOGE("MAIN", "Received OBD message");
            }
            /*int len = snprintf(can_frame_buffer, 32, "t%03X%01X", message.identifier, message.data_length_code);
            for (int i = 0; i < message.data_length_code; i++)
            {
                len += snprintf(can_frame_buffer + len, 32 - len, "%02X", message.data[i]);
            }
            len += snprintf(can_frame_buffer + len, 32 - len, "\r");
            // usb_serial_jtag_write_bytes((const void *)can_frame_buffer, strlen(can_frame_buffer), 10);
            // usb_serial_jtag_ll_txfifo_flush();
            printf("%s", can_frame_buffer);
            fflush(stdout);*/
            // slcan_ack();
        }
        else
        {
            ESP_LOGI("MAIN", "No message received");
        }
        /*counter++;
        if (counter >= 1000)
        {
            twai_message_t obdMsg;
            obdMsg.identifier = 0x7DF;
            obdMsg.data_length_code = 8;
            obdMsg.data[0] = 0x01;
            obdMsg.data[1] = 0x03;
            obdMsg.data[2] = 0x00;

            esp_err_t err = twai_transmit(&obdMsg, 1000);
            if (err != ESP_OK)
            {
                ESP_LOGI("MAIN", "Failed to transmit message: %d", err);
            }
            else
            {
                // ESP_LOGI("MAIN", "Message transmitted");
                 printf("t/");
                 printf("%03X", obdMsg.identifier);
                 for (int i = 0; i < obdMsg.data_length_code; i++)
                 {
                     printf("%02X", obdMsg.data[i]);
                 }
                 printf("\n");
            }*/
        counter = 0;
    }
}

void app_main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setbuf(stdout, NULL);
    spiffs_init();
    // Example usage:
    const char *filename = "/spiffs/CAN.TXT";
    const char *data = "Hello, SPIFFS!";
    // char buffer[100];

    // write_file(filename, data);
    // read_file(filename, buffer, sizeof(buffer));

    // printf("Read from file: %s\n", buffer);
    usb_serial_jtag_driver_config_t usb_serial_jtag_driver_config = {
        .rx_buffer_size = 256,
        .tx_buffer_size = 1,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_driver_config));
    // char *buffer = malloc(256);
    char *buffer = read_spiffs_file_to_buffer("/spiffs/CAN.TXT");
    // read_file("/spiffs/CAN.TXT", buffer, sizeof(buffer));
    printf("test\n");
    printf("file: %s\n\n\n ", buffer);

    can_send_queue = xQueueCreate(10, sizeof(twai_message_t));
    can_receive_queue = xQueueCreate(10, sizeof(twai_message_t));
    serial_in_queue = xQueueCreate(10, 20 * sizeof(uint8_t *));
    serial_out_queue = xQueueCreate(10, 20 * sizeof(uint8_t *));
    vTaskDelay(1 / portTICK_PERIOD_MS);

    uint8_t *rxbf = (uint8_t *)malloc(1028);
    // start the CAN driver
    slcan_init();
    vTaskDelay(5 / portTICK_PERIOD_MS);
    // Create the twai and slcan tasks
    xTaskCreate(slcan_task, "slcan_task", 4096, NULL, 1, NULL);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    xTaskCreate(can_task, "can_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    // xTaskCreate(twai_receive_task, "twai_receive_task", 4096, NULL, 10, NULL);
    ESP_LOGI("MAIN", "Setup finished");
}
