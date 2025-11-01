#ifndef SETUPINIT_H
#define SETUPINIT_H

#include <Arduino.h>
#include <string.h>


// functions
String getMacAddress();
bool FlashWrite(const String& key, const String& value);
String FlashRead(const String& key);
bool initFlashStorage();
bool writeInitFlash();
bool checkInit();

#endif