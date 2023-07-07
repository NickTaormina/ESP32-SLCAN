#include "can.h"

// define the queues
// QueueHandle_t can_send_queue;
// QueueHandle_t can_receive_queue;
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "stdio.h"
#include "string.h"

#define TAG "SPIFFS"

void can_task(void *pvParameters)
{
    ESP_LOGE("CAN", "CAN task started");
    twai_message_t *receiveMsg;
    twai_message_t sendMsg;
    while (1)
    {
        twai_status_info_t test;
        twai_get_status_info(&test);
        if (test.state == TWAI_STATE_RUNNING)
        {
            break;
        }
        else
        {
            printf("bus not open");
        }
    }
    char *can_frame_buffer = malloc(32);
    // continuously check for received messages
    while (1)
    {
        // ESP_LOGI("CAN", "CAN task running");
        //  receive can messages
        receiveMsg = (twai_message_t *)malloc(sizeof(twai_message_t));
        if (twai_receive(receiveMsg, 1 / portTICK_PERIOD_MS) == ESP_OK)
        {
            // push the received message to the queue
            // ESP_LOGI("CAN", "Received message");
            if (1)
            {
                // ESP_LOGI("CAN", "Received OBD message");

                // len += snprintf(can_frame_buffer + len, 32 - len, "\r");
                printf("t%03X%01X%02X%02X%02X%02X%02X%02X%02X%02X\r", receiveMsg->identifier, receiveMsg->data_length_code, receiveMsg->data[0], receiveMsg->data[1], receiveMsg->data[2], receiveMsg->data[3], receiveMsg->data[4], receiveMsg->data[5], receiveMsg->data[6], receiveMsg->data[7]);
                // printf("\n");
                fflush(stdout);
                fflush(stderr);

                // ESP_LOGI("CAN", "ligma");
            }

            // push_to_queue(can_receive_queue, &receiveMsg, 0);
            //   ESP_LOGI("CAN", "Pushed message to queue");
        }
        else
        {
            // ESP_LOGE("CAN", "No message received");
            //  send the slcan noack message (push bad frame to the queue or something)
        }
        free(receiveMsg);
        // ESP_LOGI("CAN", "Checking for messages to send");
        //  pull messages from the queue and send them

        // vTaskDelay(1 / portTICK_RATE_MS);
    }
}

// todo: implement bitrate switching
void can_init(int bitrate)
{
    ESP_LOGI("MAIN", "Initializing CAN bus");
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NO_ACK);
    g_config.rx_queue_len = 500;
    g_config.tx_queue_len = 5;
    // g_config.intr_flags = ESP_INTR_FLAG_IRAM;
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_LOGI("MAIN", "CAN configs initialized");
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        ESP_LOGI("CAN", "CAN Driver installed\n");
    }
    else
    {
        ESP_LOGE("CAN", "Failed to install driver\n");
        return;
    }

    // Start TWAI driver
    if (twai_start() == ESP_OK)
    {
        ESP_LOGI("CAN", "CAN Driver started");
    }
    else
    {
        ESP_LOGE("CAN", "Failed to start driver\n");
        return;
    }
}

// sends the message to the TWAI
bool write_can_message(twai_message_t message)
{
    if (twai_transmit(&message, 999) == ESP_OK)
    {
        // esp_restart();
        // ESP_LOGE("CAN", "Sent CAN message: %03X %02X %02X %02X %02X", message.identifier, message.data[0], message.data[1], message.data[2], message.data[3]);
        return true;
    }
    else
    {
        ESP_LOGE("CAN", "Failed to send CAN message");
        return false;
    }
}
