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


Commands
--------

[Arduino] board flashed with this example supports the following commands:
  - `getPinMode` - gets one or several pin modes
  - `setPinMode` - sets a pin mode
  - `pinRead` - forces device to send pin value
  - `pinWrite` - writes pin value


### Get pin mode

This command expected an array of pin numbers as parameters. For each
requested pin the `pinMode` notification will be send back. If the pin
numbers array is empty then *all* pins will be reported back.

A client application (usually HTML5) should get current pin modes at startup.

~~~{.js}
{
  command: "getPinMode",
  parameters: []
}
~~~


### Set pin mode

This command sets a pin properties. The `pinMode` notification will be send back.
A pin mode is defined by the integer number:
  - `11` - digital output
  - `12` - digital input
  - `13` - digital input with pull-up resistor
  - `21` - analog output
  - `22` - analog input
  - `23` - analog input with pull-up resistor

For example, to set pin `2` to the output, client application should send the following command:

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

This command expected an array of pin numbers as parameters. For each
requested pin the `pinRead` notification will be send back. If the pin
numbers array is empty then *all* pins will be reported back.

~~~{.js}
{
  command: "pinRead",
  parameters: []
}
~~~


### Write pin value

This command sets an output pin value. The `pinWrite` notification will be send back.
Does nothing if pin mode isn't output.

For example, to write *HIGH* to the pin `2`, client application should send the following command:

~~~{.js}
{
  command: "pinWrite",
  parameters: {
    pin: 2,
    value: 1
  }
}
~~~

It's also possible to set values for PWM pins (ranged from 0 to 255).


Notifications
-------------

The following notifications are supported:
  - `pinMode` - reports current pin mode
  - `pinRead` - reports input pin value
  - `pinWrite` - reports output pin value
  - `reset` - reports board reset


### Pin mode

Pin mode notification is reported when a pin mode's changed.
It reports pin number and the pin mode (see `setPinMode` command).

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

This notification is sent for input pins when valus's changed or when
pin value's explicitly requested with `readPin` command.

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

This notification is sent for output pins when value's changed with
`writePin` command.

~~~{.js}
{
  command: "pinWrite",
  parameters: {
    pin: 2,
    value: 1
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
