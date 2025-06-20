#pragma once

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <DFRobot_AS3935_I2C.h>

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
#define I2C_ADDRESS_BME280 0x77

// AS3935 configuration
#define AS3935_CAPACITANCE 8
#define AS3935_INDOORS 0
#define AS3935_OUTDOORS 1
#define AS3935_MODE AS3935_OUTDOORS
#define AS3935_DISTURBER_DETECTION true
#define AS3935_NOISE_LEVEL 2
#define AS3935_WATCHDOG_THRESHOLD 1
#define AS3935_SPIKE_REJECTION 1

// BME280 configuration
#define BME280_TEMPERATURE_OFFSET 0.0f // Temperature offset in degrees Celsius

extern volatile int8_t AS3935IsrTrig;

extern DFRobot_AS3935_I2C lightningSensor;
extern Adafruit_BME280 bme;

void initI2C();
void scanI2CBus(std::function<void(const char*)> logFunc);
bool initBME280();
bool initAS3935();
void readBME280Data(float &temperature, float &humidity, float &pressure);
void handleInterrupt(int interruptSource, void (*publishFunc)(const char*));
void AS3935_ISR();