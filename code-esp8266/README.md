Arduino code for ESP8266-12E
----

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