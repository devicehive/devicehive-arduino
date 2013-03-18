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

    N_PINS = NUM_DIGITAL_PINS
};

struct PinState
{
    uint8_t mode;
    int old_val;
    //unsigned int auto_read;
};

PinState pin_state[N_PINS];
InputMessage rx_msg; // received message


// send 'pinMode' notification
void notifyPinMode(int pin)
{
    OutputMessage tx_msg(2001);
    tx_msg.putByte(pin);
    tx_msg.putByte(pin_state[pin].mode);
    DH.write(tx_msg);
}


// send 'pinRead' notification
void notifyPinRead(int pin, int val)
{
    OutputMessage tx_msg(2003);
    tx_msg.putByte(pin);
    tx_msg.putShort(val);
    DH.write(tx_msg);
}

// read value and send 'pinRead' notification
void notifyPinRead(int pin)
{
    switch (pin_state[pin].mode)
    {
        case MODE_DIGITAL_INPUT:
        case MODE_DIGITAL_INPUT_PULLUP:
            notifyPinRead(pin,
                digitalRead(pin));
            break;

        case MODE_ANALOG_INPUT:
        case MODE_ANALOG_INPUT_PULLUP:
            notifyPinRead(pin,
                analogRead(pin));
            break;

        default:
            // don't send notifications
            // for unsupported pin modes
            break;
    }
}


// send 'pinWrite' notification
void notifyPinWrite(int pin, int val)
{
    OutputMessage tx_msg(2004);
    tx_msg.putByte(pin);
    tx_msg.putShort(val);
    DH.write(tx_msg);
}


// send 'reset' notification
void notifyReset()
{
    OutputMessage tx_msg(2010);
    // no payload
    DH.write(tx_msg);
}


// handle 'getPinMode' command - send 'pinMode' notifications back
void handleGetPinMode()
{
    const long cmd_id = rx_msg.getUInt32();
    const int count = rx_msg.getUInt16();

    if (0 < count)
    {
        // notify requested pins
        for (int i = 0; i < count; ++i)
        {
            const int pin = rx_msg.getUInt8();
            if (0 <= pin && pin < N_PINS)
                notifyPinMode(pin);
        }
    }
    else
    {
        // notify all pins
        for (int pin = 0; pin < N_PINS; ++pin)
            notifyPinMode(pin);
    }

    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
}


// handle 'setPinMode' command - set 'pinMode' notification back
void handleSetPinMode()
{
    const long cmd_id = rx_msg.getUInt32();
    const int pin = rx_msg.getUInt8();
    const int mode = rx_msg.getUInt8();

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
            pin_state[pin].old_val = digitalRead(pin);
            pinMode(pin, INPUT);
            break;

        case MODE_ANALOG_INPUT:
            pin_state[pin].old_val = analogRead(pin);
            pinMode(pin, INPUT);
            break;

        case MODE_DIGITAL_INPUT_PULLUP:
            pin_state[pin].old_val = digitalRead(pin);
            pinMode(pin, INPUT_PULLUP);
            break;

        case MODE_ANALOG_INPUT_PULLUP:
            pin_state[pin].old_val = analogRead(pin);
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
    notifyPinRead(pin);
}


// handle 'pinRead' command - send 'pinRead' notifications back
void handlePinRead()
{
    const long cmd_id = rx_msg.getUInt32();
    const int count = rx_msg.getUInt16();

    if (0 < count)
    {
        // notify requested pins
        for (int i = 0; i < count; ++i)
        {
            const int pin = rx_msg.getUInt8();
            if (0 <= pin && pin < N_PINS)
                notifyPinRead(pin);
        }
    }
    else
    {
        // notify all pins
        for (int pin = 0; pin < N_PINS; ++pin)
            notifyPinRead(pin);
    }

    DH.writeCommandResult(cmd_id,
        CMD_STATUS_SUCCESS,
        CMD_RESULT_OK);
}


// handle 'pinWrite' command - send 'pinWrite' notification back
void handlePinWrite()
{
    const long cmd_id = rx_msg.getUInt32();
    const int pin = rx_msg.getUInt8();
    const int val = rx_msg.getUInt16();

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


// send 'pinRead' notification for digital input pins (if changed)
void notifyInputPins()
{
    for (int pin = 0; pin < N_PINS; ++pin)
        switch (pin_state[pin].mode)
        {
            case MODE_DIGITAL_INPUT:
            case MODE_DIGITAL_INPUT_PULLUP:
            {
                const int val = digitalRead(pin);
                if (val != pin_state[pin].old_val)
                {
                    pin_state[pin].old_val = val;
                    notifyPinRead(pin, val);
                }
            } break;

            // TODO: use timeout or value hysteresis
            case MODE_ANALOG_INPUT:
            case MODE_ANALOG_INPUT_PULLUP:
            {
                const int val = analogRead(pin);
                if (val != pin_state[pin].old_val)
                {
                    pin_state[pin].old_val = val;
                    notifyPinRead(pin, val);
                }
            } break;
        }
}


/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    for (int pin = 0; pin < N_PINS; ++pin)
    {
        pin_state[pin].mode = MODE_DIGITAL_OUTPUT;
        pinMode(pin, OUTPUT);

        pin_state[pin].old_val = 0;
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
    notifyInputPins();

    // process input commands
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;

            case 1001: handleGetPinMode(); break;
            case 1002: handleSetPinMode(); break;
            case 1003: handlePinRead();    break;
            case 1004: handlePinWrite();   break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
