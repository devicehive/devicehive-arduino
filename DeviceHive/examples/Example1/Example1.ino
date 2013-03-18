#include <DeviceHive.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

// device registration data
// intent numbers should be greater than 255!
// please refer to http://www.devicehive.com/binary/#SystemMessages/RegisterJson for complete syntax of registration info
const char *REG_DATA = "{"
    "id:'f4b07b4d-5b02-47e7-bd9b-c8d56e0cfdd1',"
    "key:'Arduino_LED',"
    "name:'Arduino LED',"
    "deviceClass:{"
        "name:'Arduino_LED',"
        "version:'1.0'},"
    "equipment:[{code:'led',name:'led',type:'led'}],"
    "commands:["
        "{intent:1000,name:'set',params:u8},"
        "{intent:1001,name:'blink',params:{on:u16,off:u16,count:u8}}"
    "],"
    "notifications:["
        "{intent:2000,name:'button',params:u8}"
    "]"
"}";

const int BTN_PIN = 12;
const int LED_PIN = 13;

// VERY IMPORTANT: the order and types of fields in struct
// should be exactly the same as those defined in registration data
// {\"intent\":1001,\"name\":\"blink\",\"params\":{\"on\":\"u16\",\"off\":\"u16\",\"count\":\"u8\"}}

struct BlinkParam
{
    unsigned short on;
    unsigned short off;
    byte count;
};

void setLedState(int state)
{
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void sendButtonState(int state)
{
    OutputMessage tx_msg(2000);
    tx_msg.putByte(state);
    DH.write(tx_msg);
}


InputMessage rx_msg; // received message
int old_btn_state;

/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP); // ... so you don't need a pull-up resistor
    old_btn_state = digitalRead(BTN_PIN);

    Serial.begin(115200);
    DH.begin(Serial);
    DH.writeRegistrationResponse(REG_DATA);
}


/**
 * Loop procedure is called continuously.
 */
void loop(void)
{
    // ned button state change notifications
    const int btn_state = digitalRead(BTN_PIN);
    if (btn_state != old_btn_state)
    {
        sendButtonState(btn_state);
        old_btn_state = btn_state;
    }

    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;


            case 1000: // "set" - sets the LED state
            {
                const long cmd_id = rx_msg.getLong();
                const byte state = rx_msg.getByte();

                setLedState(state);
                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;


            case 1001: // "blink" - blinks the LED
            {
                const long cmd_id = rx_msg.getLong();
                BlinkParam params = rx_msg.get<BlinkParam>();
                // TODO: check for very long delays?

                for (int i = 0; i < params.count; ++i)
                {
                    setLedState(1);     // ON
                    delay(params.on);
                    setLedState(0);     // OFF
                    delay(params.off);
                }

                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
