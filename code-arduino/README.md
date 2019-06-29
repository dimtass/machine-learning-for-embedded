Arduino IDE code
----

> This code is part of the post series in [here](https://www.stupid-projects.com/machine-learning-on-embedded-part-1/)

The code is tested with the following MCUs:

* Arduino UNO
* Arduino nano
* Arduino mini
* Arduino Leonardo
* Teensy 3.2
* Teensy 3.5

## Instructions
To build the project open Arduino IDE and then load the `code-arduino.ino`
code file, build and upload. In case of the Teensy boards you can also
overclock to the maximum frequencies.

## Libraries
To build this code you need the TimerOne library. Just install it from
the Library manager in the arduino IDE. The version I've used is 1.0.1.

In case of Teensy 3.5 there's a conflict for the TimerOne lib from the
library manager as the board package comes with that integrated for the
specific MCU. In this case you need to uninstall the previous (just delete
the folder with the library) and build.

## Versions
Arduino IDE: 1.8.9
TimerOne: 1.0.1