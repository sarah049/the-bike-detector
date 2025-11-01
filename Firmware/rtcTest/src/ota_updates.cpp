// #include <Arduino.h>
// #include <esp_ota_ops.h>
// #include <esp_system.h>
// #include <esp_log.h>
// #include <nvs_flash.h>
// #include <string.h>
// #include <Preferences.h>

// #include "ota_updates.h"
// #include "comms.h"
// #include "cellLib.h"

// static const char *TAG = "OTA_UPDATES";

// // OTA update status
// static bool otaInProgress = false;
// static int otaProgress = 0;
// static String otaError = "";

// // Cellular communication pins
// #define RX_PIN 6
// #define TX_PIN 7

// // OTA server configuration
// const char* otaServer = "efpi-10.mit.edu";  // Server URL
// const int otaPort = 80;                     // Default HTTP port
// const char* otaVersionEndpoint = "/tunnel_anacamba/version"; // Version check endpoint
// const char* otaFirmwareEndpoint = "/tunnel_anacamba/firmware"; // Firmware download endpoint

// // Function to check if a new firmware version is available
// bool checkForUpdates(const char* currentVersion) {
//     Serial.println("Checking for firmware updates via cellular...");
    
//     // Initialize cellular module
//     if (!cellLibInit(TX_PIN, RX_PIN)) {
//         Serial.println("Failed to initialize cellular module for OTA check");
//         otaError = "Failed to initialize cellular module";
//         return false;
//     }
    
//     // Create HTTP connection to check version
//     HttpConnection connection(otaServer, otaPort);
    
//     // Prepare version check request
//     String versionQuery = "version=" + String(currentVersion);
    
//     // Send GET request to check version
//     int result = connection.getJson(versionQuery, otaVersionEndpoint);
    
//     if (result < 0) {
//         Serial.println("Failed to check for updates: " + String(result));
//         otaError = "Failed to check for updates: " + String(result);
//         return false;
//     }
    
//     // Parse response to determine if update is needed
//     // This is a simplified example - you'll need to adapt this to your server's response format
//     String response = connection.getResponse();
//     bool updateAvailable = false;
    
//     if (response.indexOf("new_version") >= 0) {
//         // Extract version from response and compare with current version
//         // This is a simplified example - you'll need to implement proper version comparison
//         if (response.indexOf(currentVersion) < 0) {
//             updateAvailable = true;
//         }
//     }
    
//     if (updateAvailable) {
//         Serial.println("New firmware version available");
//     } else {
//         Serial.println("No firmware updates available");
//     }
    
//     return updateAvailable;
// }

// // Function to perform OTA update
// bool performOTAUpdate() {
//     if (otaInProgress) {
//         Serial.println("OTA update already in progress");
//         return false;
//     }
    
//     otaInProgress = true;
//     otaProgress = 0;
//     otaError = "";
    
//     Serial.println("Starting OTA update via cellular...");
    
//     // Initialize cellular module
//     if (!cellLibInit(TX_PIN, RX_PIN)) {
//         Serial.println("Failed to initialize cellular module for OTA update");
//         otaError = "Failed to initialize cellular module";
//         otaInProgress = false;
//         return false;
//     }
    
//     // Create HTTP connection to download firmware
//     HttpConnection connection(otaServer, otaPort);
    
//     // Prepare firmware download request
//     String firmwareQuery = "action=download";
    
//     // Send GET request to download firmware
//     int result = connection.getBinary(firmwareQuery, otaFirmwareEndpoint);
    
//     if (result < 0) {
//         Serial.println("Failed to download firmware: " + String(result));
//         otaError = "Failed to download firmware: " + String(result);
//         otaInProgress = false;
//         return false;
//     }
    
//     // Get the firmware binary data
//     uint8_t* firmwareData = connection.getBinaryData();
//     size_t firmwareSize = connection.getBinarySize();
    
//     if (firmwareData == NULL || firmwareSize == 0) {
//         Serial.println("No firmware data received");
//         otaError = "No firmware data received";
//         otaInProgress = false;
//         return false;
//     }
    
//     Serial.printf("Received firmware binary, size: %d bytes\n", firmwareSize);
    
//     // Get the next update partition
//     const esp_partition_t *next_update = esp_ota_get_next_update_partition(NULL);
//     if (next_update == NULL) {
//         Serial.println("No OTA partition available for update");
//         otaError = "No OTA partition available for update";
//         otaInProgress = false;
//         return false;
//     }
    
//     Serial.printf("Writing to partition: %s\n", next_update->label);
    
