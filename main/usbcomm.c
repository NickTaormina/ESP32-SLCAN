#include "usbcomm.h"
#include "slcan.h"

void init_usbcomm(void)
{

    xTaskCreate(usbcomm_rx_task, "usbcomm_rx_task", 3072, NULL, 1, NULL);
    xTaskCreate(usbcomm_tx_task, "usbcomm_tx_task", 3072, NULL, 1, NULL);
}

// task to handle outputting to the serial port
void usbcomm_tx_task(void *pvParameter)
{

    while (1)
    {
        serial_message_t txmsg;
        if (xQueueReceive(serial_out_queue, (void *)&txmsg, portMAX_DELAY) == pdTRUE)
        {
            printf("%s", txmsg.data);
            flush_output();
        }
    }
}

void usbcomm_rx_task(void *pvParameter)
{
    ESP_LOGE("slcan", "slcan task started");

    serial_message_t rxmsg;
    int msgLen = 0;
    int rxStoreLen = 0;
    uint8_t *rxbf = (uint8_t *)malloc(512);
    // Define rx_buffer for serial in
    static uint8_t rx_store[2 * 256];
    ESP_LOGE("slcan", "slcan task started");
    while (1)
    {
        // read the serial port inputs for commands
        msgLen = usb_serial_jtag_read_bytes(rxbf, 256, 0);
        if (msgLen > 0)
        {
            ESP_LOGE("slcan", "got bytes");
            //  store the message in case it is incomplete
            if (rxStoreLen + msgLen <= (2 * 256))
            {
                memcpy(rx_store + rxStoreLen, rxbf, msgLen);
                rxStoreLen += msgLen;
                for (int i = 0; i < rxStoreLen; i++)
                {
                    if (rx_store[i] == SLCAN_CR)
                    {
                        // send the message to be processed
                        // processSlCommand(rx_store);
                        ESP_LOGE("slcan", "got command");
                        xQueueSend(serial_in_queue, (void *)&rxmsg, portMAX_DELAY);
                        //   clear the message from the store
                        memset(rx_store, 0, rxStoreLen);
                        rxStoreLen = 0;
                        ESP_LOGE("slcan", "sent command");
                    }
                }
            }
            else
            {
            }
        }
        vTaskDelay(2);
    }
}

void flush_output()
{
    fflush(stdout);
    fflush(stderr);
}