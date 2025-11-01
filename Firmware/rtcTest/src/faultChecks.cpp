#include <HardwareSerial.h>
#include "faultChecks.h"
#include "esp_adc_cal.h"


#define BATT_THRESH 3.3 //in Voltage [V]
#define BATT_PIN 5 // GPIO pin to read SOC from

int oneDaytoSec = 86400; //oneDaytoSec = 24*60*60 *60
bool SENSOR_ERR = false;
bool BATT_ERR = false; 
bool COMMS_ERR = false; 
bool SERVER_ERR = false;

uint8_t faults = 0b00000000;
uint8_t sensorErrOHCode = 0b0001;
uint8_t battErrOHCode = 0b0010;
uint8_t commsErrOHCode = 0b0100;
uint8_t serverErrOHCode = 0b1000;

float read_analog_voltage(int gpio_pin, int num_average);

/*
  SENSOR CHECK
*/
bool checkSensor(time_t startTime) {
  // TODO: implement checking if the sensor is working properly 
  // return 0 if working, 1 if fault

  time_t timeElapsed = time(NULL) - startTime;


  //send I2C transmit command to ESP32 
  int esp32ReceiveSensorSignal = 0; //should be set to ESP32 I2C receive response
    if (!esp32ReceiveSensorSignal|| timeElapsed > oneDaytoSec) { //check if timeElapsed is actually in terms of seconds
    SENSOR_ERR = true; //raises SENSOR_ERR 
  }
  else SENSOR_ERR = false;
  faults |= sensorErrOHCode;
  return SENSOR_ERR;

}

/*
  BATTERY
*/

bool checkBattery() {
  // TODO: implement checking if the battery is working properly
  int num_avg = 10;
  float currBattVolt = read_analog_voltage(BATT_PIN, num_avg);
  if (currBattVolt < BATT_THRESH) BATT_ERR = true;
  else BATT_ERR = false;
  faults |= battErrOHCode;
  return BATT_ERR; // battery threshold
}


/* State of Charge (SOC)/ Battery Check */
/*----------------------------------
   read_analog_voltage Function:
   Arguments:
      int gpio_pin: GPIO pin number from which to read the analog value
      int num_average: Number of readings to average for smoothing the output
   Return value:
      float: Returns the average voltage reading from the specified GPIO pin in volts
*/
float read_analog_voltage(int gpio_pin, int num_average)
{
  esp_adc_cal_characteristics_t adc_chars; // Structure to store ADC calibration characteristics
  int adc_readings_raw = 0;                // Variable to accumulate raw ADC readings

  // Loop to collect multiple ADC readings for averaging
  for (int i = 0; i < num_average; i++)
  {
    adc_readings_raw += analogRead(gpio_pin); // Add current reading to total
    delayMicroseconds(10);                    // Tiny delay to allow for ADC settling
  }
  adc_readings_raw = adc_readings_raw / num_average; // Calculate average of readings

  // Determine the ADC unit based on the GPIO pin number
  adc_unit_t current_adc = ADC_UNIT_1; // Default ADC unit
  if (gpio_pin == BATT_PIN) //BATT_PIN is 5
  {
    current_adc = ADC_UNIT_2; // Change ADC unit for specific pins, if necessary
  }

  // Characterize the ADC at 11 dB attenuation and 12-bit width for accurate voltage conversion
  esp_adc_cal_characterize(current_adc, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  // Convert the average raw reading to voltage in volts, using calibration characteristics
  return (esp_adc_cal_raw_to_voltage(adc_readings_raw, &adc_chars) / 1000.0);
}

/*
  SERVER --> MAIN POST CODE COMBINED WITH COMMS
*/
bool checkComms(int commStatus){
  // check cellular connection
  if (!commStatus) COMMS_ERR = true; //commStatus == 0 means error
  else COMMS_ERR = false;
  faults |= commsErrOHCode;
  return COMMS_ERR;
}

bool checkServer(){
  // check handshake / ping to server
  //if there is a return json, then no server-err
  return 0;
}


uint8_t checkFaults(time_t startTime){
  Serial.printf("Checking Faults now\n");
  
  checkSensor(startTime);
  checkBattery();
  if(!checkComms(comms_result)) { //cellular connection fails, go to deep sleep for 2 min and try again
    // Enter sleep mode 
    // esp_sleep_enable_timer_wakeup(120000000); // Sleep for 2 min = 120 * 1000000 microseconds
    // esp_deep_sleep_start();
  }
  return faults;

}