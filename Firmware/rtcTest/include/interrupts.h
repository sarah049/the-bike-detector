#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <Arduino.h>
#include <string.h>
// RTC Variables
extern int oneDaytoSec;

extern RTC_DATA_ATTR int setup_state; // 0 is brand new, 1 is already established with server
// extern RTC_DATA_ATTR int faultRTCArray[maxNumRTCEntries];
// extern RTC_DATA_ATTR char timestampRTCArray[maxNumRTCEntries];
extern RTC_DATA_ATTR int bike_count;

// MONITOR INFO
extern RTC_DATA_ATTR String mac_address;

void rebootInterrupt();
int sensorInterrupt(time_t startTime, int numNewBikes, char deviceMonitorID[], uint8_t rtcArray[], uint32_t RTCArrayIndex);
void postInterrupt(); 

#endif