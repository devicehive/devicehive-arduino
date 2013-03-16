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
        "{intent:2003,name:'pinRead',params:{pin:u8,value:u16}},"
        "{intent:2004,name:'pinWrite',params:{pin:u8,value:u16}},"
        "{intent:2010,name:'reset',params:null}"
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

struct PinState
{
    uint8_t mode;
    //uint16_t last_read;
    //unsigned int auto_read;
};

PinState pin_state[N_PINS];
InputMessage rx_msg; // received message

void notifyPinMode(int pin)
{
    OutputMessage tx_msg(2001);
    tx_msg.putByte(pin);
    tx_msg.putByte(pin_state[pin].mode);
    DH.write(tx_msg);
}


void notifyPinRead(int pin)
{
    int val = 0;
    switch (pin_state[pin].mode)
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
            // don't send notifications
            // for unsupported pin modes
            return;
    }

    OutputMessage tx_msg(2003);
    tx_msg.putByte(pin);
    tx_msg.putShort(val);
    DH.write(tx_msg);
}


void notifyPinWrite(int pin, int val)
{
    OutputMessage tx_msg(2004);
    tx_msg.putByte(pin);
    tx_msg.putShort(val);
    DH.write(tx_msg);
}


void notifyReset()
{
    OutputMessage tx_msg(2010);
    DH.write(tx_msg);
}


// send pin mode notifications
void handleGetPinMode(InputMessageEx &msg)
{
    const long cmd_id = msg.getUInt32();
    const int count = msg.getUInt16();

    if (0 < count)
    {
        // notify requested pins
        for (int i = 0; i < count; ++i)
        {
            const int pin = msg.getUInt8();
            if (0 <= pin && pin < N_PINS)
                notifyPinMode(pin);
        }
    }
    else
    {
        // notify all pins
        for (int i = 0; i < N_PINS; ++i)
            notifyPinMode(i);
    }

    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
}


// set a pin mode
void handleSetPinMode(InputMessageEx &msg)
{
    const long cmd_id = msg.getUInt32();
    const int pin = msg.getUInt8();
    const int mode = msg.getUInt8();

    if (!(0 <= pin && pin < N_PINS))
    {
        DH.writeCommandResult(cmd_id,
            CMD_STATUS_FAILED,
            "Invalid pin");
        return;
    }

    switch (mode)
    {
        case MODE_DIGITAL_OUTPUT:
        case MODE_ANALOG_OUTPUT:
            pinMode(pin, OUTPUT);
            break;

        case MODE_DIGITAL_INPUT:
        case MODE_ANALOG_INPUT:
            pinMode(pin, INPUT);
            break;

        case MODE_DIGITAL_INPUT_PULLUP:
        case MODE_ANALOG_INPUT_PULLUP:
            pinMode(pin, INPUT_PULLUP);
            break;

        default:
            DH.writeCommandResult(cmd_id,
                CMD_STATUS_FAILED,
                "Invalid mode");
            return;
    }

    pin_state[pin].mode = mode;
    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
    notifyPinMode(pin);
}


// send pin read value notification
void handlePinRead(InputMessageEx &msg)
{
    const long cmd_id = msg.getUInt32();
    const int count = msg.getUInt16();

    if (0 < count)
    {
        // notify requestd pins
        for (int i = 0; i < count; ++i)
        {
            const int pin = msg.getUInt8();
            if (0 <= pin && pin < N_PINS)
                notifyPinRead(pin);
        }
    }
    else
    {
        // notify all pins
        for (int i = 0; i < N_PINS; ++i)
            notifyPinRead(i);
    }

    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
}


// write to a pin
void handlePinWrite(InputMessageEx &msg)
{
    const long cmd_id = msg.getUInt32();
    const int pin = msg.getUInt8();
    const int val = msg.getUInt16();

    if (!(0 <= pin && pin < N_PINS))
    {
        DH.writeCommandResult(cmd_id,
            CMD_STATUS_FAILED,
            "Invalid pin");
        return;
    }

    switch (pin_state[pin].mode)
    {
        case MODE_DIGITAL_OUTPUT:
            digitalWrite(pin, val);
            break;

        case MODE_ANALOG_OUTPUT:
            analogWrite(pin, val);
            break;

        default:
            DH.writeCommandResult(cmd_id,
                CMD_STATUS_FAILED,
                "Invalid mode");
            return;
    }

    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
    notifyPinWrite(pin, val);
}


/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    for (int i = 0; i < N_PINS; ++i)
    {
        pin_state[i].mode = MODE_DIGITAL_OUTPUT;
        pinMode(i, OUTPUT);
        //pin_state[i].
    }
    Serial.begin(115200);

    DH.begin(Serial);
    DH.writeRegistrationResponse(REG_DATA);
    notifyReset();
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
                handleGetPinMode(rx_msg);
                break;

            case 1002:   // "setPinMode" - set a pin mode
                handleSetPinMode(rx_msg);
                break;

            case 1003:   // "pinRead" - send pin read value notification
                handlePinRead(rx_msg);
                break;

            case 1004:   // "pinWrite" - write to a pin
                handlePinWrite(rx_msg);
                break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
