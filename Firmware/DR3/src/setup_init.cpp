#include "setup_init.h"
#include <Preferences.h>

Preferences preferences;
// String mac_addr;

// Returns Mac Address
String getMacAddress(){
  uint64_t chipID = ESP.getEfuseMac();
  char ssid[13];
  snprintf(ssid, 13, "%llX", ESP.getEfuseMac());
  ssid[12] = '\0';
  String mac_addr = (String)(ssid) ; // convert from char to String
  return mac_addr;
}


//Flash Write
bool FlashWrite(const String& key, const String& value) {
  bool success = false;
  
  Serial.printf("I'm writing key: %s, value: %s\n", key.c_str(), value);

  preferences.begin("monitor_info", false);
  success = preferences.putString(key.c_str(), value);
  preferences.end();
  
  
  return success;
}
   
// Flash Read
String FlashRead(const String& key) {
  String value = "invalid";
  
  preferences.begin("monitor_info", true);
  value = preferences.getString(key.c_str(), "");  // default value
  preferences.end();

  
  return value;
}
  
// Initialize flash storage - call this once at startup
bool initFlashStorage() {
  if (preferences.begin("monitor_info", false)) {
    Serial.println("Flash storage initialized successfully");
    return true;
  } else {
    Serial.println("Failed to initialize flash storage");
    return false;
  }
}

// initialize mac address, firmware version, and initialization var

bool writeInitFlash() {
  // Only initializes variable "initialized"
  // mac address is gotten from the function getMacAddress()
  // firmware version is an explicit const in main.cpp

  bool success = initFlashStorage();
  
  if (!FlashWrite("initialized", "true")) {
    Serial.println("Failed to write 'initialized' to flash");
    success = false;
  }
  
  return success;
}

// ensure initialization
bool checkInit() {
  if (FlashRead("initialized") == "true"){
    return true;
  }
  return writeInitFlash();

}