//     // Open the update partition
//     esp_ota_handle_t ota_handle;
//     esp_err_t err = esp_ota_begin(next_update, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
//     if (err != ESP_OK) {
//         Serial.printf("esp_ota_begin failed: %s\n", esp_err_to_name(err));
//         otaError = "Failed to begin OTA update: " + String(esp_err_to_name(err));
//         otaInProgress = false;
//         return false;
//     }
    
//     // Write the firmware data to the OTA partition
//     size_t bytes_written = 0;
//     const size_t chunk_size = 4096;
    
//     while (bytes_written < firmwareSize) {
//         size_t to_write = min(chunk_size, firmwareSize - bytes_written);
//         err = esp_ota_write(ota_handle, firmwareData + bytes_written, to_write);
        
//         if (err != ESP_OK) {
//             Serial.printf("esp_ota_write failed: %s\n", esp_err_to_name(err));
//             otaError = "Failed to write OTA data: " + String(esp_err_to_name(err));
//             esp_ota_abort(ota_handle);
//             otaInProgress = false;
//             return false;
//         }
        
//         bytes_written += to_write;
//         otaProgress = (bytes_written * 100) / firmwareSize;
//         Serial.printf("OTA update progress: %d%%\n", otaProgress);
//     }
    
//     // Finish the OTA update
//     err = esp_ota_end(ota_handle);
//     if (err != ESP_OK) {
//         Serial.printf("esp_ota_end failed: %s\n", esp_err_to_name(err));
//         otaError = "Failed to end OTA update: " + String(esp_err_to_name(err));
//         otaInProgress = false;
//         return false;
//     }
    
//     // Set the boot partition
//     err = esp_ota_set_boot_partition(next_update);
//     if (err != ESP_OK) {
//         Serial.printf("esp_ota_set_boot_partition failed: %s\n", esp_err_to_name(err));
//         otaError = "Failed to set boot partition: " + String(esp_err_to_name(err));
//         otaInProgress = false;
//         return false;
//     }
    
//     Serial.println("OTA update successful, rebooting...");
//     otaInProgress = false;
//     otaProgress = 100;
    
//     // Store the update status in preferences
//     Preferences preferences;
//     if (preferences.begin("ota_info", false)) {
//         preferences.putBool("update_pending", true);
//         preferences.end();
//     }
    
//     // Reboot after a short delay
//     delay(1000);
//     esp_restart();
//     return true;
// }

// // Function to check if an update is pending after reboot
// bool isUpdatePending() {
//     Preferences preferences;
//     bool updatePending = false;
    
//     if (preferences.begin("ota_info", true)) {
//         updatePending = preferences.getBool("update_pending", false);
//         preferences.end();
//     }
    
//     return updatePending;
// }

// // Function to clear the update pending flag
// void clearUpdatePending() {
//     Preferences preferences;
    
//     if (preferences.begin("ota_info", false)) {
//         preferences.putBool("update_pending", false);
//         preferences.end();
//     }
// }

// // Function to get the current OTA update status
// OTAUpdateStatus getOTAStatus() {
//     OTAUpdateStatus status;
//     status.inProgress = otaInProgress;
//     status.progress = otaProgress;
//     status.error = otaError;
//     return status;
// }

// // Function to get the current running partition
// esp_ota_img_states_t getRunningPartition() {
//     const esp_partition_t *running = esp_ota_get_running_partition();
//     if (running == NULL) {
//         return ESP_OTA_IMG_UNDEFINED;
//     }
    
//     if (strcmp(running->label, "ota_0") == 0) {
//         return ESP_OTA_IMG_OTA_0;
//     } else if (strcmp(running->label, "ota_1") == 0) {
//         return ESP_OTA_IMG_OTA_1;
//     } else if (strcmp(running->label, "factory") == 0) {
//         return ESP_OTA_IMG_FACTORY;
//     }
    
//     return ESP_OTA_IMG_UNDEFINED;
// }

// // Function to initialize OTA functionality
// bool initOTA() {
//     // Initialize NVS
//     esp_err_t err = nvs_flash_init();
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         // NVS partition was truncated and needs to be erased
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(err);
    
//     // Check if we're running from OTA partition
//     const esp_partition_t *running = esp_ota_get_running_partition();
//     Serial.printf("Running partition type %d subtype %d (offset 0x%08x)\n",
//              running->type, running->subtype, running->address);
    
//     // If we're running from an OTA partition, mark the other OTA partition as the next update target
//     if (strcmp(running->label, "ota_0") == 0) {
//         const esp_partition_t *next_update = esp_ota_get_next_update_partition(NULL);
//         Serial.printf("Next update partition will be: %s\n", next_update->label);
//     } else if (strcmp(running->label, "ota_1") == 0) {
//         const esp_partition_t *next_update = esp_ota_get_next_update_partition(NULL);
//         Serial.printf("Next update partition will be: %s\n", next_update->label);
//     }
    
//     return true;
// } 