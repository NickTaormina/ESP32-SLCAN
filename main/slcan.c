#include "slcan.h"

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
    open_can_interface();
}

void slcan_close(void)
{
    printf("close driver");
    busIsRunning = false;
    // vTaskSuspend(readHandle);
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
    if (bytes == NULL)
    {
        return;
    }
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
        if (frameLen <= 8 && frameLen > 0)
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
                slcan_nack();
                // ESP_LOGE("SLCAN", "Failed to send CAN message");
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
    //  Create a string representation of the CAN frame data using snprintf
    int len = snprintf(can_frame_buffer, CAN_FRAME_BUFFER_SIZE, "t%03X%01X", message.identifier, message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++)
    {
        len += snprintf(can_frame_buffer + len, CAN_FRAME_BUFFER_SIZE - len, "%02X", message.data[i]);
    }

    // Print the CAN frame data to the log
    printf("%s\r", can_frame_buffer);
    fflush(stdout);
}

void setFilter(uint8_t *bytes)
{
    uint32_t frameID = 0;
    // gets the frame id from the ascii hex codes
    for (int i = 0; i < 3; i++)
    {
        frameID = (frameID << 4) | (asciiToHex(bytes[i + 1]));
    }
}

void processSlCommand(uint8_t *bytes)
{
    switch ((char)bytes[0])
    {
    case 'O':
        if (bytes[1] != '\0' && isdigit((char)bytes[1]))
        {
            setup_speed(bytes[1]);
        }
        if (open_can_interface())
        {
            slcan_ack();
        }
        else
        {
            slcan_nack();
        }
        break;
    case 'C':
        // slcan_close();
        slcan_ack();
        break;
    case 't':
        send_can(bytes);
        slcan_ack();
        break;
    case 'T':
        // send_can(bytes);
        // slcan_ack();
        break;
    case 'S':
        if (setup_speed(bytes[1]))
        {
            slcan_ack();
        }
        else
        {
            slcan_nack();
        }
        break;
    case 's': // todo: implement this
        if (bytes == NULL)
        {
            printf("\a"); // BELL (Ascii 7) for ERROR
            return;
        }

        // Extract BTR0 and BTR1 values from the input array
        uint8_t btr0 = (asciiToHex(bytes[1]) << 4) | asciiToHex(bytes[2]);
        uint8_t btr1 = (asciiToHex(bytes[3]) << 4) | asciiToHex(bytes[4]);

        // Calculate bitrate based on CAN232 formula
        uint32_t bitrate = 16000000 / ((btr0 + 1) * (btr1 + 1));

        // Print the corresponding bitrate
        printf("Setup with BTR0/BTR1 CAN bit-rates where xx and yy is a hex value.\r\n"
               "This command is only active if the CAN channel is closed.\r\n"
               "xx BTR0 value in hex\r\n"
               "yy BTR1 value in hex\r\n"
               "Example: s031C\r\n"
               "Setup CAN with BTR0=0x%02X & BTR1=0x%02X\r\n"
               "which equals to %u Kbit.\r\n"
               "Returns: CR (Ascii 13) for OK or BELL (Ascii 7) for ERROR.\r\n",
               btr0, btr1, bitrate);
        printf("\r\n"); // CR (Ascii 13) for OK
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
    default:
        break;
    }
}
