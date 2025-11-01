#include <HardwareSerial.h>
#include "faultChecks.h"
#include "posting.h"
#include "setup_init.h"
#include "esp_adc_cal.h"


// #define BATT_THRESH 3.3 //in Voltage [V]
// #define BATT_PIN 5 // GPIO pin to read SOC from
int num_avg = 10;

uint8_t faults = 0b00000000;

//Error One-Hot Codes: battery = +1, comms = +2, server = +4
uint8_t battErrOHCode = 0b0001;
uint8_t commsErrOHCode = 0b0010;
uint8_t serverErrOHCode = 0b0100;

float read_analog_voltage(int gpio_pin, int num_average);


/*
  BATTERY
*/

uint8_t checkBattery(uint8_t batt_pin, float batt_thresh, float buck_convert_voltage_factor) {
   //number of voltage readings to average over
  float currBattVolt = read_analog_voltage(batt_pin, num_avg) * buck_convert_voltage_factor; //get current battery voltage
  if (currBattVolt < batt_thresh) { 
    return battErrOHCode;
  }
  
  return 0; 
}
// float checkBattery(uint8_t batt_pin, float batt_thresh, float buck_convert_voltage_factor) {
//   // TODO: implement checking if the battery is working properly
//   int num_avg = 10;
//   float currBattVolt = read_analog_voltage(batt_pin, num_avg) * buck_convert_voltage_factor;
//   // if (currBattVolt < batt_thresh) BATT_ERR = true;
//   // else BATT_ERR = false;
//   faults |= battErrOHCode;
//   return currBattVolt; // battery threshold
// }



/* State of Charge (SOC)/ Battery Check */
/*----------------------------------
   read_analog_voltage Function:
   Arguments:
      int gpio_pin: GPIO pin number from which to read the analog value
      int num_average: Number of readings to average for smoothing the output
   Return value:
      float: Returns the average voltage reading from the specified GPIO pin in volts
*/
float read_analog_voltage(int batt_pin, int num_average)
{
  esp_adc_cal_characteristics_t adc_chars; // Structure to store ADC calibration characteristics
  int adc_readings_raw = 0;                // Variable to accumulate raw ADC readings

  // Loop to collect multiple ADC readings for averaging
  for (int i = 0; i < num_average; i++)
  {
    adc_readings_raw += analogRead(batt_pin); // Add current reading to total
    delayMicroseconds(10);                    // Tiny delay to allow for ADC settling
  }
  adc_readings_raw = adc_readings_raw / num_average; // Calculate average of readings

  // Determine the ADC unit based on the GPIO pin number
  // adc_unit_t current_adc = ADC_UNIT_1; // Default ADC unit
  adc_unit_t current_adc = ADC_UNIT_2; // Change ADC unit for specific pins, if necessary (BATT pin is 5, so yes it's needed)
  
  // Characterize the ADC at 11 dB attenuation and 12-bit width for accurate voltage conversion
  esp_adc_cal_characterize(current_adc, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  // Convert the average raw reading to voltage in volts, using calibration characteristics
  return (esp_adc_cal_raw_to_voltage(adc_readings_raw, &adc_chars) / 1000.0);
}




/*
  SERVER --> MAIN POST CODE COMBINED WITH COMMS
*/

// reutrns ONLY COMMS AND SERVER FAULT CODE
uint8_t checkCommsandServer(uint32_t rtc_start_time, String firmware_v, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor){
  // check cellular connection - is init? 
  // check handshake / ping to server
  //if there is a return json, then no server-err
  // TODO have comms return in posting.c and pass that along here 
  String init_val = FlashRead("initialized");
  uint8_t comms_server_err = postInitServer(init_val, firmware_v, rtc_start_time, batt_pin,  batt_thresh, buck_convert_voltage_factor);

  return comms_server_err;
}



uint8_t checkAllFaults(time_t startTime, String FIRMWARE_VERSION, uint8_t batt_pin, float batt_thresh, float buck_convert_voltage_factor){
  Serial.printf("Checking Faults now\n");
  
  float batt_volt = read_analog_voltage(batt_pin, num_avg) * buck_convert_voltage_factor;
  faults = 0;
  uint8_t batt_err = checkBattery(batt_pin, batt_thresh, buck_convert_voltage_factor);
  
  uint8_t comms_server_err = checkCommsandServer(startTime, FIRMWARE_VERSION,  batt_pin, batt_thresh, buck_convert_voltage_factor);

  // if(!) { //cellular connection fails, go to deep sleep for 2 min and try again
  //   // Enter sleep mode 
  //   // esp_sleep_enable_timer_wakeup(120000000); // Sleep for 2 min = 120 * 1000000 microseconds
  //   // esp_deep_sleep_start();
  // }

  return batt_err + comms_server_err + faults;

}