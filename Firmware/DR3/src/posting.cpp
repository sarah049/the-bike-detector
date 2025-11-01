#include "setup_init.h"
#include "interrupts.h"
#include "faultChecks.h"
#include "comms.h"
#include <cstring>
// #include <ArduinoJson.h>

struct ChunkedData {
    String baseJson;
    String* timestampChunks;
    int numChunks;
};

ChunkedData aggregateData(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], uint8_t batt_pin, float batt_thresh, float buck_convert_voltage_factor) {
    Serial.printf("Aggregating Data\n");
    
    //TODO Check faults
    String batt_volt = (String)(checkBattery(batt_pin, batt_thresh, buck_convert_voltage_factor)); 
    String faultString = (String)(checkAllFaults(rtc_start_time, FIRMWARE_VERSION, batt_pin, batt_thresh, buck_convert_voltage_factor)); // TODO
    String mac_address = getMacAddress();
    
    // Create the base JSON structure without the time list
    String baseJson = "{\"Monitor_ID\": \"" + mac_address + 
                     "\",\"Firmware_V\": \"" + FIRMWARE_VERSION + 
                     "\",\"Failure_ID\": \"" + faultString + 
                     "\",\"Batt_Volt\": \"" + batt_volt + 
                     "\",\"Start_Time\": \"" + rtc_start_time + 
                     "\",\"Time_List\": \"[";
    
    // Calculate chunks based on our napkin math
    const int MAX_TIMESTAMPS_PER_CHUNK = 20;  // Based on our calculation
    int numChunks = (RTCArrayIndex + MAX_TIMESTAMPS_PER_CHUNK - 1) / MAX_TIMESTAMPS_PER_CHUNK;
    String* timestampChunks = new String[numChunks];
    
    for (int chunk = 0; chunk < numChunks; chunk++) {
        int startIdx = chunk * MAX_TIMESTAMPS_PER_CHUNK;
        int endIdx = min(startIdx + MAX_TIMESTAMPS_PER_CHUNK, (int)(RTCArrayIndex));
        
        String chunkStr;
        for (int i = startIdx; i < endIdx; i++) {
            chunkStr += String(rtcArray[i]);
            if (i < endIdx - 1) {
                chunkStr += ", ";
            }
        }
        timestampChunks[chunk] = chunkStr;
    }
    
    ChunkedData result = {
        baseJson,
        timestampChunks,
        numChunks
    };
    
    return result;
}
    
  
  uint8_t postDataToServer(uint32_t rtc_start_time, String FIRMWARE_VERSION, uint32_t RTCArrayIndex, uint32_t rtcArray[], long long int TIME_BTWN_POST, uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor) {
    // post all accumulated data to the server 
  
    Serial.printf("Going to POST to server\n");

    // try to POST to server
    ChunkedData data = aggregateData(rtc_start_time, FIRMWARE_VERSION, RTCArrayIndex, rtcArray, batt_pin, batt_thresh, buck_convert_voltage_factor);
    
    // Send each chunk
    int comm_result_errorCode = 0;
    for (int i = 0; i < data.numChunks; i++) {
        String fullJson = data.baseJson + data.timestampChunks[i] + "]\"}";
        comm_result_errorCode = prep_and_post_comms(fullJson, false); // Only initialize on first chunk
        if (comm_result_errorCode < 0) break;
    }
    
    // Clean up
    delete[] data.timestampChunks;
    
    // see if firmware needs update TODO OTA, 
    /* TODO: OTA */
  
    // if any of the above fail, just set the timer and go back to sleep
    // set wakeup timer
    esp_sleep_enable_timer_wakeup(TIME_BTWN_POST);

    String comm_result = getServerResponse();

    // comms = +2
    // server = +4
    if (comm_result != ""){
      return 2;
    } else if (comm_result_errorCode != 0){
      return 4;
    }

  
    return 0;
  }

  
  uint8_t postInitServer(String init_val, String FIRMWARE_VERSION, uint32_t rtc_start_time,uint8_t batt_pin, float batt_thresh,  float buck_convert_voltage_factor){
    // The restart / handshake post

    // Monitor ID, firmware version, and initialization should be done before this. 
    float batt_volt = checkBattery(batt_pin, batt_thresh, buck_convert_voltage_factor);
    // String faultString = (String)(checkAllFaults(rtc_start_time, batt_pin, batt_thresh, buck_convert_voltage_factor)); // TODO
    String batt_fault = "0";
    if (batt_volt < batt_thresh){
      batt_fault = "1";
    }

    String mac_address = getMacAddress();

    String sendCheck = "{\"Monitor_ID\": \"" + mac_address + "\" ,\"Firmware_V\": \"" + FIRMWARE_VERSION +  "\" ,\"Initialized\": \"" + init_val +   "\" ,\"Fault_ID\": \"" + batt_fault + "\"}";
    int comm_result_errorCode = prep_and_post_comms(sendCheck, true);

    String comm_result = getServerResponse();

    // comms = +2
    // server = +4
    if (comm_result != ""){
      return 2;
    } else if (comm_result_errorCode != 0){
      return 4;
    }

    // returns COMMS AND SERVER FAULTS
    return 0;
  }
  

  