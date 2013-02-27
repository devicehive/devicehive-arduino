DeviceHive library for Arduino
==============================

This is auxiliary library implementing [DeviceHive](http://www.devicehive.com/)
[binary protocol](http://www.devicehive.com/binary/) on Arduino platforms.


Installation
------------

Installation of DeviceHive Arduino library is quite simple. Just download
library archive [here](http://www.devicehive.com/ARDUINO_LIBRARY.zip) and
unpack it to the `libraries` folder of your Arduino installation path.
More details could be found at [Arduino](http://arduino.cc/en/Guide/Libraries) page.


Quick start
-----------

Once DeviceHive Arduino library is installed you can use it. Just include it
into your sketch, do not forget to initialize DeviceHive engine with
appropriate serial port:

~~~{.cpp}
#include <DeviceHive.h>

void setup()
{
    // ...
    Serial.begin(9600);
    DH.begin(Serial);
}
~~~

The second step is to listen for incomming messages and process them:

~~~{.cpp}
InputMessage rx_msg; // received message
void loop()
{
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            ... // process message
        }

        rx_msg.reset();
    }
}
~~~


DeviceHive Engine
-----------------

DeviceHive engine is connected to a serial port and converts messages into/from
stream of bytes: on the one hand it converts messages to the series of bytes and
writes them to the serial port, on the other hand it receives series of bytes
from the serial port and tries to parse messages.


### Initialization

It's important to initialize DeviceHive engine with an appropriate serial port.
A good place to do that is `setup()` function. Usually, the `Serial` serial
port is used, but some boards (such as Arduino Mega) also have additional
[serial ports](http://arduino.cc/en/Reference/Serial):
Serial1, Serial2 and Serial3. You can use any of them.

~~~{.cpp}
Serial.begin(9600);
DH.begin(Serial);
~~~


### Sending messages

To send message one of `DH.write()` methods should be used. But before you have
to prepare message. The OutputMessage provides interface for message formatting.
The following example shows how to send command result which contains command
identifier, status and result.

~~~{.cpp}
OutputMessage msg(INTENT_COMMAND_RESULT);
msg.putUInt32(cmd_id);
msg.putString(status);
msg.putString(result);
DH.write(msg);
~~~

There are also a few auxiliary *writeXXX* methods to send a system-defined messages:
  - `DH.writeRegistrationResponse(data)` - sends registration data in JSON format.
  - `DH.writeCommandResult(id, status, result)` - sends command result exactly the same way as example above.


### Receiving messages

Message receiving is a long-term process and it's performed in the `loop()`
function. DeviceHive engine's `DH.read()` method tries to parse message header,
receive message body and check message checksum.

~~~{.cpp}
InputMessage rx_msg;
void loop()
{
    if (DH.read(rx_msg) == DH_PARSE_OK)
    {
        switch (rx_msg.intent)
        {
            ... // process message
        }

        rx_msg.reset();
    }
}
~~~

Note, `rx_msg` should be global variable because message usually is collected
during several `loop()` calls.

Once `DH.read()` method returns `DH_PARSE_OK` the received message can be processed.
Other possible return values are usually errors:
  - `DH_PARSE_MESSAGE_TOO_BIG` - incoming message is too big
  - `DH_PARSE_BAD_CHECKSUM` - invalid checksum, message is corrupted
  - `DH_PARSE_INCOMPLETE` - not enough data, receiving in progress
  - `DH_PARSE_TIMED_OUT` - message receiving is timed out
  - `DH_PARSE_NO_SERIAL` - DeviceHive engine is not initialized


It's possible to provide a timeout for receiving message. If time interval from
the begin of message is great than timeout then the message will be dropped.
The following code sets the 2 seconds receiving timeout:

~~~{.cpp}
DH.setRxTimeout(2000);
~~~

Receiving timeout is equal to 1 second by default. If this timeout is set
to zero then no any timeout will be checked.


Messages
--------

A message has intent number which identifies message purpose and a custom
payload which depends on the message type. The maximum message payload size is
statically defined by the `MAX_MSG_SIZE` constant which is `256` by default.
That means that library is unable to process messages with payload size more
than 256 bytes although DeviceHive binary protocol allows messages up to 64K.


### Formatting

The `OutputMessage` provides a set of methods to format message payload:
  - `put(buf, len)` method is used to write custom byte sequence such as a custom structure.
  - `putString(str)` method prepends string content with 2-bytes string length.
  - `putUInt32(val)` and `putInt32(val)` methods are used to write a 32-bits integer in little-endian format.
  - `putUInt16(val)` and `putInt16(val)` methods are used to write a 16-bits integer in little-endian format.
  - `putUInt8(val)` and `putInt8(val)` methods are used to write a 8-bits integer.

Each of these methods does nothing if there is no enough space in message payload.


### Parsing

The `InputMessage` provides a set of methods to parse message payload:
  - `skip(len)` method just skips specified number of bytes.
  - `get(buf, len)` method is used to read custom byte sequence such a custom structure.
  - `getString(str, max_len)` method reads string length and string content. Returned string is always NULL-terminated.
  - `getUInt32()` and `getInt32()` methods are used to read a 32-bits integer in little-endian format.
  - `getUInt16()` and `getInt16()` methods are used to read a 16-bits integer in little-endian format.
  - `getUInt8()` and `getInt8()` methods are used to read a 8-bits integer.

The `reset()` method is used to reset reading poisition to the begin
of message payload. This may be important because usually there is only
one global `rx_msg` variable which is used to process all incoming messages.
So we have to rewind reading poisition after message's processed.
