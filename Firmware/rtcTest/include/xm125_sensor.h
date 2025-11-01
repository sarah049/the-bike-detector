#ifndef XM125_SENSOR_H
#define XM125_SENSOR_H

#include <Arduino.h>

/**
 * Initialize the XM125 sensor via I2C
 * @return true if initialization successful, false otherwise
 */
bool initXM125Sensor();

/**
 * Read data from the XM125 sensor
 * @return The sensor reading value (0-65535)
 */
uint16_t readXM125Data();

/**
 * Check if the sensor has detected a bike (based on threshold)
 * @return true if a bike is detected, false otherwise
 */
bool isBikeDetected();

/**
 * Get the current sensor status
 * @return The sensor status byte
 */
uint8_t getXM125Status();

/**
 * Set the sensor threshold
 * @param threshold The threshold value (0-255)
 * @return true if successful, false otherwise
 */
bool setXM125Threshold(uint8_t threshold);

/**
 * Put the sensor in low power mode
 * @return true if successful, false otherwise
 */
bool setXM125LowPowerMode();

/**
 * Wake up the sensor from low power mode
 * @return true if successful, false otherwise
 */
bool wakeXM125Sensor();

/**
 * Calibrate the sensor
 * @return true if calibration successful, false otherwise
 */
bool calibrateXM125Sensor();

#endif // XM125_SENSOR_H 