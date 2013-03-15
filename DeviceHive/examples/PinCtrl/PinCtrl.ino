#include <DeviceHive.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

// device registration data
const char *REG_DATA = "{"
    "id:'03684ca0-6dee-46d2-98b7-06f2cd079f5c',"
    "key:'5b2e34b4-995a-4d66-8b0f-392232bc1563',"
    "name:'Arduino Pins',"
    "deviceClass:{"
        "name:'Arduino',"
        "version:'1.0'},"
    "equipment:[],"
    "commands:["
        "{intent:1001,name:'getPinMode',params:[u8]},"
        "{intent:1002,name:'setPinMode',params:{pin:u8,mode:u8}},"
        "{intent:1003,name:'pinRead',params:[u8]},"
        "{intent:1004,name:'pinWrite',params:{pin:u8,value:u16}}"
    "],"
    "notifications:["
        "{intent:2001,name:'pinMode',params:{pin:u8,mode:u8}},"
        "{intent:2003,name:'pinRead',params:{pin:u8,value:u16}}"
    "]"
"}";

enum PinMode
{
    MODE_DIGITAL_OUTPUT         = 11,
    MODE_DIGITAL_INPUT          = 12,
    MODE_DIGITAL_INPUT_PULLUP   = 13,

    MODE_ANALOG_OUTPUT          = 21,
    MODE_ANALOG_INPUT           = 22,
    MODE_ANALOG_INPUT_PULLUP    = 23,

    N_PINS = 14
};

uint8_t pin_mode[N_PINS];
InputMessage rx_msg; // received message

void sendPinMode(int pin)
{
    OutputMessage tx_msg(2001);
    tx_msg.putByte(pin);
    tx_msg.putByte(pin_mode[pin]);
    DH.write(tx_msg);
}


void sendPinRead(int pin)
{
    int val = 0;
    switch (pin_mode[pin])
    {
        case MODE_DIGITAL_INPUT:
        case MODE_DIGITAL_INPUT_PULLUP:
            val = digitalRead(pin);
            break;

        case MODE_ANALOG_INPUT:
        case MODE_ANALOG_INPUT_PULLUP:
            val = analogRead(pin);
            break;

        default:
            return;
    }

    OutputMessage tx_msg(2003);
    tx_msg.putByte(pin);
    tx_msg.putShort(val);
    DH.write(tx_msg);
}


/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    for (int i = 0; i < N_PINS; ++i)
        pin_mode[i] = MODE_DIGITAL_OUTPUT;
    Serial.begin(115200);

    DH.begin(Serial);
    DH.writeRegistrationResponse(REG_DATA);
}


/**
 * Loop procedure is called continuously.
 */
void loop(void)
{
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;


            case 1001:   // "getPinMode" - send pin mode notifications
            {
                const long cmd_id = rx_msg.getUInt32();
                const int count = rx_msg.getUInt16();

                if (0 < count)
                {
                    for (int i = 0; i < count; ++i)
                    {
                        const int pin = rx_msg.getUInt8();
                        if (0 <= pin && pin < N_PINS)
                            sendPinMode(pin);
                    }
                }
                else
                {
                    for (int i = 0; i < N_PINS; ++i)
                        sendPinMode(i);
                }

                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;


            case 1002:   // "setPinMode" - set a pin mode
            {
                const long cmd_id = rx_msg.getUInt32();
                const int pin = rx_msg.getUInt8();
                const int mode = rx_msg.getUInt8();

                if (0 <= pin && pin < N_PINS)
                {
                    pin_mode[pin] = mode;
                    sendPinMode(pin);

                    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
                }
                else
                    DH.writeCommandResult(cmd_id, CMD_STATUS_FAILED, "Invalid pin");
            } break;


            case 1003:   // "pinRead" - send pin read value notification
            {
                const long cmd_id = rx_msg.getUInt32();
                const int count = rx_msg.getUInt16();

                if (0 < count)
                {
                    for (int i = 0; i < count; ++i)
                    {
                        const int pin = rx_msg.getUInt8();
                        if (0 <= pin && pin < N_PINS)
                            sendPinRead(pin);
                    }
                }
                else
                {
                    for (int i = 0; i < N_PINS; ++i)
                        sendPinRead(i);
                }

                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;


            case 1004:   // "pinWrite" - write to a pin
            {
                const long cmd_id = rx_msg.getUInt32();
                const int pin = rx_msg.getUInt8();
                const int val = rx_msg.getUInt16();

                if (0 <= pin && pin < N_PINS)
                {
                    switch (pin_mode[pin])
                    {
                        case MODE_DIGITAL_OUTPUT:
                            digitalWrite(pin, val);
                            break;

                        case MODE_ANALOG_OUTPUT:
                            analogWrite(pin, val);
                            break;
                    }

                    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
                }
                else
                    DH.writeCommandResult(cmd_id, CMD_STATUS_FAILED, "Invalid pin");
            } break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
