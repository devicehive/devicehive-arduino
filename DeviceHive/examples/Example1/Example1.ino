#include <DeviceHive.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

// LedCube device registration data
// Intent numbers should be greater than 255
// Please refer to URL for complete syntax of registration info 
const char *REG_DATA = "{"
    "\"id\":\"f4b07b4d-5b02-47e7-bd9b-c8d56e0cfdd1\","
    "\"key\":\"Arduino_LED\","
    "\"name\":\"Arduino LED\","
    "\"deviceClass\":{"
        "\"name\":\"Arduino_LED\","
        "\"version\":\"1.0\"},"
    "\"equipment\":[{\"code\":\"led\",\"name\":\"led\",\"type\":\"led\"}],"
    "\"commands\":["
        "{\"intent\":1000,\"name\":\"set\",\"params\":\"u8\"},"
        "{\"intent\":1001,\"name\":\"blink\",\"params\":{\"on\":\"u16\",\"off\":\"u16\",\"count\":\"u8\"}}"
    "],"
    "\"notifications\":["
        "{\"intent\":2000,\"name\":\"button\",\"params\":\"u8\"}"
    "]"
"}";

InputMessage rx_msg; // received message
const int LED_PIN = 13;
const int BUTTON_PIN = 12;

int oldButtonState;

// VERY IMPORTANT: the order and types of fields in struct should be exactly the same as those
// defined in initialization string *REG_DATA
// {\"intent\":257,\"name\":\"blink\",\"params\":{\"on\":\"u16\",\"off\":\"u16\",\"count\":\"u8\"}}

struct BlinkParam
{
  short on;
  short off;
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

/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    Serial.begin(115200);
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // ... so you don't need a pull-up resistor
    
    oldButtonState = digitalRead(BUTTON_PIN);

    DH.begin(Serial);
    DH.writeRegistrationResponse(REG_DATA);
}

/**
 * Loop procedure is called continuously.
 */
void loop(void)
{
    int buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState != oldButtonState)
    {
      sendButtonState(buttonState);  
      oldButtonState = buttonState;
    }    
    
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;


            case 1000:
            {
                const long cmd_id = rx_msg.getLong();
                const byte state = rx_msg.getByte();

                setLedState(state);
                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
                break;
            } 
            case 1001:
            {
              const long cmd_id = rx_msg.getLong();
              
              BlinkParam p;
              
              rx_msg.get(&p, sizeof(p));
              
              for (int i = 0; i < p.count; i++)
              {
                setLedState(1);
                delay(p.on);
                setLedState(0);
                delay(p.off);  
              }
              
              DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
              break;
            }
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
