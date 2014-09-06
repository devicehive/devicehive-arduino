#include <DeviceHive.h>
#include <LiquidCrystal.h>
#include <Console.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS A2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const unsigned long pollInterval = 1000;
unsigned long lastPoll = 0;

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAILED       "Failed"
#define CMD_RESULT_OK           "OK"

const char *ALLJOYN_CHANNEL = "com.devicehive.demo.hack123";
InputMessage rx_msg; // received message

// device registration data
// intent numbers should be greater than 255!
// please refer to http://www.devicehive.com/binary/#SystemMessages/RegisterJson for complete syntax of registration info
// VERY IMPORTANT: the order and types of fields in struct
// should be exactly the same as those defined in registration data
// {\"intent\":1001,\"name\":\"blink\",\"params\":{\"on\":\"u16\",\"off\":\"u16\",\"count\":\"u8\"}}
char *REG_DATA = "{"
    "id:'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx',"
    "key:'Hack_DEMO',"
    "name:'AllJoyn Demo',"
    "deviceClass:{"
        "name:'Hack_DEMO_Class',"
        "version:'1.0'},"
    "equipment:[{code:'lcd',name:'lcd',type:'lcd'}],"
    "commands:["
        "{intent:1000,name:'set',params:str}"
    "],"
    "notifications:["
        "{intent:2000,name:'temperature',params:str}"
    "]"
"}";

void updateRegDataWithGuid(char * r)
{
  Process p;
  p.runShellCommand("/root/get_guid.sh");
  
  while (p.running());
  
  int i = 0;
  while ( (p.available() > 0) && (i < 36))
  {
    r[i + 5] = p.read();
    i++;
  }
}

float getTemp()
{
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);
}

void setLedState(int state)
{
    digitalWrite(13, state ? HIGH : LOW);
}

void setLcdText(char * text)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
}

void sendTemp(float temp)
{
    OutputMessage tx_msg(2000);
    char s[6];
    dtostrf(temp, 5, 2, s);
    tx_msg.putString(s);
    DH.write(tx_msg);
}

void sendAllJoynInfoResponse(void)
{
    OutputMessage tx_msg(30002);
    tx_msg.putString(ALLJOYN_CHANNEL);
    DH.write(tx_msg);
}

/**
 * Initializes the Arduino firmware.
 */
void setup(void)
{
    sensors.begin();
    
    lcd.begin(20,2);
    lcd.clear();
    pinMode(13, OUTPUT);

    // setLcdText("Bridge init...");
    Bridge.begin();
    Console.begin();
    while (!Console);
    DH.begin(Console);
    // setLcdText("Bridge init...Done!");
    
    updateRegDataWithGuid(REG_DATA);
    sendAllJoynInfoResponse();
    DH.writeRegistrationResponse(REG_DATA);
}


/**
 * Loop procedure is called continuously.
 */
void loop(void)
{
    int i = DH.read(rx_msg);
    if (i == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;
            case 1000: // "set" - sets the LED state
            {
                const long cmd_id = rx_msg.getLong();
                char text[255];
                
                unsigned int res = rx_msg.getString(text, 255);

                setLcdText(text);
                DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
            } break;

            case 30001: // alljoyn info request
                sendAllJoynInfoResponse();
                break;

            case 30003: // alljoyn session status
            {
                int state = rx_msg.getByte();
                setLedState(state);
            } break;
        }

        rx_msg.reset(); // reset for the next message parsing
    }
    
    if (1 && millis() - lastPoll > pollInterval)
    {
      float temp = getTemp();
      sendTemp(temp);
      lastPoll = millis();
    }
}  
