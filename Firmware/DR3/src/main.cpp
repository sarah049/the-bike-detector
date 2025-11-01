#include <Arduino.h>
// #include <string.h>
#include <HardwareSerial.h>
#include "comms.h"
#include "faultChecks.h"
#include "interrupts.h"
#include "setup_init.h"
#include "posting.h"
#include "setup_init.h"

#include "driver/rtc_io.h"

//GPIO Pin Assignments
#define SENSOR_DETECT_PIN   GPIO_NUM_4
#define BATT_PIN            GPIO_NUM_0
#define SENSOR_SCL_PIN      GPIO_NUM_2
#define SENSOR_SDA_PIN      GPIO_NUM_3
#define COMMS_POWERGATE     GPIO_NUM_7
#define LED_PIN             GPIO_NUM_6
#define DEBUG_PIN           GPIO_NUM_1
#define RESET_PIN           GPIO_NUM_9
// D+ and D- pins are for the USB and are not used so flash from USB-C is possible
// #define DPLUS_PIN GPIO_NUM_18
// #define DMINUS_PIN

// debug pins
#define LED_PIN_1 GPIO_NUM_8 // Pin to flash LED
// #define LED_PIN_2 11 // Pin to flash LED

// OTA update configuration
#define FIRMWARE_VERSION "1.0.0"

// General Startup Variables
#define TIME_BTWN_POST_US 3600000000 // time in us between posts, 3600* 1000000 = 1 hr
#define TIME_BTWN_POST_S 3600

#define BATT_THRESH 6.4 

// RTC variables
/* rtc_start_time: the time of the last post. 

times in the rtcArray will be the absolute times a bike was seen. 
rtc_start_time is to help the server-side compute when exactly a bike was seen

For example, 
The last post was 10:00:00 AM May 5
The start time is 5
The device saw a bike at time 10, 23, and 65

This means a bike was seen at 10:00:10 AM May 5, 10:00:23 AM May 5, 10:01:05 AM May 5

*/
RTC_DATA_ATTR uint32_t rtc_start_time;
RTC_DATA_ATTR uint32_t rtc_time_to_post;
RTC_DATA_ATTR uint32_t rtcArray[2020];
RTC_DATA_ATTR uint32_t RTCArrayIndex;

// Power 
float buck_convert_voltage_factor = (3.13); // factor to "undivide" voltage divider to read battery voltage  


bool setup_ok = false;
bool post_success = false;


void setup() {
  Serial.begin(115200);

  // powergate comms 
  pinMode(COMMS_POWERGATE, OUTPUT);
  digitalWrite(COMMS_POWERGATE, LOW);

  // get wake up reason 
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // check wake up reason
  switch (wakeup_reason){
    case ESP_SLEEP_WAKEUP_GPIO:
      Serial.print("Woke up from sensor pin high \n");

      // only pin 4 will cause this (RST button will go to default case)
      sensorInterrupt(rtc_start_time, rtcArray, RTCArrayIndex, SENSOR_DETECT_PIN, TIME_BTWN_POST_US, FIRMWARE_VERSION, BATT_PIN, BATT_THRESH, buck_convert_voltage_factor, COMMS_POWERGATE, rtc_time_to_post, TIME_BTWN_POST_S);
      
      // update rtc variables
      //array is already a pointer, so rtcArray has been updated
      RTCArrayIndex = sensor_arr_ind;
      rtc_start_time = sensor_start_time;
      rtc_time_to_post = sensor_post_time;

      // then go back to sleep
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.printf("in timer wakeup\n");
      // POST data time
      // check faults inside of post interrupt 
      post_success = postInterrupt(rtc_start_time, FIRMWARE_VERSION, RTCArrayIndex, rtcArray, TIME_BTWN_POST_US, BATT_PIN, BATT_THRESH, buck_convert_voltage_factor, COMMS_POWERGATE);

      // reset rtcindex and start time if successful POST
      if (post_success){
        RTCArrayIndex = 0;
        rtc_start_time = time(NULL);
        rtc_time_to_post = rtc_start_time + TIME_BTWN_POST_S;
      } else{
        rtc_time_to_post += TIME_BTWN_POST_S;
      }
      esp_sleep_enable_timer_wakeup(TIME_BTWN_POST_US);
      
      break;
    default: //REBOOT/SETUP state
      Serial.printf("in reboot\n");
      //no wakeup reason. must be a reboot/setup
      setup_ok = rebootInterrupt(rtc_start_time, FIRMWARE_VERSION, BATT_PIN, BATT_THRESH, buck_convert_voltage_factor, COMMS_POWERGATE); 

      // set the new start time and new post time
      rtc_start_time = time(NULL);
      rtc_time_to_post = rtc_start_time + TIME_BTWN_POST_S;
      RTCArrayIndex = 0;
      esp_sleep_enable_timer_wakeup(TIME_BTWN_POST_US);
      
  }

}


void loop() {
  // set up wakeup from sensor pin (4)
  pinMode(SENSOR_DETECT_PIN, INPUT_PULLDOWN);
  esp_deep_sleep_enable_gpio_wakeup(1 << SENSOR_DETECT_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
  gpio_set_direction((gpio_num_t)SENSOR_DETECT_PIN, GPIO_MODE_INPUT);
 
  // go to deep sleep
  esp_deep_sleep_start();

}



