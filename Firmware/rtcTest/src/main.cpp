#include <Arduino.h>
#include <string.h>
#include <HardwareSerial.h>
#include <Preferences.h>
#include "comms.h"
#include "faultChecks.h"
#include "interrupts.h"
// #include "ota_updates.h"

#include "driver/rtc_io.h"

//GPIO Pin Assignments
#define SENSOR_DETECT_PIN   GPIO_NUM_4
#define BATT_PIN            GPIO_NUM_0
#define SENSOR_SCL_PIN      GPIO_NUM_2
#define SENSOR_SDA_PIN      GPIO_NUM_3
#define LED_PIN             GPIO_NUM_6
#define DEBUG_PIN           GPIO_NUM_1
#define RESET_PIN           GPIO_NUM_9
// #define DPLUS_PIN GPIO_NUM_18
// #define DMINUS_PIN


// debug pins
// #define LED_PIN_1 8 // Pin to flash LED
// #define LED_PIN_2 11 // Pin to flash LED

// General Startup Variables
#define TIME_BTWN_POST 3600000000 // time in us between posts, 3600000000 = 1 hr
#define BATT_THRESH 3.3 // TODO get battery voltage threshold

RTC_DATA_ATTR long long rtc_start_time;
RTC_DATA_ATTR uint8_t rtcArray[10];
RTC_DATA_ATTR uint32_t RTCArrayIndex ;
time_t start_time;
int action_to_do = 0; // 0 = setup/reboot, 1 = record data, 2 = post data

float buck_convert_voltage_factor = (9/5.1); // factor to "undivide" voltage divider to read battery voltage  

float time_to_send_to_cellular = 20.0;

Preferences preferences;
time_t elapsed_time;
String mac_address = "";
float firmware_v = 1.0;
char testMonID[10] = "test";

static void sensor_timer_callback(void* arg);

// OTA update configuration
#define FIRMWARE_VERSION "1.0.0"


String aggregateData(){
  Serial.printf("Aggregating Data\n");
  // Serial.printf("Current faults: %s\n", faults);
  // "check faults"
  // getting time from start time
  elapsed_time = time(NULL) - start_time ;
  bike_count++; //should eventually comment this out because will be incremented in SENSOR Interrupt

  String faultString = "";
  // //int faults array to string
  // for (int i = 0; i < numFaults; i++) {
  //   faultString += itoa(faults[i], &faultString); //is this correct?
  // }
  String return_var = ("{\"Failure_ID\": \"" + faultString + "\" ,\"Location_ID\": \"") + mac_address+ "\",\"count\": " + bike_count + "}";
  // String return_var = "Failure_ID=" + faults + "&Location_ID=" + mac_address + "&count=" + bike_count;
  // String return_var = "error=" + faults + "&mac=" + mac_address + "&time=" + elapsed_time + "&count=" + bike_count;
  
  return return_var;
}

// void flashLED(){
//   digitalWrite(LED_PIN_1,1);
//   delay(100);
//   digitalWrite(LED_PIN_1,0);
//   delay(100);
// }


//Flash Write
bool FlashWrite(const String& key, const String& value) {
  bool success = false;
  
  // Try to open preferences with retry mechanism
  int retryCount = 0;
  while (retryCount < 3) {
    if (preferences.begin("flash_mem", false)) {
      success = preferences.putString(key.c_str(), value);
      preferences.end();
      if (success) {
        Serial.printf("String written to 'flash_mem' namespace: %s\n", key.c_str());
      } else {
        Serial.printf("Failed to write to flash: %s\n", key.c_str());
      }
      break;
    } else {
      Serial.printf("Failed to open 'flash_mem' namespace, retry %d\n", retryCount + 1);
      retryCount++;
      delay(100); // Short delay before retry
    }
  }
  
  return success;
}
 
// Flash Read
String FlashRead(const String& key) {
  String value = "";
  
  // Try to open preferences with retry mechanism
  int retryCount = 0;
  while (retryCount < 3) {
    if (preferences.begin("flash_mem", true)) {
      value = preferences.getString(key.c_str(), "");  // default value
      preferences.end();
      Serial.printf("Read from flash: %s = %s\n", key.c_str(), value.c_str());
      break;
    } else {
      Serial.printf("Failed to open 'flash_mem' namespace for reading, retry %d\n", retryCount + 1);
      retryCount++;
      delay(100); // Short delay before retry
    }
  }
  
  return value;
}

// Initialize flash storage - call this once at startup
bool initFlashStorage() {
  if (preferences.begin("flash_mem", false)) {
    Serial.println("Flash storage initialized successfully");
    return true;
  } else {
    Serial.println("Failed to initialize flash storage");
    return false;
  }
}

//TODO:
bool writeMacAddrToFlash() {
  bool success = true;
  FlashWrite("monitor_id", mac_address);
  if (!FlashWrite("monitor_id", mac_address)) {
    Serial.println("Failed to write 'monitor_id' to flash");
    success = false;
  }
  
  if (!FlashWrite("firmware_v", (String)(firmware_v))) {
    Serial.println("Failed to write 'firmware_v' to flash");
    success = false;
  }
  if (!success){
    return false;
  }

  if (!FlashWrite("initialized", "true")) {
    Serial.println("Failed to write 'initialized' to flash");
    success = false;
  }
  
  return success;
}

