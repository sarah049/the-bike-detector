// #include <Arduino.h>
// #include <HardwareSerial.h>

//Comms - 3 PINs (can assign 3 hardware UART pins)

#ifndef COMMS_H
#define COMMS_H

void sendATCommand(const char *command);
void prep_and_post_comms(String queryString);
String getServerResponse();

#endif
