#include "slcan.h"

// define the queues
QueueHandle_t serial_in_queue;
QueueHandle_t serial_out_queue;

void slcan_ack()
{
    printf("\r");
    fflush(stdout);
}
void slcan_nack()
{
    printf("\a");
    fflush(stdout);
}

void slcan_init(void)
{

    can_init(500000);
}

// Define rx_buffer for serial in
static uint8_t rx_buffer[RX_BUF_SIZE];
static uint8_t rx_store[2 * RX_BUF_SIZE];

// slcan task
void slcan_task(void *pvParameters)
{

    int msgLen = 0;
    int rxStoreLen = 0;
    while (1)
    {
        // read the serial port
        msgLen = uart_read_bytes(UART_NUM_0, rx_buffer, RX_BUF_SIZE, 100);
        if (msgLen > 0)
        {
            // store the message in case it is incomplete
            if (rxStoreLen + msgLen <= (2 * RX_BUF_SIZE))
            {
                memcpy(rx_store + rxStoreLen, rx_buffer, msgLen);
                rxStoreLen += msgLen;
            }
            else
            {
                // error
            }
            // look for the end of the message
            for (int i = 0; i < rxStoreLen; i++)
            {
                if (rx_store[i] == SLCAN_CR)
                {
                    // if the message is complete and the buffer is empty, send it to the queue
                    // and clear the buffer
                    if (i == rxStoreLen - 1)
                    {
                        // send the message to the queue
                        xQueueSend(serial_in_queue, rx_store, 0);
                        // clear the message from the store
                        memset(rx_store, 0, rxStoreLen);
                        rxStoreLen = 0;
                        break;
                    }
                    else // if the buffer is not empty, send the message to the queue and shift the buffer
                    {
                        xQueueSend(serial_in_queue, rx_store, i + 1);
                        // store the rest of the message, shifted to the beginning of the buffer
                        memcpy(rx_store, rx_store + i + 1, rxStoreLen - i - 1);
                        rxStoreLen = rxStoreLen - i - 1;
                        break;
                    }
                }
            }
        }

        // process any messages in the can receive queue
        if (uxQueueMessagesWaiting(can_receive_queue) > 0)
        {
            // get the message from the queue
            twai_message_t message;
            xQueueReceive(can_receive_queue, &message, 0);
            // send the message to be processed into slcan format
            slcan_receiveFrame(message);
        }

        // process any messages in the serial out queue
        //  if anything in queue, send to the serial port
        if (uxQueueMessagesWaiting(serial_out_queue) > 0)
        {
            // get the message from the queue
            uint8_t message[2 * RX_BUF_SIZE];
            memset(message, 0, 2 * RX_BUF_SIZE);
            xQueueReceive(serial_out_queue, message, 0);
            // send the message to the serial port
            uart_write_bytes(UART_NUM_0, (const char *)message, strlen((const char *)message));
            slcan_ack();
        }

        // process any messages in the serial in queue
        // if anything in queue, send to the process function
        if (uxQueueMessagesWaiting(serial_in_queue) > 0)
        {
            // get the message from the queue
            uint8_t message[2 * RX_BUF_SIZE];
            memset(message, 0, 2 * RX_BUF_SIZE);
            xQueueReceive(serial_in_queue, message, 0);
            // process the message
            processSlCommand(message);
        }
    }
}

void slcan_close(void)
{
    printf("close driver");
    busIsRunning = false;
    vTaskSuspend(readHandle);
    twai_stop();
    twai_driver_uninstall();
}
// Lookup table to map ASCII characters to their corresponding hexadecimal values
const uint8_t ascii_hex_lookup[256] =
    {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['A'] = 10,
        ['B'] = 11,
        ['C'] = 12,
        ['D'] = 13,
        ['E'] = 14,
        ['F'] = 15,
};

int asciiToHex(uint8_t ascii_hex_digit)
{
    // Look up the hexadecimal value in the lookup table
    return ascii_hex_lookup[ascii_hex_digit];
}

// parses and sends a frame given message from slcan
void send_can(uint8_t *bytes)
{
    twai_message_t msg;
    uint32_t frameID = 0;
    // gets the frame id from the ascii hex codes
    for (int i = 0; i < 3; i++)
    {
        frameID = (frameID << 4) | (asciiToHex(bytes[i + 1]));
    }
    if (frameID != 0)
    {
        msg.identifier = frameID;
        msg.flags = 0;

        // gets message length
        int frameLen = asciiToHex(bytes[SLCAN_FRAME_LEN_OFFSET]);

        // checks if the frame is a standard frame
        if (frameLen <= 8)
        {
            msg.data_length_code = frameLen;
            // gets the data from the ascii hex codes
            for (int i = 0; i < frameLen; i++)
            {
                msg.data[i] = (asciiToHex(bytes[SLCAN_FRAME_DATA_OFFSET + i * 2]) << 4 | asciiToHex(bytes[SLCAN_FRAME_DATA_OFFSET + i * 2 + 1]));
            }

            // sends the frame
            if (write_can_message(msg) != true)
            {
                // print noack or something
                ESP_LOGE("SLCAN", "Failed to send CAN message");
            };
        }
        else
        {
        }
    }
}
// Create a buffer to hold the string representation of the CAN frame data
#define CAN_FRAME_BUFFER_SIZE 32
char can_frame_buffer[CAN_FRAME_BUFFER_SIZE];
// Process a received CAN frame into slcan format
void slcan_receiveFrame(twai_message_t message)
{
    // Create a string representation of the CAN frame data using snprintf
    int len = snprintf(can_frame_buffer, CAN_FRAME_BUFFER_SIZE, "t%03X%01X", message.identifier, message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++)
    {
        len += snprintf(can_frame_buffer + len, CAN_FRAME_BUFFER_SIZE - len, "%02X", message.data[i]);
    }

    // Print the CAN frame data to the log
    xQueueSend(serial_out_queue, can_frame_buffer, 0);

    // slcan_ack();
}

void setFilter(uint8_t *bytes)
{
    uint32_t frameID = 0;
    // gets the frame id from the ascii hex codes
    for (int i = 0; i < 3; i++)
    {
        // printf("id: %02X", bytes[i]);
        frameID = (frameID << 4) | (asciiToHex(bytes[i + 1]));
    }
}

void processSlCommand(uint8_t *bytes)
{
    // printf("process sl command: %02X", bytes[0]);
    switch ((char)bytes[0])
    {
    case 'O':
        slcan_init();
        vTaskResume(readHandle);
        break;
    case 'C':
        slcan_close();
        slcan_ack();
        break;
    case 't':
        send_can(bytes);
        slcan_ack();
        break;
    case 'T':
        send_can(bytes);
        slcan_ack();
        break;
        /*
        case 'W':
        break;
        case 'm':
        break;
        case 'M':
        break;
        case 'r':
        break;
        case 'R':
        break;
        case 'V':
        break;
        case 'N':
        break;
        case 'U':
        break;
        case 'Z':
        break;
        case 'S':
        break;
        case 's':
        break;
        case 'F':
        break;
        case 'L':
        break;
        case 'A':
            break;*/
    }
}
