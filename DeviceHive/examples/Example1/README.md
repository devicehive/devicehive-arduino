DeviceHive and Arduino
======================

[Raspberry Pi]: http://www.raspberrypi.org "Raspberry Pi official site"
[DeviceHive]: http://www.devicehive.com "DeviceHive official site"
[Arduino]: http://arduino.cc/en/ "Arduino official site"


This is the most basic example of DeviceHive [Raspberry Pi] - [Arduino] combo.
To set it up, you should download the Raspberry Pi [Python gateway](http://TODO_PYTHON_GATEWAY.zip)
upload it to Rasberry Pi and install Twisted module by running `sudo apt-get install python-twisted`
command. After this you should run `nano basic_gateway_example.py` to edit
and modify the parameters of gateway to point to your cloud or playground URL,
set the device path to point to your Arduino attached.
Typically it is `/dev/ttyUSB0` or `/dev/ttyACM0` (you can verify by running
`ls /dev/tty*`). These settings can be found in the very end of
the `basic_gateway_example.py` file.

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


Explained
---------

See [this](https://github.com/devicehive/devicehive-arduino/blob/master/DeviceHive/examples/Example1/Example1.ino)
file for full source codes. Let's take a closer look at the example.


### Definitions

~~~{.cpp}
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
~~~

The `REG_DATA` string contains registration data which gives the device
information to the gateway describing it's capabilities: commands it can
handle and notifications it can send.

*TODO: Describe how simple, struct and notifications are defined*. Please refer
to [this page](http://www.devicehive.com/binary/#SystemMessages/RegisterJson)
for complete syntax of registration data.

IMPORTANT: For parameters we will be passing in "blink" command, you should
define a struct that contains *EXACTLY* the same fields, of *EXACTLY* the same
type in *EXACTLY* the same order, so we can later load the newly received
parameters into this struct.

~~~{.cpp}
struct BlinkParam
{
    unsigned short on;
    unsigned short off;
    byte count;
};
~~~


### Initialization

In order to initialize DeviceHive library, you should call in `setup()` function:

~~~{.cpp}
DH.begin(Serial);
DH.writeRegistrationResponse(REG_DATA);
~~~


### Processing

After this in the `loop()` function you should monitor the pin 12 for state changes:

~~~{.cpp}
const int btn_state = digitalRead(BTN_PIN);
if (btn_state != old_btn_state)
{
    sendButtonState(btn_state);
    old_btn_state = btn_state;
}
~~~

When sending the notification to gateway, you should pass the intent ID
and put parameters into the output message:

~~~{.cpp}
void sendButtonState(int state)
{
    OutputMessage tx_msg(2000);
    tx_msg.putByte(state);
    DH.write(tx_msg);
}
~~~

To check for new messages from the cloud in the `loop()` function, do this:

~~~{.cpp}
if (DH.read(rx_msg) == DH_PARSE_OK)
~~~

Each message contains the `intent` field, which would correspond to the intent
identifier defined in initialization string. You *MUST* respond
to `INTENT_REGISTRATION_REQUEST` by sending registration data back:

~~~{.cpp}
switch (rx_msg.intent)
{
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
} // switch (intent)
~~~


Admin console
-------------

You can use admin console of your playground to play with connected Arduino.
Please connect your Arduino board to the Raspberry Pi and run gateway application.
New device named "Arduino LED" should be available in device list of your playground.
Go to the "commands" tab and press the "enter new command" button. Then enter the
`set` in the command name field and `1` or `0` in the parameters. You should see how
on-board LED will change state.

ALso you can enter `blink` command with any parameters, for example:
`{"count":3,"on":1000,"off":500}` whith switches LED 3 times.


Conslusion
----------

This example demonstrated how it's simple to control your [Arduino] over Internet
using [DeviceHive] framework.
