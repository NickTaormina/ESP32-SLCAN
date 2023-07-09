#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "slcan.h"
#include <stdio.h>
#include "can.h"
#include "hal/usb_serial_jtag_ll.h"
#include "driver/usb_serial_jtag.h"
#include "usbcomm.h"
#include "flash_handler.h"

// for can messages as a whole
void tx_task()
{
    serial_message_t buffer;
    while (1)
    {
        if (xQueueReceive(serial_in_queue, &buffer, portMAX_DELAY) == pdTRUE)
        {
            processSlCommand(buffer.data);
        }
    }
}

// for can messages as a whole
int void rx_task()
{
    ESP_LOGE("CAN", "CAN task started");
    twai_message_t receiveMsg;

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

        if (twai_receive(&receiveMsg, portMAX_DELAY) == ESP_OK)
        {
            // Process the received message and send it to the queue
            if (1)
            {
                // Write the data into the buffer using snprintf
                int len = snprintf((char *)buffer, sizeof(buffer),
                                   "t%03X%01X%02X%02X%02X%02X%02X%02X%02X%02X\r",
                                   receiveMsg.identifier, receiveMsg.data_length_code,
                                   receiveMsg.data[0], receiveMsg.data[1], receiveMsg.data[2],
                                   receiveMsg.data[3], receiveMsg.data[4], receiveMsg.data[5],
                                   receiveMsg.data[6], receiveMsg.data[7]);

                if (len >= sizeof(buffer) || len > 30)
                {
                    ESP_LOGE("CAN", "Buffer overflow detected");
                    // Handle buffer overflow error, such as logging an error message
                }
                else
                {
                    // serial_message_t txmsg;
                    // txmsg.len = len;
                    // memcpy(txmsg.data, buffer, len);
                    /*if (xQueueSend(serial_out_queue, &txmsg, portMAX_DELAY) != pdPASS)
                    {
                        // ESP_LOGE("CAN", "Failed to send message to the queue");
                        //  Handle queue send failure, such as logging an error message or taking recovery action
                    }*/
                    printf("%s", buffer);
                }
            }
        }
        else
        {
            // Handle receive failure, such as logging an error message or taking recovery action
        }
    }
}

void app_main()
{
    // init the usb driver
    usb_serial_jtag_driver_config_t usb_serial_jtag_driver_config = {
        .rx_buffer_size = 512,
        .tx_buffer_size = 512,
    };
    esp_err_t err = usb_serial_jtag_driver_install(&usb_serial_jtag_driver_config);
    if (err != ESP_OK)
    {
        printf("Error installing USB serial JTAG driver: %s\n", esp_err_to_name(err));
    }
    vTaskDelay(5);
    // init storage
    spiffs_init();
    setvbuf(stdout, NULL, _IONBF, 0);
    setbuf(stdout, NULL);
    // read_spiffs_file_to_buffer("/spiffs/SENDA.TXT");

    // set up queues
    can_send_queue = xQueueCreate(20, 2 * sizeof(twai_message_t));
    can_receive_queue = xQueueCreate(20, 2 * sizeof(twai_message_t));
    serial_in_queue = xQueueCreate(50, 2 * sizeof(serial_message_t));
    serial_out_queue = xQueueCreate(50, 2 * sizeof(serial_message_t));
    vTaskDelay(5);

    // start the usb tasks
    init_usbcomm();
    vTaskDelay(5);

    // Create the tx and rx tasks
    xTaskCreate(tx_task, "tx_task", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(rx_task, "rx_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    ESP_LOGI("MAIN", "Setup finished");
}