bool postToServer() {
  // post all accumulated data to the server 

  // check cellular connection
  // sim7080g.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // RX, TX
  // power_on_sim7080g();
  Serial.printf("Going to POST to server\n");
  uint8_t currentFaults = checkFaults(start_time);
  if (currentFaults) {
    Serial.print("There are faults");
  }
  // try to POST to server
  prep_and_post_comms(aggregateData());
  
  // get time from server

  // see if firmware needs update

  // if any of the above fail, just set the timer and go back to sleep

  // set wakeup timer
  // esp_sleep_enable_timer_wakeup(TIME_BTWN_POST);

  return true;
}

void getFromServer(){
  // get initial monitor info 

}


int cur_time;
void setup() {
  cur_time = time(NULL);
  Serial.begin(115200);
  delay(1000);
  // Serial.print("I'm awake!");

  // // Initialize OTA functionality
  // if (!initOTA()) {
  //   Serial.println("Failed to initialize OTA functionality");
  // } else {
  //   Serial.println("OTA functionality initialized successfully");
    
  //   // Check if we're running from an OTA partition
  //   esp_ota_img_states_t currentPartition = getRunningPartition();
  //   if (currentPartition == ESP_OTA_IMG_OTA_0 || currentPartition == ESP_OTA_IMG_OTA_1) {
  //     Serial.println("Running from OTA partition");
  //     clearUpdatePending(); // Clear the update pending flag
  //   }
    
  //   // Check for updates if we're not already in the middle of an update
  //   if (!isUpdatePending()) {
  //     Serial.println("Checking for firmware updates...");
  //     if (checkForUpdates(FIRMWARE_VERSION)) {
  //       Serial.println("New firmware version available, starting update...");
  //       if (performOTAUpdate()) {
  //         Serial.println("OTA update initiated successfully");
  //       } else {
  //         OTAUpdateStatus status = getOTAStatus();
  //         Serial.printf("OTA update failed: %s\n", status.error.c_str());
  //       }
  //     } else {
  //       Serial.println("No firmware updates available");
  //     }
  //   } else {
  //     Serial.println("Update pending flag detected, skipping update check");
  //   }
  // }

  // // Initialize flash storage
  // if (!initFlashStorage()) {
  //   Serial.println("Critical error: Could not initialize flash storage");
  //   // You might want to add recovery logic here
  // }


  //get data from SENSOR_SDA_PIN
    int numNewBikeSDA = 3;
  
  // get wake up reason 
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // check wake up reason
  switch (wakeup_reason){
    case ESP_SLEEP_WAKEUP_GPIO:
      //check which GPIO woke the ESP32 up

      // uint64_t wakeup_pin_mask = esp_sleep_get_gpio_wakeup_status();
      // int wakeup_pin = __builtin_ffsll(wakeup_pin_mask) - 1;
      // Serial.print("Woke up from sensor pin high \n");

      sensorInterrupt(start_time, numNewBikeSDA, testMonID, rtcArray, RTCArrayIndex);
      cur_time = time(NULL) - cur_time;

    

      Serial.print("Woke up from sensor pin high \n");
      RTCArrayIndex = RTCArrayIndex + 1;
      // RTCArrayIndex++;

      // flashLED();
      // flashLED();
      // flashLED();
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      //POST data time
      // Serial.print("Woke up from timer\n");
      // flashLED();
      break;
    default: //REBOOT/SETUP state
      //no wakeup reason. must be a reboot/setup
      // Serial.print("Setting up\n");
      start_time = time(NULL);
      RTCArrayIndex = 0;
      
      // CONFIG_ESP_TASK_WDT_TIMEOUT_S = oneDaytoSec;//Task Watchdog timeout period (in seconds)
  }
  // if (FlashRead("initialized") == ""){
  //   writeMacAddrToFlash();
  //   // if (!writeMacAddrToFlash()) {
  //   //   Serial.println("Failed to write initial data to flash. Will retry on next boot.");
  //   // }
  // }
  // esp_log_level_set("*", ESP_LOG_NONE);  // Turn off all logs

}


void loop() {
  cur_time = time(NULL) - cur_time;
  Serial.print("I'm in loop \n\n");
  Serial.printf("i've spent %d time to wake up and record data: ", cur_time);
  // Serial.print( time(NULL));

  // Serial.print("Woke up and reading with monitor id of " + FlashRead("monitor_id") + "\n");
  // Serial.print("firmware version: " + FlashRead("firmware_v") + "\n");
  Serial.print("\n");
  for (int i = 0; i < 5; i++) {
    // Serial.printf("\n rtc arr entry%d id: %s, timestamp: %s, count: %d, faults: %x", i, rtcArray[i].MonitorID, rtcArray[i].timestamp, rtcArray[i].bike_count, rtcArray[i].faults);
    Serial.printf("rtc value at %d: %d\n", i, rtcArray[i]);
  }
  Serial.printf("My next index is: %ld\n", RTCArrayIndex);

  // for (int i = 0 ;i < 10; i++){
  //   Serial.print("%d\n", rtcArray[i].bike_count);
  // }

  // set up wakeup from sensor pin (4)
  pinMode(SENSOR_DETECT_PIN, INPUT_PULLDOWN);
  esp_deep_sleep_enable_gpio_wakeup(1 << SENSOR_DETECT_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
  gpio_set_direction((gpio_num_t)SENSOR_DETECT_PIN, GPIO_MODE_INPUT);


  // set up wake up from timer (5 seconds)
  esp_sleep_enable_timer_wakeup(20000000); //this should be changed to once a day
  //trigger to setting and sending  all RTC data
  if (cur_time == 1){
    Serial.print("starting deep sleep now\n");
    // go to deep sleep 
    esp_deep_sleep_start();
  } else {
    while(true);
  }

}



