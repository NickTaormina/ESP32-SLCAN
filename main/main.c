#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "slcan.h"
#include <stdio.h>

QueueHandle_t canQueue;
QueueHandle_t commandQueue;

// receives CAN messages and places them into the queue
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
            if (message.identifier == 0x7E8 || message.identifier == 0x7DF)
            {
                // Print the received message
                printf("t/");
                printf("%03X", message.identifier);
                for (int i = 0; i < message.data_length_code; i++)
                {
                    printf("%02X", message.data[i]);
                }
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
                ESP_LOGI("MAIN", "Message transmitted");
                printf("t/");
                printf("%03X", obdMsg.identifier);
                for (int i = 0; i < obdMsg.data_length_code; i++)
                {
                    printf("%02X", obdMsg.data[i]);
                }
                printf("\n");
            }
            counter = 0;
        }
    }
}

// takes the can messages from queue and prints to usb serial
void transmitMessageToSerial(void *parameters)
{
}

void app_main()
{
    // start the CAN driver
    slcan_init();
    // Create a task that will receive and print the twai messages
    xTaskCreate(twai_receive_task, "twai receive", 2048, NULL, 5, NULL);
    ESP_LOGI("MAIN", "CAN receive task created");
}
