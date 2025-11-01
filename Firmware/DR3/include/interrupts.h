#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <Arduino.h>
#include <string.h>

// struct sensorReturn;

// RTC Variables
extern int oneDaytoSec;
extern uint32_t sensor_start_time;
extern uint32_t sensor_arr_ind;
extern uint32_t sensor_post_time;

extern RTC_DATA_ATTR int setup_state; // 0 is brand new, 1 is already established with server
extern RTC_DATA_ATTR int bike_count;
// extern int maxNumRTCEntries;


// MONITOR INFO
extern RTC_DATA_ATTR char mac_address[];

bool rebootInterrupt(uint32_t startTime, String firmware_v, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE);
bool sensorInterrupt(uint32_t startTime, uint32_t rtcArray[], uint32_t arr_ind, gpio_num_t sensor_pin, long long int TIME_BTWN_POST_US, String FIRMWARE_VERSION, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE, uint32_t next_post_time, uint32_t time_btwn_post_s);
bool postInterrupt(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], long long int TIME_BTWN_POST, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE); 

#endif