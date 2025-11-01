#include <Arduino.h>
#include "comms.h"
#include "cellLib.h"
#include "faultChecks.h"
// #include <esp32fota.h>
// #include <ArduinoJson.h>

//Comms - 3 PINs (can assign 3 hardware UART pins)
#define RX_PIN 20
#define TX_PIN 21
#define COMMS_LED_PIN 6

const char* server = "efpi-24.mit.edu";  // Server URL
const int port = 80;                     // Default HTTP port
char* data_endpoint = "/post_data"; // Endpoint for data
char* handshake_endpoint = "/check_post"; // Endpoint for handshake

// OTA configuration
const char* firmware_endpoint = "/firmware_cellular.json";
const char* FIRMWARE_VERSION = "1.0.0";
// bool otaInProgress = false;

int comms_result = 0;
String lastServerResponse = ""; // Store the last server response

// esp32FOTA FOTA;
// bool otaInProgress = false;

// // Function to check and perform OTA updates
// bool checkAndPerformOTA(HttpConnection& connection) {
//     Serial.println("Checking for firmware updates...");
    
//     // Set the current firmware version for comparison
//     FOTA.setCurrentVersion(FIRMWARE_VERSION);
    
//     // Get the firmware JSON file from the server
//     String firmwareJson = connection.sendGet(firmware_endpoint, false);
    
//     if (firmwareJson.length() == 0) {
//         Serial.println("Failed to get firmware info");
//         return false;
//     }
    
//     Serial.println("Firmware info received: " + firmwareJson);
    
//     // Parse the JSON response
//     DynamicJsonDocument doc(1024);
//     DeserializationError error = deserializeJson(doc, firmwareJson);
    
//     if (error) {
//         Serial.print("JSON parsing failed: ");
//         Serial.println(error.c_str());
//         return false;
//     }
    
//     // Extract firmware information
//     const char* newVersion = doc["version"];
//     const char* firmwareUrl = doc["url"];
    
//     if (!newVersion || !firmwareUrl) {
//         Serial.println("Invalid firmware JSON format");
//         return false;
//     }
    
//     Serial.print("Current firmware version: ");
//     Serial.println(FIRMWARE_VERSION);
//     Serial.print("Available firmware version: ");
//     Serial.println(newVersion);
    
//     // Compare versions
//     if (strcmp(FIRMWARE_VERSION, newVersion) != 0) {
//         Serial.println("New firmware available! Starting update...");
        
//         // Configure the update
//         FOTA.useDeviceHardware = true; // Use the same cellular connection
        
//         // Create a firmware update config
//         esp32FOTA_configuration config;
//         config.name = "esp32-cellular-device";
//         config.semver = FIRMWARE_VERSION;
//         config.url = firmwareUrl;
//         config.check_sig = false; // Set to true if using signature verification
//         config.use_device_hardware = true;
        
//         // Initialize FOTA with config
//         FOTA.setConfig(config);
        
//         // Start the update process
//         otaInProgress = true;
//         bool updateStarted = FOTA.update();
        
//         if (updateStarted) {
//             Serial.println("Update started successfully");
            
//             // Wait for update to complete
//             while (otaInProgress) {
//                 delay(1000);
//                 Serial.print(".");
//                 // The ESP will restart automatically when the update is complete
//             }
//             return true;
//         } else {
//             Serial.println("Failed to start update");
//             otaInProgress = false;
//         }
//     } else {
//         Serial.println("Firmware is up to date");
//     }
    
//     return false;
// }

String sendATCommand(const char* command) {
    Serial.print("Sending command: ");
    Serial.println(command);
    
    char buffer[128];
    
    String response = sendATCommandToCellular(command);
    Serial.println(response);
    return response;
}

