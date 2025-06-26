#pragma once

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DFRobot_AS3935_I2C.h>
#include <functional>
#include <time.h>

// Pin definitions (can be overridden in main)
#ifndef I2C_SDA
#define I2C_SDA 21
#endif
#ifndef I2C_SCL
#define I2C_SCL 22
#endif
#ifndef IRQ_PIN
#define IRQ_PIN 16
#endif

// I2C addresses
#define I2C_ADDRESS_AS3935 0x03
#define I2C_ADDRESS_BME280 0x76

// AS3935 configuration
#define AS3935_CAPACITANCE 64 // Stepping in multiples of 8 pF (8, 16, 24, ..., 120)
#define AS3935_INDOORS 0
#define AS3935_OUTDOORS 1
#define AS3935_MODE AS3935_OUTDOORS
#define AS3935_DISTURBER_DETECTION true
#define AS3935_NOISE_LEVEL 1 // Lowest noise level for highest sensitivity
#define AS3935_WATCHDOG_THRESHOLD 0 // Lowest threshold for highest sensitivity
#define AS3935_SPIKE_REJECTION 0 // Lowest rejection for highest sensitivity

// BME280 configuration
#define BME280_TEMPERATURE_OFFSET 0.0f // Temperature offset in degrees Celsius

extern volatile int8_t AS3935IsrTrig;
extern volatile time_t AS3935IsrTrigTime;

extern DFRobot_AS3935_I2C lightningSensor;
extern Adafruit_BME280 bme;

void initI2C();
void scanI2CBus(std::function<void(const char*)> logFunc);
bool initBME280();
bool initAS3935();
void readBME280Data(float &temperature, float &humidity, float &pressure);
void logInterrupt(int interruptSource, void (*publishFunc)(const char*));
void AS3935_ISR();