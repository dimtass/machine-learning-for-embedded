Machine learning for embedded
----

This repo is part of a blog series which is called `Machine Learning on Embedded`
and it starts here:

[https://www.stupid-projects.com/machine-learning-on-embedded-part-1/](https://www.stupid-projects.com/machine-learning-on-embedded-part-1/)

This repo is for [part 1](https://www.stupid-projects.com/machine-learning-on-embedded-part-1/) and [part 2](https://www.stupid-projects.com/machine-learning-on-embedded-part-12/) and it contains the source code for two naive NN implementations.
Each folder contains the source code for a different MCU.

I've tested this with the following MCUs

MCU | Source folder | Build system
-|-|-
Arduino Uno | code-arduino | Arduino IDE
Arduino Leonardo | code-arduino | Arduino IDE
Arduino nano | code-arduino | Arduino IDE
Arduino Due | code-arduino-due | Arduino IDE
Teensy 3.2 | code-arduino-due | Arduino IDE
Teensy 3.5 | code-arduino-due | Arduino IDE
Arduino Due | code-arduino-due | Arduino IDE
ESP8266 | code-esp8266 | Arduino IDE
STM32F103C8T6 | code-stm32f103| Arduino IDE
STM32F746NG | code-stm32f746 | Arduino IDE


## NN implementations
The source code for the NN helper functions is located in those two files:

* neural_network.h
* neural_network.c

Each code folder has its own files.

But the weights, the input and ouput and the NN implementation is in
the main source code file (e.g. code-arduino.ino). The functions that
run these naive inferences are:

* void test_neural_network(void)
* void benchmark_neural_network(void)
* void test_neural_network2(void)
* void benchmark_neural_network2(void)

These functions are numbered because there are two NN implementations.
The first one is 3-input, 1-output. The second one is 3-input, 32-hidden,
1-output.

To measure the time each inference needs to run, I'm toggling a pin.
You can see which pin that is in the code, because it's different for
each different MCU.

> You'll find more informations on the blog posts.

## License
The license is MIT and you can use the code however you like.

## Author
Dimitris Tassopoulos <dimtass@gmail.com>