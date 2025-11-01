#include <ArduinoHttpClient.h>
#include <vector>
#include <utility>
#include <Arduino.h>
#include <Wire.h>
#include <esp_wifi.h>

using namespace std;

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

// Uncomment for serial messages from the cell library
#define DEBUG_CELL

#define SerialMon Serial


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "soracom.io";  // Your provider's APN
const char user[] = "sora";        // Username if required
const char pass[] = "sora";        // Password if required


bool cellLibInit(int rx, int tx, String simcard_pin="");

// New function to send AT commands to the cellular modem
String sendATCommandToCellular(const char* command, unsigned long timeout = 1000);


class HttpConnection {
private:
    HttpClient* http;
    
    bool postRequest(String resource, String contentType, String httpRequestData);

    int readStatus();

    vector<pair<String,String>> readHeader();

    String readBody();

public:
    HttpConnection(const char* server, int port, int max_seconds_waiting=10);

    ~HttpConnection();

    // request no return
    bool getRequest(String resource);

    int readContentLength();

    // returns the body of the response or empty string if not succesful
    String sendGet(String resource, bool use_stream=false);

    // returns the body of the response or empty string if not succesful
    int postJson(String jsonString, const char* resource);

    HttpClient* httpclient();

};
