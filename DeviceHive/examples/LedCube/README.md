DeviceHive and Rainbow Cube
===========================

[Rainbow Cube]: http://seeedstudio.com/wiki/Rainbow_Cube "Rainbow Cube wiki page"
[Rainbowduino]: http://seeedstudio.com/wiki/Rainbowduino_v3.0 "Rainbowduino v3.0 wiki page"
[Raspberry Pi]: http://www.raspberrypi.org "Raspberry Pi official site"
[DeviceHive]: http://www.devicehive.com "DeviceHive official site"


[Rainbow Cube] is an array of RGB LEDs. Each LED is managed by
an Arduino-compatible controller called [Rainbowduino].
Using Rainbowduino library it's possible to set RGB color
for each LED individually.

[Rainbowduino] is flashed with simple Arduino firmware which translates
commands from the DeviceHive server to the Rainbowduino library.
Because [Rainbow Cube] doesn't have Internet connection itself, we cannot
connect it to the DeviceHive server directly. So we use [Raspberry Pi] as a
gateway. It also translates [REST protocol](http://www.devicehive.com/restful)
to the [binary protocol](http://www.devicehive.com/binary).

iPhone/iPad or HTML5 applications allow user to pick up a custom color
and fill the whole or a part of [Rainbow Cube].

The control flow is simple: User selects a color. iPhone application sends
a command to the [DeviceHive] server. [Raspberry Pi] grabs that command from
the [DeviceHive] server and sends it to the LedCube device.


LedCube device
--------------

The LedCube device is a [Rainbow Cube] and a [Rainbowduino] flashed
with LedCube example firmware. The LedCube device is connected to
the [Raspberry Pi] via a serial port cable. If we use USB cable, the LedCube
device will be available as `/dev/ttyUSB0`. Also it's possible to make
the same connection wireless using pair of XBee modules.

LedCube example uses [binary protocol](http://www.devicehive.com/binary) to
communicate with the [Raspebrry Pi] gateway. We support the following commands:

- "fill" to fill the whole Cube with one color (the same color for all LEDs)
- "cube" to fill the whole Cube with individual colors (different colors for each LEDs)
- "pixels" to set colors of several LEDs at once

To build firmware you have to provide two additional dependencies:

- [Rainbowduino library](http://www.seeedstudio.com/wiki/images/4/43/Rainbowduino_for_Arduino1.0.zip)
- [DeviceHive Arduino library](https://github.com/devicehive/devicehive-arduino/archive/1.0.zip)

Please unpack these dependencies into the Arduino's `libraries` path.
Now you can build LedCube example and flash the [Rainbowduino] board.


LedCube firmware explained
--------------------------

LedCube device firmware listens for commands from gateway,
handles them and sends results back.


### Device registration

Each device should send a registration data at startup (or reset) and as
a response to registration request message. We use JSON format:

~~~{.cpp}
const char *REG_DATA = "{"
    "id:'b125698d-61bd-40d7-b65e-e1f86852a166',"
    "key:'LED_cube',"
    "name:'LED Cube',"
    "deviceClass:{"
        "name:'LED_cube',"
        "version:'1.0'},"
    "equipment:[{code:'cube',name:'cube',type:'LED_Cube'}],"
    "commands:["
        "{intent:1001,name:'fill',params:{R:u8,G:u8,B:u8,DX:u8,DY:u8,DZ:u8}}"
        "{intent:1002,name:'cube',params:[{R:u8,G:u8,B:u8}]}"
        "{intent:1003,name:'pixels',params:[{X:u8,Y:u8,Z:u8,R:u8,G:u8,B:u8}]},"
    "],"
    "notifications:[]"
"}";
~~~

All parameters are quite obvious: device identifier, key, name, device class
and equipment list. Since LedCube device is passive, i.e. doesn't have any
sensors or buttons, there is no any notifications supported. But we support
a few commands to control LEDs color.

Note, you have to provide an unique (in namespace of playground) device
identifier for each your LedCube device. Device identifier is a simple GUID
written in lower case.

We send registration data using `DH.writeRegistrationResponse()` method.


### Supported commands

The processing loop (the `loop()` function) just waits for incoming messages
and then processes them.

~~~{.cpp}
void loop(void)
{
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            case INTENT_REGISTRATION_REQUEST:   // registration data needed
                DH.writeRegistrationResponse(REG_DATA);
                break;

            ... // process other messages
        }

        rx_msg.reset(); // reset for the next message parsing
    }
}
~~~

The code snipped above tries to read message from serial. Once message has been
received, we check incoming message intent and do some actions. In this code
snippet we send registration data back in response to registration request
which is sent by the gateway periodically.

As was mentioned above we support we following commands: "fill", "cube"
and "pixels". Let's look at these commands in more detail.


#### "fill" command

The message intent for this command is 1001. A `Color` structure (RGB components)
is used as parameter for this command followed with a `Point` structure.

~~~{.cpp}
struct Color
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct Point
{
    uint8_t X;
    uint8_t Y;
    uint8_t Z;
};
~~~

Registration data for this command is `{intent:1001,name:'fill',params:{R:u8,G:u8,B:u8,DX:u8,DY:u8,DZ:u8}}`.
A corresponding DeviceHive command (defined by the REST protocol) which looks like

~~~{.js}
{
  id: 12345,
  command: "fill",
  parameters: {
    R: 11, G: 22, B: 33,
    DX: 4, DY: 4, DZ: 4
  }
}
~~~

will be converted by the gateway to the binary format. The code snippet below
shows example of processing "fill" command in binary format:

~~~{.cpp}
const uint32_t cmd_id = rx_msg.getUInt32();
const Color color = rx_msg.get<Color>();
const Point size = rx_msg.get<Point>();

for (int x = 0; x < size.x; ++x)
    for (int y = 0; y < size.y; ++y)
        for (int z = 0; z < size.z; ++z)
{
    Rb.setPixelZXY(z, x, y,
        color.R, color.G, color.B);
}

DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
~~~

First of all we read command identifier which will be used to report result
back to DeviceHive server. Then we read RGB components of color provided and
figure dimension. It's all we need to process this command.

Now we are able to apply received color for each LED in the LedCube device.
Once the job's done we send "OK" result back to gateway, and gateway will
send it to the DeviceHive server.

Note, if `DX`, `DY`, `DZ` parts of figure dimension are omitted or equal
to zeros, then the whole cube will be filled.


#### "cube" command

The message intent for this command is 1002. An array of `Color` structures
is used as parameter for this command.

Registration data for this command is `{intent:1002,name:'cube',params:[{R:u8,G:u8,B:u8}]}`.
Please note the square braces in the registration data. This means "array of" structures.
Corresponding DeviceHive command (defined by the REST protocol) should look like:

~~~{.js}
  id: 12345,
  command: "cube",
  parameters: [
    { R: 11, G: 22, B: 33 },
    { R: 44, G: 55, B: 66 },
    ...
  ]
~~~

The number of colors provided should be exactly the same as the number of LEDs
on the LedCube device. In our case this number is `4*4*4=64`.

The code snippet below shows example of processing "cube" command in binary format:

~~~{.cpp}
const uint32_t cmd_id = rx_msg.getUInt32();
const uint16_t count = rx_msg.getUInt16();

if (count == NX*NY*NZ)
{
    for (int x = 0; x < NX; ++x)
        for (int y = 0; y < NY; ++y)
            for (int z = 0; z < NZ; ++z)
    {
        const Color color = rx_msg.get<Color>();
        Rb.setPixelZXY(z, x, y,
            color.R, color.G, color.B);
    }

    DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
}
else
{
    DH.writeCommandResult(cmd_id, CMD_STATUS_FAILED, "Invalid number of colors");
}
~~~

Contrary to the first command we read number of color structures provided.
If this number isn't equal to the number of LEDs we report error back to
the gateway. Otherwise we read color for each LED individually (Z->Y->X)
and apply it.


#### "pixels" command

This command should be used to change a few pixels of the LedCube device
but not all off them (for that purpose use "cube" command). Please note,
because message payload length is limited to the 256 bytes in DeviceHive
arduino library (see `MAX_MSG_SIZE` constant) we cannot send more that
41 pixels at once (which is more than half of LEDs in our case).

The message intent for this command is 1003. An array of `Pixel` structures
is used as parameter for this command.

~~~{.cpp}
struct Pixel
{
    Point point;
    Color color;
};
~~~

Registration data for this command is `{intent:1003,name:'pixels',params:[{X:u8,Y:u8,Z:u8,R:u8,G:u8,B:u8}]}`.
Along with the RGB components each pixels contains the XYZ coordinates.
Corresponding DeviceHive command (defined by the REST protocol) should look like:

~~~{.js}
  id: 12345,
  command: "pixels",
  parameters: [
    { X: 0, Y: 0, Z: 0, R: 11, G: 22, B: 33 },
    { X: 1, Y: 1, Z: 1, R: 44, G: 55, B: 66 },
    ...
  ]
~~~

The code snippet below shows example of processing "pixels" command in binary format:

~~~{.cpp}
const uint32_t cmd_id = rx_msg.getUInt32();
const uint16_t count = rx_msg.getUInt16();
for (uint16_t i = 0; i < count; ++i)
{
    const Pixel px = rx_msg.get<Pixel>();
    Rb.setPixelZXY(px.point.Z, px.point.X, px.point.Y,
                   px.color.R, px.color.G, px.color.B);
}

DH.writeCommandResult(cmd_id, CMD_STATUS_SUCCESS, CMD_RESULT_OK);
~~~

The code is pretty obvious: we read number of pixels provided
and then read each coordinates and color components for each pixel
individually. Note, there are no range checks for coordinates to
simplify the code.


Conclusion
----------

This example demonstrated how to control your [Rainbow Cube] over Internet
using DeviceHive framework. See also the iPhone/iPad or HTML5 parts of this example.
