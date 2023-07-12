#include "can.h"

// define the queues
// QueueHandle_t can_send_queue;
// QueueHandle_t can_receive_queue;
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "stdio.h"
#include "string.h"

// opens the twai interface and starts the driver with the specified speed
bool open_can_interface()
{
    if (!speed_set)
    {
        // we cant set up the speed
        return false;
    }
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NO_ACK);
    g_config.rx_queue_len = 500;
    g_config.tx_queue_len = 10;
    twai_timing_config_t bspeed = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    //  Initialize CAN module
    if (twai_driver_install(&g_config, &bspeed, &f_config) == ESP_OK)
    {
        // Start TWAI driver
        if (twai_start() == ESP_OK)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool setup_speed(char speed_code)
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
        break;
    }
    return speed_set;
}
#include "flash_handler.h"
// sends the message to the TWAI
bool write_can_message(twai_message_t message)
{
    if (twai_transmit(&message, pdMS_TO_TICKS(100)))
    {
        return true;
    }
    return false;
}
