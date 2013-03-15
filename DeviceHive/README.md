DeviceHive library for Arduino
==============================

This is auxiliary library implementing [DeviceHive](http://www.devicehive.com/)
[binary protocol](http://www.devicehive.com/binary/) on Arduino platforms.


Installation
------------

Installation of DeviceHive Arduino library is quite simple. Just download
library archive [here](https://github.com/Pilatuz/devicehive-arduino/archive/1.0.zip)
and unpack `DeviceHive` subfolder to the `libraries` folder of your Arduino installation path.
More details could be found at corresponding [Arduino](http://arduino.cc/en/Guide/Libraries) page.


Quick start
-----------

Once DeviceHive Arduino library is installed you can use it. Just include `DeviceHive.h`
into your sketch and do not forget to initialize DeviceHive engine with
an appropriate serial port:

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


DeviceHive Engine
-----------------

DeviceHive engine is connected to a serial port and converts messages into/from
stream of bytes. On the one hand it converts messages to the series of bytes and
writes them to the serial port. On the other hand it receives series of bytes
from the serial port and tries to parse messages.


### Initialization

It's important to initialize DeviceHive engine with an appropriate serial port.
A good place to do that is the `setup()` function. Usually, the `Serial` serial
port is used, but some boards (such as Arduino Mega) also have additional
[serial ports](http://arduino.cc/en/Reference/Serial):
`Serial1`, `Serial2` and `Serial3`. You can use any of them.

~~~{.cpp}
Serial.begin(9600);
DH.begin(Serial);
~~~


### Sending messages

To send message one of `DH.write()` methods should be used. But you have to prepare
message before. The `OutputMessage` provides interface for message formatting.
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
  - `DH.writeCommandResult(id, status, result)` - sends command result as example above.


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


It's possible to provide a timeout for message receiving. If time interval from
the begin of message is great than timeout value then the message will be dropped.
The following code sets the 2 seconds receiving timeout:

~~~{.cpp}
DH.setRxTimeout(2000);
~~~

Receiving timeout is equal to 1 second by default. But if timeout value is set
to zero then no any timeouts will be checked.


Messages
--------

A message has intent number which identifies message purpose and a custom
payload which depends on the message intent. The maximum message payload size
is statically defined by the `MAX_MSG_SIZE` constant which is 256 by default.
That means that library is unable to process messages with payload size more
than 256 bytes although DeviceHive binary protocol allows messages up to 64K
(see next section for advanced message usage).


### Formatting

The `OutputMessage` provides a set of methods to format message payload:
  - `put(buf, len)` method is used to write custom byte sequence such as a custom structure.
                    Also the `put(val)` auxiliary template methods are available.
  - `putString(str, len)` method prepends string content with 2-bytes string length.
  - `putString(str)` method prepends NULL-terminated string content with 2-bytes string length.
  - `putUInt32(val)` and `putInt32(val)` methods are used to write a 32-bits integer in little-endian format.
                     Also the following synonyms are avaialbe: `putULong(val)` and `putLong(val)`.
  - `putUInt16(val)` and `putInt16(val)` methods are used to write a 16-bits integer in little-endian format.
                     Also the following synonyms are avaialbe: `putUShort(val)` and `putShort(val)`.
  - `putUInt8(val)` and `putInt8(val)` methods are used to write a 8-bits integer.
                    Also the following synonyms are avaialbe: `putByte(val)` and `putChar(val)`.

Each of these methods does nothing if there is no enough space in message payload.


### Parsing

The `InputMessage` provides a set of methods to parse message payload:
  - `skip(len)` method just skips specified number of bytes.
  - `get(buf, len)` method is used to read custom byte sequence such a custom structure.
                    Also the `get(val)` auxiliary template methods are available.
  - `getString(str, max_len)` method reads string length and string content. Returned string is always NULL-terminated.
  - `getUInt32()` and `getInt32()` methods are used to read a 32-bits integer in little-endian format.
                  Also the following synonyms are avaialbe: `getULong()` and `getLong()`.
  - `getUInt16()` and `getInt16()` methods are used to read a 16-bits integer in little-endian format.
                  Also the following synonyms are avaialbe: `getUShort()` and `getShort()`.
  - `getUInt8()` and `getInt8()` methods are used to read a 8-bits integer.
                 Also the following synonyms are avaialbe: `getByte()` and `getChar()`.

The `reset()` method is used to reset reading position to the begin
of message payload. This may be important because usually there is only
one global `rx_msg` variable which is used to process all incoming messages.
So we have to rewind reading poisition after message has been processed.


Advanced Usage
--------------

There are a few advanced usage practices:
  - messages with an external buffer
  - messages with custom static buffer size
  - *stream* writing operations


### External buffer

You could provide a custom byte buffer (as big as you want)
for input and/or output messages.

`OutputMessageEx` is an advanced replacement of `OutputMessage`. It provides
exactly the same methods but uses an external byte buffer. For example, the
following code snippet sends a quite big message:

~~~{.cpp}
uint8_t big_buf[1024];
OutputMessageEx msg(big_buf, sizeof(big_buf), MY_INTENT_ID);
for (short i = 0; i < sizeof(big_buf)/2; ++i)
    msg.putShort(i);
DH.write(msg);
~~~

The same way `InputMessageEx` could be used. It is an advanced replacement
of `InputMessage` and contains all the same methods.

~~~{.cpp}
uint8_t big_rx_buf[1024];
InputMessageEx rx_msg(big_rx_buf, sizeof(big_rx_buf));
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

Be careful to use the same buffer for input and output messages! You can use
it only when `DH.read()` returns `DH_PARSE_OK`, i.e. at the end of message
receiving process. Otherwise, just because the byte buffer contains a part of
receiving message, output message which uses the same buffer will override
content of receiving input message.


### Custom static buffer size

There is possible to use message class templates with static buffer size as
the template argument. The output message example from the section above
could be rewritten as:

~~~{.cpp}
OutputMessageN<1024> msg(MY_INTENT_ID);
...
~~~

The input message example could be rewritten as:

~~~{.cpp}
InputMessageN<1024> rx_msg;
...
~~~


### Stream writing

One more way to send big messages is to use *stream* writing methods.
It's very low level of DeviceHive engine usage. Use it with care!

Let's start with an example which writes a command result:

~~~{.cpp}
const unsigned int status_len = strlen(status);
const unsigned int result_len = strlen(result);

unsigned int checksum = DH.writeHeader(
    INTENT_COMMAND_RESULT, sizeof(uint32_t)
    + sizeof(uint16_t) + status_len
    + sizeof(uint16_t) + result_len);
checksum += DH.writeUInt32(cmd_id);
checksum += DH.writeString(status, status_len);
checksum += DH.writeString(result, result_len);
DH.writeChecksum(checksum);
~~~

This is an implementation of the `DH.writeCommandResult(cmd_id, status, result)`
method. It sends command identifier (32-bits integer) and two strings: status
and result.

We start message writing by `DH.writeHeader()` method call. It sends message
signature, message intent and length. The message length should be very
carefully calculated! In our case it contains 32-bits integer and two
string each pretended with 16-bits length.

Next, we send message payload using `DH.writeUInt32()` and two `DH.writeString()`
method calls.

During the *stream* writing operation we also have to calculate the whole message
checksum. It is not so hard to do because each of *stream* writing methods returns
part of this checksum. All we need to do is to sum all of these values and send
final checksum at the end of message using `DH.writeChecksum()` method call.

It's all. We've sent command result.

To sum up, the following methods could be used for *stream* writing operations:
  - `DH.writeHeader(intent, length)` writes a message header to the serial stream.
      Should be called at the start of sending output message. Returns header's checksum part.
  - `DH.writePayload(buf, len)` writes a custom payload to the serial stream.
      Returns payload's checksum part.
  - `DH.writeString(str, len)` writes a string to the serial stream pretended with 16-bits length.
      Returns payload's checksum part.
  - `DH.writeUInt32(val)` writes 32-bits integer to the serial stream in little-endian format.
      Returns payload's checksum part.
  - `DH.writeUInt16(val)` writes 16-bits integer to the serial stream in little-endian format.
      Returns payload's checksum part.
  - `DH.writeChecksum(checksum)` writes checksum byte to the serial stream.
      Should be called at the end of sending output message.
