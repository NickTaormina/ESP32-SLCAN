#include <stdio.h>

#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// Define the GPIO pins for the twai Bus
#define TWAI_TX_GPIO GPIO_NUM_9
#define TWAI_RX_GPIO GPIO_NUM_10

void twai_receive_task(void *arg)
{
    // Buffer for a received message
    twai_message_t message;
    while (1)
    {
        // Receive twai message and wait for a maximum of 1 second until a message is received
        if (twai_receive(&message, 1000 / portTICK_RATE_MS) == ESP_OK)
        {
            // Print the received message
            printf("t/");
            printf("%03X", message.identifier);
            for (int i = 0; i < message.data_length_code; i++)
            {
                printf("%02X", message.data[i]);
            }
            printf("\n");
        }
    }
}

void app_main()
{
    // Configuration for the twai bus
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_GPIO, TWAI_RX_GPIO, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install the driver for the twai bus
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
    {
        printf("Failed to install twai driver\n");
        return;
    }

    // Start the twai bus
    if (twai_start() != ESP_OK)
    {
        printf("Failed to start twai bus\n");
        return;
    }

    // Create a task that will receive and print the twai messages
    xTaskCreate(twai_receive_task, "twai receive", 2048, NULL, 5, NULL);
}
