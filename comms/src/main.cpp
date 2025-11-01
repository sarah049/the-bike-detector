#include <Arduino.h>
#include "cellLib.h"

#define RX_PIN 6
#define TX_PIN 7

// Define server information for HTTP requests
const char* server = "efpi-10.mit.edu";  // Server URL
const int port = 80;                     // Default HTTP port
const char* endpoint = "/tunnel_akshay_a/debug_data"; // Endpoint

// Variables for timing POST requests
unsigned long previousMillis = 0;
const unsigned long postInterval = 10000; // 10 seconds interval between posts
int counter = 0; // Counter to include in the dummy data

// Function to send AT command and print response
void sendATCommand(const char* command) {
    // We'll use TinyGSM's functions since we can't access SerialAT directly
    Serial.print("Sending command: ");
    Serial.println(command);
    
    // Create a small buffer for response
    char buffer[128];
    
    // Use modem's sendAT and waitResponse functions
    // This will require modifying cellLib.h to expose these functions
    
    // For now, we'll use a simpler approach with a function we'll add to cellLib
    String response = sendATCommandToCellular(command);
    Serial.println(response);
}

void setup() {
    Serial.begin(115200);
    
    // Initialize the cellular module using cellLib
    delay(3000);
    Serial.println("Initializing cell module...");

    if (cellLibInit(TX_PIN, RX_PIN)) {
        Serial.println("Cell module initialized successfully");
    } else {
        Serial.println("Failed to initialize cell module");
        return;
    }
    
    // In setup() after cellLibInit()
    Serial.println("Sending diagnostic AT commands...");
    
    // Check signal quality
    Serial.println("Checking signal quality (AT+CSQ):");
    sendATCommand("AT+CSQ");
    
    // Check network registration status
    Serial.println("Checking network registration status (AT+CEREG?):");
    sendATCommand("AT+CEREG?");
    
    // Check operator
    Serial.println("Checking current operator (AT+COPS?):");
    sendATCommand("AT+COPS?");
    
    Serial.println("Setup complete. Will start sending POST requests every 10 seconds.");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check if 10 seconds have passed since the last POST request
    if (currentMillis - previousMillis >= postInterval) {
        previousMillis = currentMillis; // Save the current time
        
        // Create dummy JSON data to send
        counter++;
        String jsonData = "{\"device_id\":\"bike_detector_01\",\"counter\":" + String(counter) + ",\"timestamp\":" + String(currentMillis/1000) + ",\"status\":\"operational\"}";
        
        Serial.println("Sending POST request to " + String(server) + String(endpoint));
        Serial.println("Data: " + jsonData);
        
        // Create HTTP connection and send the POST request
        HttpConnection connection(server, port);
        int result = connection.postJson(jsonData, endpoint);
        
        // Print the result
        if (result >= 0) {
            Serial.println("POST successful! Response code: " + String(result));
        } else {
            Serial.println("POST failed! Error code: " + String(result));
        }
        
        Serial.println("Next POST in 10 seconds...");
    }
    
    // Small delay to prevent flooding the serial monitor
    delay(100);
}

// #include <Arduino.h>
// #include <HardwareSerial.h>

// HardwareSerial sim7080g(1);
// #define PWRKEY 4
// #define RX_PIN 6
// #define TX_PIN 7

// // Define server information for HTTP requests
// const char* server = "efpi-10.mit.edu";  // Server URL
// const char* endpoint = "/tunnel_akshay_a/debug_data"; // Endpoint

// // Variables for timing POST requests
// unsigned long previousMillis = 0;
// const unsigned long postInterval = 10000; // 10 seconds interval between posts
// int counter = 0; // Counter to include in the dummy data

// // Function to send AT command and print response
// void sendATCommand(const char* command, int timeout = 1000) {
//     Serial.print("Sending command: ");
//     Serial.println(command);
    
//     sim7080g.println(command);
    
//     unsigned long startTime = millis();
//     String response = "";
    
//     // Wait for response or timeout
//     while (millis() - startTime < timeout) {
//         if (sim7080g.available()) {
//             char c = sim7080g.read();
//             response += c;
//             Serial.write(c);  // Echo to serial monitor
//         }
//     }
    
//     if (response.length() == 0) {
//         Serial.println("No response received");
//     }
// }

// void power_on_sim7080g() {
//     pinMode(PWRKEY, OUTPUT);
//     digitalWrite(PWRKEY, LOW);
//     delay(1000); // Hold PWRKEY low for 1 second
//     digitalWrite(PWRKEY, HIGH);
//     delay(5000); // Wait for initialization
// }

