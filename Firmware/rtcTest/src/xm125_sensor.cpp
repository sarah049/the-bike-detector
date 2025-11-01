// #include <Arduino.h>
// #include <Wire.h>
// #include "xm125_sensor.h"

// // I2C pins for XM125 sensor
// #define XM125_SDA_PIN SENSOR_SDA_PIN  // Using the SENSOR_SDA_PIN defined in main.cpp
// #define XM125_SCL_PIN SENSOR_SCL_PIN  // Using the SENSOR_SCL_PIN defined in main.cpp

// // XM125 I2C address
// #define XM125_I2C_ADDRESS 0x55  // Default I2C address, adjust if needed

// // XM125 registers
// #define XM125_REG_STATUS 0x00
// #define XM125_REG_DATA_H 0x01
// #define XM125_REG_DATA_L 0x02
// #define XM125_REG_CONFIG 0x03
// #define XM125_REG_THRESHOLD 0x04

// // Sensor state
// static bool sensorInitialized = false;
// static uint8_t sensorStatus = 0;
// static uint16_t lastReading = 0;
// static uint8_t sensorConfig = 0;

// /**
//  * Initialize the XM125 sensor via I2C
//  * @return true if initialization successful, false otherwise
//  */
// bool initXM125Sensor() {
//     Serial.println("Initializing XM125 sensor...");
    
//     // Initialize I2C
//     Wire.begin(XM125_SDA_PIN, XM125_SCL_PIN);
//     Wire.setClock(100000);  // Set I2C clock to 100kHz
    
//     // Check if sensor is responsive
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("XM125 sensor not found at address 0x%02X. Error: %d\n", XM125_I2C_ADDRESS, error);
//         return false;
//     }
    
//     // Read initial status
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_STATUS);
//     error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to read XM125 status. Error: %d\n", error);
//         return false;
//     }
    
//     Wire.requestFrom(XM125_I2C_ADDRESS, 1);
//     if (Wire.available()) {
//         sensorStatus = Wire.read();
//         Serial.printf("XM125 initial status: 0x%02X\n", sensorStatus);
//     } else {
//         Serial.println("No data received from XM125 status register");
//         return false;
//     }
    
//     // Configure sensor
//     // Set configuration register (example: enable continuous reading mode)
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_CONFIG);
//     Wire.write(0x01);  // Enable continuous reading mode
//     error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to configure XM125. Error: %d\n", error);
//         return false;
//     }
    
//     // Read back configuration to verify
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_CONFIG);
//     error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to read XM125 configuration. Error: %d\n", error);
//         return false;
//     }
    
//     Wire.requestFrom(XM125_I2C_ADDRESS, 1);
//     if (Wire.available()) {
//         sensorConfig = Wire.read();
//         Serial.printf("XM125 configuration: 0x%02X\n", sensorConfig);
//     } else {
//         Serial.println("No data received from XM125 configuration register");
//         return false;
//     }
    
//     // Set threshold (example: set to 100)
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_THRESHOLD);
//     Wire.write(100);  // Threshold value
//     error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to set XM125 threshold. Error: %d\n", error);
//         return false;
//     }
    
//     sensorInitialized = true;
//     Serial.println("XM125 sensor initialized successfully");
//     return true;
// }

// /**
//  * Read data from the XM125 sensor
//  * @return The sensor reading value (0-65535)
//  */
// uint16_t readXM125Data() {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return 0;
//     }
    
//     // Read data registers (high byte and low byte)
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_DATA_H);
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to read XM125 data high byte. Error: %d\n", error);
//         return lastReading;  // Return last valid reading
//     }
    
//     Wire.requestFrom(XM125_I2C_ADDRESS, 2);  // Request 2 bytes (high and low)
    
//     if (Wire.available() >= 2) {
//         uint8_t dataHigh = Wire.read();
//         uint8_t dataLow = Wire.read();
        
//         // Combine high and low bytes into 16-bit value
//         lastReading = (dataHigh << 8) | dataLow;
        
//         Serial.printf("XM125 reading: %d (0x%02X 0x%02X)\n", lastReading, dataHigh, dataLow);
//         return lastReading;
//     } else {
//         Serial.println("Incomplete data received from XM125");
//         return lastReading;  // Return last valid reading
//     }
// }

