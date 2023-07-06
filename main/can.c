#include "can.h"

// define the queues
QueueHandle_t can_send_queue;
QueueHandle_t can_receive_queue;

void can_task(void *pvParameters)
{
    twai_message_t receiveMsg;
    twai_message_t sendMsg;

    // continuously check for received messages
    while (1)
    {
        // ESP_LOGI("CAN", "CAN task running");
        //  receive can messages
        if (twai_receive(&receiveMsg, portMAX_DELAY) == ESP_OK)
        {
            // push the received message to the queue
            // ESP_LOGI("CAN", "Received message");
            push_to_queue(can_receive_queue, &receiveMsg, 10);
            // ESP_LOGI("CAN", "Pushed message to queue");
        }
        else
        {
            ESP_LOGE("CAN", "No message received");
            // send the slcan noack message (push bad frame to the queue or something)
        }
        // ESP_LOGI("CAN", "Checking for messages to send");
        //  pull messages from the queue and send them
        if (pull_from_queue(can_send_queue, &sendMsg, 5))
        {
            if (write_can_message(sendMsg))
            {
                // it worked
            }
            else
            {
                // it didn't work
            }
        }
        // ESP_LOGI("CAN", "Done checking for messages to send");
        vTaskDelay(1);
    }
}

// todo: implement bitrate switching
void can_init(int bitrate)
{
    ESP_LOGI("MAIN", "Initializing CAN bus");
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 500;
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
    if (twai_transmit(&message, portMAX_DELAY) == ESP_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}
