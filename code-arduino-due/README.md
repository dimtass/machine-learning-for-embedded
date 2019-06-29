Arduino DUE code
----

> This code is part of the post series in [here](https://www.stupid-projects.com/machine-learning-on-embedded-part-1/)


This code is tested on the `Arduino DUE`. The difference with the code
from the other arduino boards is that the TimerOne library is not available
for DUE, so I had to use this lib for the timer callbacks.


## Instructions
To build the project open Arduino IDE and then load the `code-arduino-due.ino`
code file, build and upload. In case of the Teensy boards you can also
overclock to the maximum frequencies.

## Versions
Arduino IDE: 1.8.9
Arduino SAM Boards (32-bits ARM Cortex-M3): 1.6.12
DueTimer: https://github.com/ivanseidel/DueTimer 1ac2e5da962029fbe12b8faad3808b03694b0c9f