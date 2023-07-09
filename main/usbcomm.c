#include "usbcomm.h"
#include "slcan.h"
#include "driver/usb_serial_jtag.h"
#include "esp_vfs_usb_serial_jtag.h"

void init_usbcomm(void)
{

    xTaskCreate(usbcomm_rx_task, "usbcomm_rx_task", 4096, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(usbcomm_tx_task, "usbcomm_tx_task", 4096, NULL, configMAX_PRIORITIES, NULL);
}

// task to handle outputting to the serial port
void usbcomm_tx_task(void *pvParameter)
{

    while (1)
    {
        serial_message_t txmsg;
        if (xQueueReceive(serial_out_queue, (void *)&txmsg, portMAX_DELAY) == pdTRUE)
        {
            printf("%.*s", txmsg.len, txmsg.data);
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
    uint8_t *rxbf = (uint8_t *)malloc(64);
    // Define rx_buffer for serial in
    static uint8_t rx_store[64];
    ESP_LOGE("slcan", "slcan task started");
    while (1)
    {
        // read the serial port inputs for commands
        msgLen = usb_serial_jtag_read_bytes(rxbf, 64, 0);
        if (msgLen > 0)
        {
            // Define a variable to keep track of the starting index of the next message
            int nextMsgIndex = 0;

            // Store the message in case it is incomplete
            if (rxStoreLen + msgLen <= 64)
            {
                memcpy(rx_store + rxStoreLen, rxbf, msgLen);
                rxStoreLen += msgLen;
                for (int i = 0; i < rxStoreLen; i++)
                {
                    if (rx_store[i] == SLCAN_CR)
                    {
                        // Send the message to be processed
                        memcpy(rxmsg.data, rx_store + nextMsgIndex, i - nextMsgIndex);
                        rxmsg.len = i - nextMsgIndex;
                        // append_spiffs_file("/spiffs/SENDA.TXT", (char *)rxmsg.data);
                        xQueueSend(serial_in_queue, (void *)&rxmsg, portMAX_DELAY);
                        // Update the next message index
                        nextMsgIndex = i + 1;
                    }
                }

                // Move the remaining incomplete message to the beginning of the array
                int remainingMsgLen = rxStoreLen - nextMsgIndex;
                if (remainingMsgLen > 0)
                {
                    memmove(rx_store, rx_store + nextMsgIndex, remainingMsgLen);
                }

                // Update the length of the stored data
                rxStoreLen = remainingMsgLen;
            }

            else
            {
            }
        }
        else
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
}

void flush_output()
{
    fflush(stdout);
    fflush(stderr);
}