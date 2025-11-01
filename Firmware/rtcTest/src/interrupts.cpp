#include "interrupts.h"
#include "faultChecks.h"
#include "comms.h"
#include "ota_updates.h"
#include <cstring>
// #include <ArduinoJson.h>


RTC_DATA_ATTR int total_bike_count;
// RTC_DATA_ATTR int RTCArrayIndex ; //loop around (modulo memSize(RTC)/ memSize() )
const int maxNumRTCEntries = 10; //need to actually calculate, 100 doesnt fit
const int monitorIDStrLen = 20; //20 hexadecimal characters (overcalculate len)
const int timestampStrLen = 20; //actually calculate

typedef struct {
    char MonitorID[monitorIDStrLen]; 
    char timestamp[timestampStrLen];
    uint8_t bike_count;
    uint8_t faults; 
} RTCData;

// RTC_DATA_ATTR 
// RTCData rtcArray[maxNumRTCEntries];
//seems to have issues saving to rtc if its not a primitive
// RTC_DATA_ATTR uint8_t rtcArray[maxNumRTCEntries];

/*
SETUP/REBOOT/RESET Interrupt
*/
void rebootInterrupt() {

}
//


/*
SENSOR Interrupt - Recording Data
*/
int sensorInterrupt(time_t startTime, int numNewBikes, char deviceMonitorID[], uint8_t rtcArray[], uint32_t RTCArrayIndex) {
    Serial.print("I'm in sensor interrupt now\n");
    
    
    //change to peak_find_algo calc for bike_count
    total_bike_count += numNewBikes;

    //reset SENSOR CHECK watchdog timer

    //add to timestampRTCArray()??
    time_t timeElapsed = time(NULL) - startTime;
        
    //get current time
    // struct timeval tv_now; //want tv_sec to see what time 
    // gettimeofday(&tv_now, NULL);
    // char *currentDateTime = ctime(&tv_now.tv_sec); 
    //formats it as "Tue Oct 11 09:28:58 2022"
    
    //for debugging
    // Serial.print("gettime success and now = %ld\n", tv_now.tv_sec); 
    // ESP_LOGI(TAG, "myDate: %s now %ld\n", currentDateTime, tv_now.tv_sec);
    // Serial.print("at myDate: %s, total bike count is %d and numNewBikes is %d", currentDateTime, bike_count, numNewBikes);

    RTCData rtcEntry;
    bool success = 0;
    // strcpy(rtcEntry.MonitorID, deviceMonitorID);
    // strcpy(rtcEntry.timestamp, currentDateTime);
    rtcEntry.bike_count = numNewBikes;
    rtcEntry.faults = faults;
    //  = {
    //     .MonitorID = {"hi"},
    //     .timestamp = currentDateTime,
    //     .bike_count = numNewBikes,
    //     .faults = faults,
    // };

    
    if (RTCArrayIndex < 10){
        rtcArray[RTCArrayIndex] = 5; //args: (struct *ptimeval, void* ptimezone)
        success = 1;
    }
    // Serial.printf("current entry: %s, %s, %d, %x", rtcEntry.MonitorID, rtcEntry.timestamp, rtcEntry.bike_count, rtcEntry.faults);
    
    // RTCArrayIndex = (RTCArrayIndex + 1) % maxNumRTCEntries;
    // Serial.print(RTCArray[0]);
    // Serial.print(RTCArray[1]);
    // Serial.print(RTCArray[2]);
    // for (int i = 0; i < 5; i++) {
    //     // Serial.printf("\n rtc arr entry%d id: %s, timestamp: %s, count: %d, faults: %x", i, rtcArray[i].MonitorID, rtcArray[i].timestamp, rtcArray[i].bike_count, rtcArray[i].faults);
    //     Serial.printf("rtc value at %d: %d\n", i, rtcArray[i]);
    // }
    // Serial.printf("Done with sensor interrupt\n");
    return success;
}

/*
TIMER Interrupt - POSTing to Server
*/
void postInterrupt() {
    // Serial.println("Executing postInterrupt - sending data to server");
    
    // // Aggregate data to send to server
    // String dataToPost = aggregateData();
    
    // // Send data to server using cellular communication
    // prep_and_post_comms(dataToPost);
    
    // // Get the response from the server
    // String serverResponse = getServerResponse();
    
    // if (serverResponse.length() > 0) {
    //     Serial.println("Received response from server: " + serverResponse);
        
    //     // Parse the JSON response
    //     StaticJsonDocument<512> doc;
    //     DeserializationError error = deserializeJson(doc, serverResponse);
        
    //     if (!error) {
    //         // Extract time value from response
    //         if (doc.containsKey("next_post_time")) {
    //             time_t nextPostTime = doc["next_post_time"];
    //             Serial.printf("Next post time: %ld\n", nextPostTime);
                
    //             // Store the next post time in RTC memory
    //             rtc_start_time = nextPostTime;
                
    //             // Set up the timer for the next post
    //             esp_sleep_enable_timer_wakeup((nextPostTime - time(NULL)) * 1000000ULL);
    //         }
            
    //         // Extract firmware version from response
    //         if (doc.containsKey("firmware_version")) {
    //             String newFirmwareVersion = doc["firmware_version"].as<String>();
    //             Serial.printf("Server firmware version: %s, Current firmware version: %.1f\n", 
    //                          newFirmwareVersion.c_str(), firmware_v);
                
    //             // Compare with current firmware version
    //             float newVersion = newFirmwareVersion.toFloat();
    //             if (newVersion > firmware_v) {
    //                 Serial.println("New firmware version available, initiating OTA update");
                    
    //                 // Store the new firmware version in RTC memory
    //                 firmware_v = newVersion;
                    
    //                 // Initiate OTA update
    //                 if (checkForUpdates(String(firmware_v).c_str())) {
    //                     Serial.println("Starting OTA update process");
    //                     if (performOTAUpdate()) {
    //                         Serial.println("OTA update initiated successfully");
    //                     } else {
    //                         OTAUpdateStatus status = getOTAStatus();
    //                         Serial.printf("OTA update failed: %s\n", status.error.c_str());
    //                     }
    //                 } else {
    //                     Serial.println("OTA update check failed or no update available");
    //                 }
    //             } else {
    //                 Serial.println("No firmware update needed");
    //             }
    //         }
    //     } else {
    //         Serial.printf("Failed to parse JSON response: %s\n", error.c_str());
    //     }
    // } else {
    //     Serial.println("No response received from server");
    // }
    
    // Reset the RTC timer for the next post
    // This is a fallback in case the server doesn't provide a next_post_time
    // esp_sleep_enable_timer_wakeup(TIME_BTWN_POST);
    
    // // Go back to deep sleep
    // Serial.println("Going back to deep sleep");
    // esp_deep_sleep_start();
}


