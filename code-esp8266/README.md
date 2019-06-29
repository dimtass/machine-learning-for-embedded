Arduino code for ESP8266-12E
----

> This code is part of the post series in [here](https://www.stupid-projects.com/machine-learning-on-embedded-part-1/)

## Instructions
To build the project open Arduino IDE and then load the `code-esp8266.ino`
code file, build and upload. In case of the Teensy boards you can also
overclock to the maximum frequencies.

## Notes
I've used the NodeMCU ESP8266 12-E module. The silkscreen on this
module is wrong. Therefore, although he `DEBUG_PIN` is set to 5
you need actually to probe the D1 pin.

## Arduino IDE settings
The settings I've used are:

* `Board`: NodeMCU 1.0 (ESP8266-12E Module)
* `Upload Speed`: 115200
* `CPU Frequency`: 160MHz
* `Flash Size`: 4M
* `Deug Port`: Serial
* `Debug Level`: None
* `VTables`: IRAM
* `Port`: Select the port that the module is registering

## Versions
Arduino IDE: 1.8.9
esp8266 community version: 2.5.2