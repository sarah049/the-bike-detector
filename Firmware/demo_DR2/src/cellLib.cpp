#define TINY_GSM_MODEM_SIM7080

#include <TinyGsmClient.h>

#include "cellLib.h"

#define SerialAT SerialPort
HardwareSerial SerialAT(1);

#ifdef DUMP_AT_COMMANDS
    #include <StreamDebugger.h>
    StreamDebugger debugger(SerialAT, SerialMon);
    TinyGsm modem(debugger);
#else
    TinyGsm modem(SerialAT);
#endif

TinyGsmClient cell_client(modem);


bool cellLibInit(int tx, int rx, String simcard_pin) {
    // Set GSM module baud rate
    SerialAT.begin(115200, SERIAL_8N1, rx, tx);
    delay(3000);
    if(!SerialAT) return 0;

    //modem.init();
    modem.restart();

    if(simcard_pin != "") {
        modem.simUnlock(simcard_pin.c_str());
        #ifdef DEBUG_CELL
        SerialMon.println(simcard_pin);
        #endif
    }

    #ifdef DEBUG_CELL
    String modemInfo = modem.getModemInfo();
    SerialMon.print(F("Modem: "));
    SerialMon.println(modemInfo);
    SerialMon.println(modem.getSignalQuality());
    #endif

    return 1;
}


bool HttpConnection::postRequest(String resource, String contentType, String httpRequestData) {
    #ifdef DEBUG_CELL
    SerialMon.print(F("Performing HTTP POST request... "));
    #endif

    int err = http->post(resource, contentType, httpRequestData);

    #ifdef DEBUG_CELL
    SerialMon.println(resource);
    SerialMon.println(contentType);
    SerialMon.println(httpRequestData);
    #endif

    if (err != 0) {
        #ifdef DEBUG_CELL
        SerialMon.println(readStatus());
        SerialMon.println(F("failed to connect"));
        #endif

        return 0;
    }

    //int status = readStatus();
    //#ifdef DEBUG_CELL
    //SerialMon.println(status);
    //#endif
    //if (!status) {
    //    return 0;
    //}
    
    //if (http->isResponseChunked()) {
    //    SerialMon.println(F("The response is chunked"));
    //}

    return 1;
}


bool HttpConnection::getRequest(String resource) {
    #ifdef DEBUG_CELL
    SerialMon.print(F("Performing HTTP GET request... "));
    #endif

    int err = http->get(resource);
    if (err != 0) {
        #ifdef DEBUG_CELL
        SerialMon.println(F("failed to connect"));
        SerialMon.println(err);
        #endif

        return 0;
    }

    int status = readStatus();
    #ifdef DEBUG_CELL
    SerialMon.println(status);
    #endif
    if (!status) {
        return 0;
    }

    return 1;
}


int HttpConnection::readStatus() {
    return http->responseStatusCode();
}


vector<pair<String,String>> HttpConnection::readHeader() {
    vector<pair<String,String>> ans;

    while (http->headerAvailable()) {
        String headerName = http->readHeaderName();
        String headerValue = http->readHeaderValue();
        ans.emplace_back(headerName, headerValue);

        #ifdef DEBUG_CELL
        SerialMon.println(headerName + " : " + headerValue);
        #endif
    }
    return ans;
}


int HttpConnection::readContentLength() {
    int length = http->contentLength();

    #ifdef DEBUG_CELL
    if (length >= 0) {
        SerialMon.print(F("Content length is: "));
        SerialMon.println(length);
    }
    #endif

    return length;
}


String HttpConnection::readBody() {
    String body = http->responseBody();

    #ifdef DEBUG_CELL
    SerialMon.println(F("Response:"));
    SerialMon.println(body);
    SerialMon.print(F("Body length is: "));
    SerialMon.println(body.length());
    #endif

    return body;
}


HttpConnection::HttpConnection(const char* server, int port, int max_seconds_waiting) {
    #ifdef DEBUG_CELL
    SerialMon.println(server);
    SerialMon.println(port);
    #endif


    http = new HttpClient(cell_client, server, port); 

    int i = max_seconds_waiting;
    while(!modem.waitForNetwork()){
        #ifdef DEBUG_CELL
        SerialMon.print(F("Waiting for network..."));
        SerialMon.println(modem.getSignalQuality());
        #endif

        if(i-- == 0) break;

        delay(10000);
    } 

    #ifdef DEBUG_CELL
    SerialMon.println(" OK");
    SerialMon.println(modem.getSignalQuality());
    #endif

    #ifdef DEBUG_CELL
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    #endif

    i = max_seconds_waiting;
    while(!modem.gprsConnect(apn, user, pass)) {
        #ifdef DEBUG_CELL
        SerialMon.println(" fail");
        #endif

        if(i-- == 0) break;

        delay(10000);
    }
    #ifdef DEBUG_CELL
    SerialMon.println(" OK");
    #endif
}


HttpConnection::~HttpConnection() {
    http->stop();

    #ifdef DEBUG_CELL
    SerialMon.println(F("Server disconnected"));
    #endif

    delete http;
}


String HttpConnection::sendGet(String resource, bool use_stream) {
    const int tries_per_request = 3;

    bool res = false;
    for(int l=0;l<tries_per_request;l++) {
        delay(10);
        res = getRequest(resource);
        if(res) break;
    }
    if(!res)
        return "";

    if(!use_stream)
        return readBody();

    String ret;
    int length = readContentLength();
    for (int i = 0; i < length; i++) {
        if(http->available())
            ret += (char)(http->read());
    }

    #ifdef DEBUG_CELL
    SerialMon.println(ret);
    #endif

    return ret;
}

int HttpConnection::postJson(String jsonString, const char* resource) {
    const int tries_per_request = 3;

    bool res = false;
    for(int l=0;l<tries_per_request;l++) {
        delay(10);
        res = postRequest(resource, "application/json", jsonString);
        if(res) break;
    }

    int status = readStatus();
    if(!res || status!=200) {
        #ifdef DEBUG_CELL
        SerialMon.printf("POST request failed with status %d\n", status);
        #endif
        return -1;
    }

    String response = readBody();

    return (int) atoi(response.c_str());
}


HttpClient* HttpConnection::httpclient() {
    return http;
}

String sendATCommandToCellular(const char* command, unsigned long timeout) {
    String response = "";
    
    // Send the command to the modem
    SerialAT.println(command);
    
    // Wait for the response with a timeout
    unsigned long start = millis();
    while (millis() - start < timeout) {
        if (SerialAT.available()) {
            char c = SerialAT.read();
            response += c;
        }
    }
    
    return response;
}