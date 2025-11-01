#include <Arduino.h>
#include <string.h>
#include <HardwareSerial.h>
#include "comms.h"


#include "esp_adc_cal.h"
#include "driver/rtc_io.h"

// HardwareSerial sim7080g(1);

#define SENSOR_DETECT GPIO_NUM_4
#define SENSOR_DETECT_PIN_NUM 4
// RTC Variables
RTC_DATA_ATTR int setup_state = 0; // 0 is brand new, 1 is already established with server
RTC_DATA_ATTR int bike_count = 0;

// MONITOR INFO
RTC_DATA_ATTR String mac_address = "";

//TODO: assign GPIO pins to different sections
#define REBOOT_WAKEUP_PIN 0 // Pin to wakeup esp for reboot
#define REBOOT_BITMASK 0x00000000 // 0

// Sensor pins
#define SENSOR_WAKEUP_PIN 1 // Pin to wakeup esp from sensor
#define SENSOR_BITMASK 0x00000001 // 1
// + raw data info


// Battery pins
int BATT_IN = 5; // GPIO pin to read from

// debug pins
#define LED_PIN_1 8 // Pin to flash LED
#define LED_PIN_2 11 // Pin to flash LED

// General Startup Variables
#define TIME_BTWN_POST 3600000000 // time in us between posts, 3600000000 = 1 hr
#define BATT_THRESH 3 // TODO get battery voltage threshold

int action_to_do = 0; // 0 = setup/reboot, 1 = record data, 2 = post data

float buck_convert_voltage_factor = (9/5.1); // factor to "undivide" voltage divider to read battery voltage  
String faults = "0000";

float time_to_send_to_cellular = 20.0;
time_t start_time;
time_t elapsed_time;

/*
  SENSOR
*/
int checkSensor() {
  // TODO: implement checking if the sensor is working properly 
  // return 0 if working, 1 if fault
  return 0;
}

/*
  BATTERY
*/

int checkBattery() {
  // TODO: implement checking if the battery is working properly 
  // if (curBattVolt(BATT_IN, 5) < BATT_THRESH){
  //   return 1;
  // }
  return 0; // battery threshold
}


/*
  SERVER --> MAIN POST CODE COMBINED WITH COMMS
*/
int checkComms(){
  // check cellular connection
  return 0;
}

int checkServer(){
  // check handshake / ping to server
  return 0;
}



String aggregateData(){
  Serial.printf("Aggregating Data\n");
  // Serial.printf("Current faults: %s\n", faults);
  // "check faults"
  // getting time from start time
  elapsed_time = time(NULL) - start_time ;
  bike_count++;
  String return_var = "{\"Failure_ID\": \""+ faults + "\" ,\"Location_ID\": \"" + mac_address+ "\",\"count\": " + bike_count+"}";
  // String return_var = "Failure_ID=" + faults + "&Location_ID=" + mac_address + "&count=" + bike_count;
  // String return_var = "error=" + faults + "&mac=" + mac_address + "&time=" + elapsed_time + "&count=" + bike_count;
  
  return return_var;
}

void flash_led(){
  digitalWrite(LED_PIN_1,1);
  delay(100);
  digitalWrite(LED_PIN_1,0);
  delay(100);
}


void checkFaults(){
  Serial.printf("Checking Faults now\n");
  if (checkSensor()){
    faults[0] = '1';
  } else {
    faults[0] = '0';
  }

  if (checkBattery()){
    faults[1] = '1';
  } else {
    faults[1] = '0';
  }
  if (checkComms()){
    faults[2] = '1';
  } else {
    faults[2] = '0';
  }
  if (checkServer()){
    faults[3] = '1';
  } else {
    faults[3] = '0';
  }

}

bool postToServer() {
  // post all accumulated data to the server 

  // check cellular connection
  // sim7080g.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // RX, TX
  // power_on_sim7080g();
  Serial.printf("Going to POST to server\n");
  checkFaults();
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








void setup() {
  

  Serial.begin(115200);

  //setup time
  start_time = time(NULL);

  //LED 
  pinMode(LED_PIN_1,OUTPUT);
  // get mac address --> RTC
  mac_address = "68:67:25:07:76:e0";

}



void loop() {
  Serial.printf("Waiting for a bike\n");

  if (gpio_get_level(SENSOR_DETECT) == 1){
    Serial.printf("Bike Detected!\n");
    flash_led();
    flash_led();
    postToServer();
    digitalWrite(SENSOR_DETECT_PIN_NUM,0); //pin 4
  }

  elapsed_time = time(NULL) - start_time;

  // if (elapsed_time > time_to_send_to_cellular) {
  //   // post to server (increments bike count internally)
  //   postToServer();

  //   Serial.printf("sent to cellular");
  //   Serial.print(bike_count);

  //   elapsed_time = 0;
  //   start_time = time(NULL);

  // }


}



