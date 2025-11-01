#include "setup_init.h"
#include "interrupts.h"
#include "faultChecks.h"
#include "comms.h"
#include "posting.h"

/*ONLY INTERRUPT WILL CONTROL COMMS POWERGATE */
const uint16_t comms_power_delay = 1000;

const int maxNumRTCEntries = 2020; 
uint32_t sensor_start_time;
uint32_t sensor_arr_ind;
uint32_t sensor_post_time;


/*
SETUP/REBOOT/RESET Interrupt

This function checks all faults in the device and initializes the device (saving to flash memory)

To check the battery voltage, checkBattery is called. 
To check the comms 

*/
bool rebootInterrupt(uint32_t startTime, String firmware_v, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE) {
    // check faults    
    // printf("inside reboot interrupt sequence \n");
    // check battery voltage
    float cur_batt_volt = checkBattery(batt_pin, batt_thresh, buck_convert_voltage_factor);
    uint8_t cur_faults = 0;
    if (cur_batt_volt < batt_thresh) cur_faults = 1;

    // check initialization
    String cur_init_state = FlashRead("initialized");
    if (cur_init_state == ""){
        cur_init_state = "false";
    }
    bool init_ok = checkInit();

    // Check comms and server

    // Pull Comms power high
    digitalWrite(COMMS_POWERGATE, HIGH);
    delay(comms_power_delay);
    
    uint8_t comm_server_faults = postInitServer( cur_init_state, firmware_v, startTime, batt_pin,  batt_thresh, buck_convert_voltage_factor);
    cur_faults += comm_server_faults; 
    // check for initialization 
    
    // Pull Comms power low
    digitalWrite(COMMS_POWERGATE, LOW);

    // if device is initialized and there are no faults, 
    if (init_ok && (cur_faults == 0)){
        return true;
    } 
    return false;
}


/*
SENSOR Interrupt - Recording Data

returns if all bikes seen were sucessfully recorded. 

When the sensor interrupt is called, 
the current time is recorded. 

The function will then look for another bike pin signal for 2 seconds. 
If a new bike signal is detected, it'll record the time and repeat the process. (the 2 seconds
start from the last bike seen). 

Once it's been 2 seconds since the last bike seen, it evaluates if the current time is past
the time the device should post (aka the device missed the timer wakeup signal), 
we then attempt to post. 

Afterwards, we update the start time, next time to post, and current array index

*/
bool sensorInterrupt(uint32_t startTime, uint32_t rtcArray[], uint32_t arr_ind, gpio_num_t sensor_pin, long long int TIME_BTWN_POST_US, String FIRMWARE_VERSION, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE, uint32_t next_post_time, uint32_t time_btwn_post_s) {
    // find out time stamp
    uint32_t last_bike_time = time(NULL);
    bool success = 0;
    bool lastPinState = 0;
    bool pinState = 1;
    bool flag = 1;


    Serial.printf("Starting 2 sec\n");
    // while loop to check for more bikes going by 
    while (flag || (time(NULL) - last_bike_time < 2)) { //2 seconds since last SENSOR_DETECT_PIN high
        if (flag) Serial.printf("Inside 2 sec loop\n");
        pinState = gpio_get_level(sensor_pin);
        bool rising_edge = (pinState && !lastPinState);
        // if it's the first bike seen or seeing a rising change in the 
        // bike sensro pin, we record a bike/time
        if (flag || rising_edge) {
            flag = 0;
            if (arr_ind < maxNumRTCEntries){
                last_bike_time = time(NULL);
                rtcArray[arr_ind] = last_bike_time;
                arr_ind = (arr_ind + 1) % maxNumRTCEntries;
                success = 1;
                
            }
        }
        lastPinState = pinState;
    }

    // in case timer went off when a bike went by, post if it's time 
    if (time(NULL) > next_post_time){
        bool post_success = postInterrupt(startTime, FIRMWARE_VERSION, arr_ind, rtcArray, TIME_BTWN_POST_US, batt_pin, batt_thresh, buck_convert_voltage_factor, COMMS_POWERGATE);

        if (post_success){
            // reset srray index, start time, and next post time
            arr_ind = 0;
            startTime = time(NULL);
            next_post_time = startTime + time_btwn_post_s;
        } else{
            // if failed post, set next post time to a time_btwn_post_s period more
            next_post_time += time_btwn_post_s;
        }

        esp_sleep_enable_timer_wakeup(TIME_BTWN_POST_US);
    }

    // Update start time, array index, and next post time 
    sensor_start_time = startTime;
    sensor_arr_ind = arr_ind;
    sensor_post_time = next_post_time;
    
    return success;
}

/*
TIMER Interrupt - POSTing to Server

returns of POST was successful or not

*/
bool postInterrupt(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], long long int TIME_BTWN_POST, uint8_t batt_pin, float batt_thresh, float buck_convert_voltage_factor, gpio_num_t COMMS_POWERGATE) {
    
    // Pull Comms power high
    digitalWrite(COMMS_POWERGATE, HIGH);
    delay(comms_power_delay);

    // post data to server
    int post_fault = postDataToServer(rtc_start_time, FIRMWARE_VERSION, RTCArrayIndex, rtcArray, TIME_BTWN_POST, batt_pin, batt_thresh, buck_convert_voltage_factor);

    // Pull Comms power low
    digitalWrite(COMMS_POWERGATE, LOW);

    if (post_fault == 0){
        return true;
    }
    return false;
}


