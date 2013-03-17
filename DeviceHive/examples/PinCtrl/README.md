DeviceHive and Arduino: control pins via Web
============================================

[Raspberry Pi]: http://www.raspberrypi.org "Raspberry Pi official site"
[DeviceHive]: http://www.devicehive.com "DeviceHive official site"
[Arduino]: http://arduino.cc/en/ "Arduino official site"

This example demonstrates ability to control [Arduino]'s pins over Web.
It's possible to set pin mode (input or output), set values for output pins,
and get values for input pins.

[Arduino] board should be connected to the Internet via [Raspberry Pi]
which acts as a gateway. See other examples for connection details.

`PinCtrl` example supports simple commands and notifications.


Commands
--------

The following commands are supported:
  - `getPinMode` - gets a pin mode
  - `setPinMode` - sets a pin mode
  - `pinRead` - gets an input pin value
  - `pinWrite` - sets an output pin value


### Get pin mode

This command forces to send `pinMode` notifications for requested pins.
Requisted pins are specified as array of integers in command parameters.
If the pin array is empty then *all* pins will be reported back.

A client application (usually HTML5) should get all pin modes at startup:

~~~{.js}
{
  command: "getPinMode",
  parameters: []
}
~~~


### Set pin mode

This command sets a pin properties. Requested pin and mode are specified
as command parameters. The `pinMode` notification will be sent back.
For input pins the 'pinRead' notification also will be sent.

A pin mode is defined by an integer value:
  - `11` - digital output
  - `12` - digital input
  - `13` - digital input with pull-up resistor
  - `21` - analog output
  - `22` - analog input
  - `23` - analog input with pull-up resistor

To set pin `2` as a digital output, client application
should send the following command:

~~~{.js}
{
  command: "setPinMode",
  parameters: {
    pin: 2,
    mode: 11
  }
}
~~~


### Read pin value

This command forces to send `pinRead` notifications for requested pins.
Requested pins are specified as array of integers in command parameters.
If the pin array is empty then *all* pins will be reported back.

~~~{.js}
{
  command: "pinRead",
  parameters: [3,4,5]
}
~~~


### Write pin value

This command sets an output pin value. The `pinWrite` notification will
be sent back. Does nothing if pin mode isn't an output.

For example, to write *HIGH* to the pin `2`, client application
should send the following command:

~~~{.js}
{
  command: "pinWrite",
  parameters: {
    pin: 2,
    value: 1
  }
}
~~~

It's also possible to set values for PWM (analog output) pins ranged from 0 to 255.


Notifications
-------------

The following notifications are supported:
  - `pinMode` - reports a pin mode
  - `pinRead` - reports an input pin value
  - `pinWrite` - reports an output pin value
  - `reset` - reports board reset


### Pin mode

Pin mode notification is sent when a pin mode's changed.
It reports pin number and the pin mode (see `setPinMode` command)
in notification parameters.

~~~{.js}
{
  command: "pinMode",
  parameters: {
    pin: 2,
    mode: 11
  }
}
~~~


### Read pin value

This notification is sent for an input pin when valus's changed or when
pin value's explicitly requested with `readPin` command. It reports pin
number and the input value in notification parameters.

~~~{.js}
{
  command: "pinRead",
  parameters: {
    pin: 2,
    value: 1
  }
}
~~~


### Write pin value

This notification is sent for an output pin when value's changed with
`writePin` command. It reports pin number and the input value
in notification parameters.

~~~{.js}
{
  command: "pinWrite",
  parameters: {
    pin: 2,
    value: 0
  }
}
~~~


### Board reset

This notification is sent after [Arduino] board's reset.
This notification has no parameters.


~~~{.js}
{
  command: "reset",
  parameters: null
}
~~~


Conclusion
----------

This example demonstrated how to control your [Arduino] pins over Internet
using DeviceHive framework. See also the HTML5 parts of this example.
