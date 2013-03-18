DeviceHive and Arduino
===========================

[Raspberry Pi]: http://www.raspberrypi.org "Raspberry Pi official site"
[DeviceHive]: http://www.devicehive.com "DeviceHive official site"
[Arduino]: http://arduino.cc/en/ "Arduino official site"


This is the most basic sample of DeviceHive [Raspberry Pi] - [Arduino] combo.
To set it up, you should download the Raspberry Pi [Python gateway](http://TODO_PYTHON_GATEWAY.zip)
upload it to Rasberry Pi and install Twisted module by running `sudo apt-get install python-twisted`
command. After this you should run `nano basic_gateway_example.py` to edit
and modify the parameters of gateway to point to your cloud or playground URL,
set the device path to point to your Arduino attached.
Typically it is `/dev/ttyUSB0` or `/dev/ttyACM0` (you can verify by running
`ls /dev/tty*`). These settings can be found in the very end of the file.

After you save the file and run it, the gatway code will request Arduino for
it's initialization parameters and pass them to DeviceHive cloud. After that
you will be able to send command and get notifications to/from your Arduino.
In this example, the commands are "set" - which will set the state of on-board
LED connected to pin 13 to on/off (1/0) and "blink" which will accept
a structure which consists of on/off/count fields to set on/off period
in milliseconds and count (for example: `{"on":500, "off":1000, "count":5}`).
In addition to these commands, the sample Arduino code will also be sending
notifications when pin 12 changes its sate. You can test it by
connecting/disconnecting +5V probe to PIN 12 to simulate LOW/HIGH states.

Let's take a closer look at the example:

`REG_DATA` contains registration string which gives the device information to
the gateway describing it's capabilities: commands it can handle
and notifications it can send.

*Describe how simple, struct and notifications are defined*. Please refer
to [this page](http://www.devicehive.com/binary/#SystemMessages/RegisterJson)
for complete syntax of registration data.

IMPORTANT: For parameters we will be passing in "blink" command, you should
define a struct that contains *EXACTLY* the same fields, of *EXACTLY* the same
type in *EXACTLY* the same order, so we can later load the newly received
parameters into this struct.

In order to initialize DeviceHive library, you should call:

~~~{.cpp}
DH.begin(Serial);
DH.writeRegistrationResponse(REG_DATA);
~~~

After this in the `loop()` you should query the pin 12 for state change:

~~~{.cpp}
const int btn_state = digitalRead(BTN_PIN);
if (btn_state != old_btn_state)
{
    sendButtonState(btn_state);
    old_btn_state = btn_state;
}
~~~

when sending the notification to gateway, you should pass the intent ID
and put parameters into the output message:

~~~{.cpp}
void sendButtonState(int state)
{
    OutputMessage tx_msg(2000);
    tx_msg.putByte(state);
    DH.write(tx_msg);
}
~~~

To check for new messages from the cloud in the `loop()`, do this:

~~~{.cpp}
if (DH.read(rx_msg) == DH_PARSE_OK)
~~~

Each message contains the field intent, which would correspond to the intent
identifer defined in initialization string. You *MUST* respond
to `INTENT_REGISTRATION_REQUEST` by sending registration data back:

~~~{.cpp}
case INTENT_REGISTRATION_REQUEST:   // registration data needed
    DH.writeRegistrationResponse(REG_DATA);
    break;
~~~

Handle "set" command:

~~~{.cpp}
case 1000:
{
    const long cmd_id = rx_msg.getULong();
    const byte state = rx_msg.getByte();

    setLedState(state);
    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
} break;
~~~

For "blink" command you should unload the received parameters values into the struct:

~~~{.cpp}
case 1001:
{
    const long cmd_id = rx_msg.getLong();
    BlinkParam params = rx_msg.get<BlinkParam>();
    ...
~~~

do the actual blinking and report command result back to gateway:

~~~{.cpp}
    ...
    for (int i = 0; i < params.count; ++i)
    {
        setLedState(1);     // ON
        delay(params.on);
        setLedState(0);     // OFF
        delay(params.off);
    }

    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
} break;
~~~
