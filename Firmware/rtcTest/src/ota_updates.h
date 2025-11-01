// #ifndef OTA_UPDATES_H
// #define OTA_UPDATES_H

// #include <Arduino.h>
// #include <esp_ota_ops.h>

// // Enum for OTA image states
// enum esp_ota_img_states_t {
//     ESP_OTA_IMG_UNDEFINED,
//     ESP_OTA_IMG_FACTORY,
//     ESP_OTA_IMG_OTA_0,
//     ESP_OTA_IMG_OTA_1
// };

// // Structure to hold OTA update status
// struct OTAUpdateStatus {
//     bool inProgress;
//     int progress;
//     String error;
// };

// // Function declarations
// bool checkForUpdates(const char* currentVersion);
// bool performOTAUpdate();
// bool isUpdatePending();
// void clearUpdatePending();
// OTAUpdateStatus getOTAStatus();
// esp_ota_img_states_t getRunningPartition();
// bool initOTA();

// #endif // OTA_UPDATES_H 