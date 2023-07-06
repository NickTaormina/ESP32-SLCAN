#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// Define the GPIO pins for the twai Bus
#define TWAI_TX_GPIO GPIO_NUM_9
#define TWAI_RX_GPIO GPIO_NUM_8

void twai_receive_task(void *arg)
{
    // Buffer for a received message
    twai_message_t message;
    ESP_LOGI("MAIN", "CAN receive task started");
    int counter = 0;
    while (1)
    {
        // Receive twai message and wait for a maximum of 1 second until a message is received
        if (twai_receive(&message, 100) == ESP_OK)
        {
            if (message.identifier == 0x7E8)
            {
                // Print the received message
                printf("t/");
                printf("%03X", message.identifier);
            }
        }
        else
        {
            ESP_LOGI("MAIN", "No message received");
        }
        counter++;
        if (counter >= 1000)
        {
            twai_message_t obdMsg;
            obdMsg.identifier = 0x7DF;
            obdMsg.data_length_code = 8;
            obdMsg.data[0] = 0x02;
            obdMsg.data[1] = 0x01;
            obdMsg.data[2] = 0x00;
            obdMsg.data[3] = 0x00;
            obdMsg.data[4] = 0x00;
            obdMsg.data[5] = 0x00;
            obdMsg.data[6] = 0x00;
            obdMsg.data[7] = 0x00;
            esp_err_t err = twai_transmit(&obdMsg, 1000);
            if (err != ESP_OK)
            {
                ESP_LOGI("MAIN", "Failed to transmit message: %d", err);
            }
            else
            {
                ESP_LOGI("MAIN", "Message transmitted");
            }
            counter = 0;
        }
    }
}

void app_main()
{
    ESP_LOGI("MAIN", "Initializing CAN bus");
    // Configuration for the twai bus
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_GPIO, TWAI_RX_GPIO, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_LOGI("MAIN", "CAN configs initialized");

    // Install the driver for the twai bus
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        printf("Failed to install twai driver\n");
        return;
    }
    ESP_LOGI("MAIN", "CAN driver installed");

    // Start the twai bus
    if (twai_start() != ESP_OK)
    {
        printf("Failed to start twai bus\n");
        return;
    }
    ESP_LOGI("MAIN", "CAN bus started");

    // Create a task that will receive and print the twai messages
    xTaskCreate(twai_receive_task, "twai receive", 2048, NULL, 5, NULL);
    ESP_LOGI("MAIN", "CAN receive task created");
}
