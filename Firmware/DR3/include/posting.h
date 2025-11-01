#ifndef POSTING_H
#define POSTING_H

#include <Arduino.h>
#include <string.h>

struct ChunkedData;

// functions
uint8_t postDataToServer(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], long long int TIME_BTWN_POST, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor);
ChunkedData aggregateData(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor);
uint8_t postInitServer(String init_val, String FIRMWARE_VERSION, uint32_t rtc_start_time,uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor);

#endif