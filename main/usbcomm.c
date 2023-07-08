#include "usbcomm.h"

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
        xQueueReceive(*usbcomm_tx_queue, (void *)&txmsg, portMAX_DELAY);
        printf("%s", txmsg.data);
    }
}

void usbcomm_rx_task(void *pvParameter)
{
    serial_message_t rxmsg;
    int msgLen = 0;
    int rxStoreLen = 0;
    uint8_t *rxbf = (uint8_t *)malloc(512);
    while (1)
    {
    }
}