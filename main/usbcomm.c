#include "usbcomm.h"
#include "slcan.h"

void init_usbcomm(void)
{
    usb_serial_jtag_driver_config_t usb_serial_jtag_driver_config = {
        .rx_buffer_size = 256,
        .tx_buffer_size = 1,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_driver_config));
}

// task to handle outputting to the serial port
void usbcomm_tx_task(void *pvParameter)
{
    serial_message_t txmsg;
    while (1)
    {
        xQueueReceive(serial_out_queue, (void *)&txmsg, portMAX_DELAY);
        printf("%s", txmsg.data);
        flush_output();
    }
}
#define SLCAN_CR2 '\r'

void usbcomm_rx_task(void *pvParameter)
{
    int msgLen = 0;
    int rxStoreLen = 0;
    ESP_LOGE("slcan", "slcan task started");

    uint8_t *rxbf = (uint8_t *)malloc(512);
    serial_message_t rxmsg;
    int msgLen = 0;
    int rxStoreLen = 0;
    uint8_t *rxbf = (uint8_t *)malloc(512);
    // Define rx_buffer for serial in
    static uint8_t rx_store[2 * 256];
    while (1)
    {
        // read the serial port inputs for commands

        msgLen = usb_serial_jtag_read_bytes(rxbf, 512, 0);
        if (msgLen > 0)
        {
            //  store the message in case it is incomplete
            if (rxStoreLen + msgLen <= (2 * 256))
            {
                memcpy(rx_store + rxStoreLen, rxbf, msgLen);
                rxStoreLen += msgLen;
                for (int i = 0; i < rxStoreLen; i++)
                {
                    if (rx_store[i] == SLCAN_CR2)
                    {
                        // send the message to be processed
                        processSlCommand(rx_store);
                        //   clear the message from the store
                        memset(rx_store, 0, rxStoreLen);
                        rxStoreLen = 0;
                    }
                }
            }
            else
            {
            }
        }
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void flush_output()
{
    fflush(stdout);
    fflush(stderr);
}