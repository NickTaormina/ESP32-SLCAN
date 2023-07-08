#include "can.h"

// define the queues
// QueueHandle_t can_send_queue;
// QueueHandle_t can_receive_queue;
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "stdio.h"
#include "string.h"

void can_task(void *pvParameters)
{
    ESP_LOGE("CAN", "CAN task started");
    twai_message_t *receiveMsg;

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
            vTaskDelay(50 / portTICK_RATE_MS);
        }
    }

    // Continuously check for received messages
    uint8_t buffer[30];

    while (1)
    {
        // Receive CAN messages
        receiveMsg = (twai_message_t *)malloc(sizeof(twai_message_t));

        if (receiveMsg == NULL)
        {
            ESP_LOGE("CAN", "Failed to allocate memory for receiveMsg");
            continue; // Skip processing this message and proceed to the next iteration
        }

        if (twai_receive(receiveMsg, 1 / portTICK_PERIOD_MS) == ESP_OK)
        {
            // Process the received message and send it to the queue
            if (receiveMsg->identifier == 0x7E8)
            {
                ESP_LOGE("CAN", "Received message from ECU: %02X%02X%02X%02X%02X%02X%02X%02X", receiveMsg->data[0], receiveMsg->data[1], receiveMsg->data[2], receiveMsg->data[3], receiveMsg->data[4], receiveMsg->data[5], receiveMsg->data[6], receiveMsg->data[7]);
            }
            if (1)
            {
                // Write the data into the buffer using snprintf
                int len = snprintf((char *)buffer, sizeof(buffer),
                                   "t%03X%01X%02X%02X%02X%02X%02X%02X%02X%02X\r",
                                   receiveMsg->identifier, receiveMsg->data_length_code,
                                   receiveMsg->data[0], receiveMsg->data[1], receiveMsg->data[2],
                                   receiveMsg->data[3], receiveMsg->data[4], receiveMsg->data[5],
                                   receiveMsg->data[6], receiveMsg->data[7]);

                if (len >= sizeof(buffer) || len > 64)
                {
                    ESP_LOGE("CAN", "Buffer overflow detected");
                    // Handle buffer overflow error, such as logging an error message
                }
                else
                {
                    serial_message_t txmsg;
                    txmsg.len = len;
                    memcpy(txmsg.data, buffer, len);
                    if (xQueueSend(serial_out_queue, &txmsg, portMAX_DELAY) != pdPASS)
                    {
                        ESP_LOGE("CAN", "Failed to send message to the queue");
                        // Handle queue send failure, such as logging an error message or taking recovery action
                    }
                }
            }
        }
        else
        {
            // Handle receive failure, such as logging an error message or taking recovery action
        }

        free(receiveMsg);
    }
}

// opens the twai interface and starts the driver with the specified speed
void open_can_interface()
{
    ESP_LOGI("MAIN", "Initializing CAN bus");
    if (!speed_set) // !speed_set
    {
        // we cant set up the speed
        return;
    }
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NO_ACK);
    g_config.rx_queue_len = 500;
    twai_timing_config_t bspeed = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_LOGI("MAIN", "CAN configs initialized");
    // Initialize CAN module
    if (twai_driver_install(&g_config, &bspeed, &f_config) == ESP_OK)
    {
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
    else
    {
        // Handle error
    }
}

void setup_speed(char speed_code)
{
    switch (speed_code)
    {
    case '0':
        bus_speed = (twai_timing_config_t){.brp = 400, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '1':
        bus_speed = (twai_timing_config_t){.brp = 200, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '2':
        bus_speed = (twai_timing_config_t){.brp = 80, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '3':
        bus_speed = (twai_timing_config_t){.brp = 40, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '4':
        bus_speed = (twai_timing_config_t){.brp = 32, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '5':
        bus_speed = (twai_timing_config_t){.brp = 16, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '6':
        bus_speed = (twai_timing_config_t){.brp = 8, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '7':
        bus_speed = (twai_timing_config_t){.brp = 4, .tseg_1 = 16, .tseg_2 = 8, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    case '8':
        bus_speed = (twai_timing_config_t){.brp = 4, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false};
        speed_set = true;
        break;
    default:
        // Handle error
        speed_set = false;
        return;
    }
}

// sends the message to the TWAI
bool write_can_message(twai_message_t message)
{
    if (twai_transmit(&message, 10) == ESP_OK)
    {
        return true;
    }
    else
    {
        ESP_LOGE("CAN", "Failed to send CAN message");
        return false;
    }
}