// /**
//  * Check if the sensor has detected a bike (based on threshold)
//  * @return true if a bike is detected, false otherwise
//  */
// bool isBikeDetected() {
//     uint16_t reading = readXM125Data();
    
//     // Check if reading exceeds threshold
//     // This is a simplified example - adjust based on actual sensor behavior
//     return (reading > 100);  // Threshold value
// }

// /**
//  * Get the current sensor status
//  * @return The sensor status byte
//  */
// uint8_t getXM125Status() {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return 0;
//     }
    
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_STATUS);
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to read XM125 status. Error: %d\n", error);
//         return sensorStatus;  // Return last known status
//     }
    
//     Wire.requestFrom(XM125_I2C_ADDRESS, 1);
//     if (Wire.available()) {
//         sensorStatus = Wire.read();
//         Serial.printf("XM125 status: 0x%02X\n", sensorStatus);
//     } else {
//         Serial.println("No data received from XM125 status register");
//     }
    
//     return sensorStatus;
// }

// /**
//  * Set the sensor threshold
//  * @param threshold The threshold value (0-255)
//  * @return true if successful, false otherwise
//  */
// bool setXM125Threshold(uint8_t threshold) {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return false;
//     }
    
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_THRESHOLD);
//     Wire.write(threshold);
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to set XM125 threshold. Error: %d\n", error);
//         return false;
//     }
    
//     Serial.printf("XM125 threshold set to: %d\n", threshold);
//     return true;
// }

// /**
//  * Put the sensor in low power mode
//  * @return true if successful, false otherwise
//  */
// bool setXM125LowPowerMode() {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return false;
//     }
    
//     // Set configuration register to enable low power mode
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_CONFIG);
//     Wire.write(sensorConfig | 0x02);  // Set bit 1 for low power mode
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to set XM125 low power mode. Error: %d\n", error);
//         return false;
//     }
    
//     Serial.println("XM125 set to low power mode");
//     return true;
// }

// /**
//  * Wake up the sensor from low power mode
//  * @return true if successful, false otherwise
//  */
// bool wakeXM125Sensor() {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return false;
//     }
    
//     // Set configuration register to disable low power mode
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_CONFIG);
//     Wire.write(sensorConfig & ~0x02);  // Clear bit 1 for normal mode
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to wake XM125 sensor. Error: %d\n", error);
//         return false;
//     }
    
//     Serial.println("XM125 sensor woken from low power mode");
//     return true;
// }

// /**
//  * Calibrate the sensor
//  * @return true if calibration successful, false otherwise
//  */
// bool calibrateXM125Sensor() {
//     if (!sensorInitialized) {
//         Serial.println("XM125 sensor not initialized");
//         return false;
//     }
    
//     Serial.println("Calibrating XM125 sensor...");
    
//     // Send calibration command (example: write 0x80 to config register)
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_CONFIG);
//     Wire.write(sensorConfig | 0x80);  // Set bit 7 for calibration
//     uint8_t error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to send calibration command. Error: %d\n", error);
//         return false;
//     }
    
//     // Wait for calibration to complete (example: 100ms)
//     delay(100);
    
//     // Read status to check if calibration was successful
//     Wire.beginTransmission(XM125_I2C_ADDRESS);
//     Wire.write(XM125_REG_STATUS);
//     error = Wire.endTransmission();
    
//     if (error != 0) {
//         Serial.printf("Failed to read status after calibration. Error: %d\n", error);
//         return false;
//     }
    
//     Wire.requestFrom(XM125_I2C_ADDRESS, 1);
//     if (Wire.available()) {
//         sensorStatus = Wire.read();
//         Serial.printf("XM125 status after calibration: 0x%02X\n", sensorStatus);
        
//         // Check if calibration was successful (example: bit 0 set)
//         if (sensorStatus & 0x01) {
//             Serial.println("XM125 calibration successful");
//             return true;
//         } else {
//             Serial.println("XM125 calibration failed");
//             return false;
//         }
//     } else {
//         Serial.println("No data received from XM125 status register after calibration");
//         return false;
//     }
// } 