// // Function to perform HTTP POST request using AT commands
// void performHttpPost(String jsonData) {
//     Serial.println("Starting HTTP POST procedure...");
    
//     // Step 1: Activate the network bearer
//     sendATCommand("AT+CNACT=0,1", 5000);
    
//     // Step 2: Check bearer status
//     sendATCommand("AT+CNACT?", 2000);
    
//     // Step 3: Set URL configuration
//     String urlConfig = "AT+SHCONF=\"URL\",\"http://" + String(server) + "\"";
//     sendATCommand(urlConfig.c_str(), 2000);
    
//     // Step 4: Set body length
//     sendATCommand("AT+SHCONF=\"BODYLEN\",1024", 2000);
    
//     // Step 5: Set header length
//     sendATCommand("AT+SHCONF=\"HEADERLEN\",350", 2000);
    
//     // Step 6: Start HTTP service
//     sendATCommand("AT+SHCONN", 5000);
    
//     // Step 7: Check HTTP service status
//     sendATCommand("AT+SHSTATE?", 2000);
    
//     // Step 8: Clear HTTP header
//     sendATCommand("AT+SHCHEAD", 2000);
    
//     // Step 9: Add HTTP headers
//     sendATCommand("AT+SHAHEAD=\"Content-Type\",\"application/json\"", 1000);
//     sendATCommand("AT+SHAHEAD=\"User-Agent\",\"ESP32 SIM7080G Client\"", 1000);
//     sendATCommand("AT+SHAHEAD=\"Connection\",\"keep-alive\"", 1000);
    
//     // Step 10: Set up the POST body
//     int bodyLength = jsonData.length();
//     String bodyCmdStr = "AT+SHBOD=" + String(bodyLength) + ",10000";
//     sendATCommand(bodyCmdStr.c_str(), 2000);
    
//     // Step 11: Send the JSON data
//     sendATCommand(jsonData.c_str(), 2000);
    
//     // Step 12: Make the HTTP POST request to the endpoint
//     String requestCmdStr = "AT+SHREQ=\"" + String(endpoint) + "\",3";
//     sendATCommand(requestCmdStr.c_str(), 10000); // Longer timeout for request
    
//     // Step 13: Disconnect HTTP service
//     sendATCommand("AT+SHDISC", 2000);
// }

// void setup() {
//     Serial.begin(115200);
//     sim7080g.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // RX, TX
    
//     Serial.println("Starting SIM7080G setup...");
//     power_on_sim7080g();

//     // Test AT command
//     sendATCommand("AT", 1000);

//     // Set PDP context for GPRS
//     sendATCommand("AT+CGDCONT=1,\"IP\",\"soracom.io\"", 2000);
//     sendATCommand("AT+CGDCONT?", 2000);
    
//     // Set authentication for PDP context
//     sendATCommand("AT+CGAUTH=1,1,\"sora\",\"sora\"", 2000);
    
//     // Check band configuration
//     sendATCommand("AT+CBANDCFG?", 2000);
    
//     // Set operator selection to automatic
//     sendATCommand("AT+COPS=0", 2000);
    
//     // Wait for network registration
//     delay(5000);
    
//     // Check network registration status
//     sendATCommand("AT+CEREG?", 2000);
    
//     Serial.println("Setup complete. Will start sending POST requests every 10 seconds.");
// }

// void loop() {
//     unsigned long currentMillis = millis();
    
//     // Check if 10 seconds have passed since the last POST request
//     if (currentMillis - previousMillis >= postInterval) {
//         previousMillis = currentMillis; // Save the current time
        
//         // Check network status before attempting the POST
//         Serial.println("Checking network status before POST attempt...");
//         sendATCommand("AT+CSQ", 1000);       // Signal quality
//         sendATCommand("AT+CEREG?", 1000);    // Network registration
//         sendATCommand("AT+CGATT?", 1000);    // GPRS attachment
        
//         // Create dummy JSON data to send
//         counter++;
//         String jsonData = "{\"device_id\":\"bike_detector_01\",\"counter\":" + String(counter) + 
//                          ",\"timestamp\":" + String(currentMillis/1000) + 
//                          ",\"status\":\"operational\"}";
        
//         Serial.println("Data to send: " + jsonData);
        
//         // Perform HTTP POST request
//         performHttpPost(jsonData);
        
//         Serial.println("Next POST in 10 seconds...");
//     }
    
//     // Small delay to prevent flooding the serial monitor
//     delay(100);
// } 
