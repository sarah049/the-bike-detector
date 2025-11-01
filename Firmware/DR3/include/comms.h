// #include <Arduino.h>
// #include <HardwareSerial.h>

// Comms - 3 PINs (can assign 3 hardware UART pins)

#ifndef COMMS_H
#define COMMS_H

String sendATCommand(const char *command);
int prep_and_post_comms(String queryString, bool needsInit);
String getServerResponse();
// void setupOTA();
// bool checkAndPerformOTA(HttpConnection &connection);
// void otaProgress(int progress);

#endif
