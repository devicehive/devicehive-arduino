#include <DeviceHive.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

// LedCube device registration data
const char *REG_DATA = "{"
    "id:\"f4b07b4d-5b02-47e7-bd9b-c8d56e0cfdd1\","
    "key:\"Arduino_LED\","
    "name:\"Arduino LED\","
    "deviceClass:{"
        "name:\"Arduino_LED\","
        "version:\"1.0\"},"
    "equipment:[{code:\"led\",name:\"led\",type:\"led\"}],"
    "commands:["
        "{intent:256,name:\"set\",params:u8}"
    "],"
    "notifications:["
        "{intent:257,name:\"button\",params:u8}"
    "]"
"}";

InputMessage rx_msg; // received message
const int LED_PIN = 13;

void setLedState(int state)
{
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void sendButtonState(int state)
{
    OutputMessage tx_msg(257);
    tx_msg.putUInt8(state);
    DH.write(tx_msg);
}

/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
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


            case 256:
            {
                const uint32_t cmd_id = rx_msg.getUInt32();
                const uint8_t state = rx_msg.getUInt8();

                setLedState(state);
                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);

                sendButtonState(128);
            } break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
