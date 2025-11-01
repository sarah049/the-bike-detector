#include <Arduino.h>
#include "comms.h"
#include "cellLib.h"
#include "faultChecks.h"

//Comms - 3 PINs (can assign 3 hardware UART pins)
#define RX_PIN 6
#define TX_PIN 7

const char* server = "efpi-10.mit.edu";  // Server URL
const int port = 80;                     // Default HTTP port
const char* endpoint = "/tunnel_anacamba/debug_data"; // Endpoint

int comms_result = 0;
String lastServerResponse = ""; // Store the last server response

void sendATCommand(const char* command) {
    Serial.print("Sending command: ");
    Serial.println(command);
    
    char buffer[128];
    
    String response = sendATCommandToCellular(command);
    Serial.println(response);
}


void prep_and_post_comms(String queryString){
    Serial.println("Initializing cell module...");

    if (cellLibInit(TX_PIN, RX_PIN)) {
        Serial.println("Cell module initialized successfully");
    } else {
        Serial.println("Failed to initialize cell module");
        return;
    }

    Serial.println("Sending diagnostic AT commands...");

    Serial.println("Checking signal quality (AT+CSQ):");
    sendATCommand("AT+CSQ");

    Serial.println("Checking network registration status (AT+CEREG?):");
    sendATCommand("AT+CEREG?");

    Serial.println("Checking current operator (AT+COPS?):");
    sendATCommand("AT+COPS?");

   Serial.println("Sending POST request to " + String(server) + String(endpoint));
    Serial.println("Data: " + queryString);

    // Create HTTP connection and send the POST request
    HttpConnection connection(server, port);
    int result = connection.postJson(queryString, endpoint);

    comms_result = result;
    // Print the result
    if (result >= 0) {
        Serial.println("POST successful! Response code: " + String(result));
    } else {
        Serial.println("POST failed! Error code: " + String(result));
    }
}

// Function to get the last server response
String getServerResponse() {
    return lastServerResponse;
}