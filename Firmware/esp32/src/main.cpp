#include <WiFi.h>
#include <esp_wifi.h>

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

char network[] = "608_24G";
// char password[] = "608g2020";
// char network[] = "MIT";
char password[] = "{#gz32W%Rs";

uint8_t channel = 6; //network channel on 2.4 GHz
// byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
byte bssid[] = {0x90, 0x4C, 0x81, 0x76, 0x7A, 0xE0}; //6 byte MAC address of AP you're targeting. 90:4C:81:76:7A:E0

void setup(){
  delay(3000);
  Serial.begin(9600);  
  Serial.println("Starting setup...");

  // Set WiFi mode to Station mode
  WiFi.mode(WIFI_STA);
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(network);
  
  // Try connecting to WiFi
  WiFi.begin(network, password, channel, bssid);

  // Wait until connected or timeout (10 seconds)
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= 10000) { // 10 second timeout
      Serial.println("Failed to connect to WiFi within timeout.");
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to WiFi! IP Address: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
}

void loop(){
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
  delay(2000);
}