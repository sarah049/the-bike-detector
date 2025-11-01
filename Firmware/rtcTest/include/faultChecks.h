#ifndef FAULT_CHECKS_H
#define FAULT_CHECKS_H

//Faults & Errors 
extern bool SENSOR_ERR;
extern bool BATT_ERR; 
extern bool COMMS_ERR; 
extern bool SERVER_ERR; 

const int numFaults = 4;
extern uint8_t faults; //faults array, one-hot encoding
extern int oneDaytoSec;

extern time_t sensor_detect_timer_start;
extern time_t sensor_detect_timer_end;

extern int comms_result;

uint8_t checkFaults(time_t startTime);
bool checkSensor(time_t startTime);

#endif