int prep_and_post_comms(String queryString, bool needsInit = true) {
    char* endpoint = data_endpoint;
    
    if (needsInit) {
        endpoint = handshake_endpoint;
        Serial.println("Initializing cell module...");

        if (cellLibInit(TX_PIN, RX_PIN)) {
            Serial.println("Cell module initialized successfully");
        } else {
            Serial.println("Failed to initialize cell module");
            return 404;
        }

        Serial.println("Sending diagnostic AT commands...");

        Serial.println("Checking network mode (AT+CNMP=38):");
        sendATCommand("AT+CNMP=38");

        Serial.println("Checking sim status (AT+CPIN?):");
        sendATCommand("AT+CPIN?");

        Serial.println("Checking network mode (AT+CMNB=1):");
        sendATCommand("AT+CMNB=1");

        Serial.println("Checking operator (AT+COPS?):");
        sendATCommand("AT+COPS?");

        Serial.println("Checking signal quality (AT+CSQ):");
        sendATCommand("AT+CSQ");
        
        Serial.println("Checking band config status (AT+CBANDCFG=?):");
        sendATCommand("AT+CBANDCFG?");

        Serial.println("Checking antenna (AT+CFUN?):");
        sendATCommand("AT+CFUN?");

        // Set up GPRS connection
        Serial.println("Setting up GPRS connection...");
        const char* apn = "soracom.io";
        const char* username = "sora";
        const char* password = "sora";
        
        // Define PDP context
        sendATCommand("AT+CGDCONT=1,\"IP\",\"soracom.io\"");

        sendATCommand("AT+CGATT?");

        delay(1000);

        
        
        // Attach to GPRS
        // String gprsResponse = sendATCommand("AT+CGATT=1");
        // if (gprsResponse.indexOf("OK") == -1) {
        //     Serial.println("Failed to attach to GPRS");
        //     lastServerResponse = "GPRS attach failed";
        //     return 404;
        // }
        
        // Check APN
        // sendAT(GF("+CGNAPN"));
    
        
        // Configure bearer settings with authentication
        sendATCommand("AT+CNCFG=0,1,\"soracom.io\",\"sora\",\"sora\",3");
        sendATCommand("AT+CNACT=0,1");
        sendATCommand("AT+CNACT=0,1");
        sendATCommand("AT+CNACT=0,1");
        sendATCommand("AT+CNACT=0,1");
        sendATCommand("AT+CNACT=0,1");
        // sendATCommand("AT+CGACT=1");

        sendATCommand("AT+CGNAPN");
        
        // Activate network connection
        // bool gprsConnected = false;
        // int attempts = 0;
        // while (!gprsConnected && attempts < 5) {
        //     String actResponse = sendATCommand("AT+CNACT=0,1");
        //     if (actResponse.indexOf("+APP PDP: 0,ACTIVE") != -1) {
        //         gprsConnected = true;
        //         Serial.println("GPRS connection established successfully");
        //     } else {
        //         Serial.println("GPRS activation attempt " + String(attempts + 1) + " failed");
        //         attempts++;
        //         delay(1000);
        //     }
        // }
        
        // if (!gprsConnected) {
        //     Serial.println("Failed to establish GPRS connection after 5 attempts");
        //     lastServerResponse = "GPRS connection failed";
        //     return 404;
        // }

        Serial.println("Checking network registration status (AT+CEREG?):");
        String response = sendATCommand("AT+CEREG?");
        
        response.replace(" ", "");

        const String search = "0,5";
        int index = response.indexOf(search);
        if (index != -1) {
            Serial.println("Network registration successful");
        } else {
            Serial.println("Network registration failed");
            lastServerResponse = "comms failed";
            return 404;
        }

        Serial.println("Checking current operator (AT+COPS?):");
        sendATCommand("AT+COPS?");
    }

    Serial.println("Sending POST request to " + String(server) + String(endpoint));
    Serial.println("Data: " + queryString);

    // Create HTTP connection and send the POST request
    HttpConnection connection(server, port);
    int result = connection.postJson(queryString, endpoint);
    
    comms_result = result;
    // Print the result
    if (result >= 0) {
        Serial.println("POST successful! Response code: " + String(result));

        // Check for firmware updates after successful POST
        // Serial.println("Checking for firmware updates...");
        // if (!otaInProgress) {
        //     checkAndPerformOTA(connection);
        // }
        //TODO

    } else {
        Serial.println("POST failed! Error code: " + String(result));
    }
    lastServerResponse = "";
    return result;
}

// Function to get the last server response
String getServerResponse() {

    return lastServerResponse;
}

// Callback function for FOTA
// void otaProgress(int progress) {
//     Serial.printf("OTA Progress: %d%%\n", progress);
//     if (progress == 100) {
//         Serial.println("OTA update completed, device will restart...");
//         otaInProgress = false;
//     }
// }

// void setupOTA() {
//     // Register callback for OTA progress
//     FOTA.setProgressCallback(otaProgress);
    
//     // Initialize OTA configuration
//     esp32FOTA_configuration config;
//     config.name = "esp32-cellular-device";
//     config.semver = FIRMWARE_VERSION;
//     config.check_sig = false; // Set to true if using signature verification
//     config.use_device_hardware = true;
    
//     // Set the configuration
//     FOTA.setConfig(config);
    
//     Serial.println("OTA system initialized");
// }