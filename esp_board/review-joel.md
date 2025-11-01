# Schematic

* You want to use the ESP32-C3-WROOM-02 module. The MINI is LGA and basically impossible to hand-solder
* Settle on your connectors before finalizing the board
* You'll want a USBC connector on their to be able to easily program and get serial out, etc. Here's a USBC connector that is hand-solderable: USB4 145-03-0170-C
* Does your cellular module get attached to the ESP32 board? If so you will want to add headers for that.
* For the reset/boot switches, one they used last year was the Button_Switch_SMD:SW_Push_1P1T_XKB_TS-1187A. But you can pick most any SMT button that you like.
* Add status LEDs. You may not use them in the final device, but you'll want them for debugging
* You can leave out the transistors. They are there so the programmer can automatically put the ESP32 into download mode but it's just as easy to hit the boot/reset buttons and do that manually. In fact, I don't think those transistors will do anything b/c with your USB connector you won't have DTR/RTS lines. 

* I'll look at layout once schematic is updated