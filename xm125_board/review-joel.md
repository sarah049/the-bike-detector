# Schematic

* And see note about connectors on ESP32 review. Same applies here.
* You'll want GND and VCC in addition to the four pins on the USB_Data port for your programmer. And ideally that header is arranged so it is easy to connect to a programmer module, like it has pins sticking up that are arranged in the same arrangement as the programmer arrangement
* Good to add pullups on SDA/SCL even if you don't use them
* Add in status LEDs for debugging even if you don't use them in the final
* I would do a comp of the Sparkfun schematic and the Acconeer XE125 eval board schematic and make sure there's nothing important missing from the sparkfun one.
* You need a connector that goes to the ESP32 board so you can send over the bike interrupt and more granular sensor data (which you'll want for debugging). Not sure if you'll use the DEBUG UART or the I2C for that comms.

