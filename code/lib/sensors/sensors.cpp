#include <sensors.h>
#include <Arduino.h>

// Sensor instances
DFRobot_AS3935_I2C lightningSensor(IRQ_PIN, I2C_ADDRESS_AS3935);
Adafruit_BME280 bme;

volatile int8_t AS3935IsrTrig = 0;
volatile time_t AS3935IsrTrigTime = 0;

void IRAM_ATTR AS3935_ISR() {
    AS3935IsrTrig = 1;
    AS3935IsrTrigTime = time(nullptr); // Update the time when the interrupt occurs
}

bool initBME280() {
    const int maxRetries = 5;
    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        if (bme.begin(I2C_ADDRESS_BME280, &Wire)) {
            return true;
        }
        delay(200); // Wait 200ms before retrying
    }
    return false;
}

void initI2C() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("I2C initialized");
}

void scanI2CBus(std::function<void(const char*)> logFunc) {
    logFunc("Scanning I2C bus...");
    byte error, address;
    int nDevices = 0;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            nDevices++;
            String msg = "I2C device found at address: " + String(address, HEX);
            logFunc(msg.c_str());
        }
    }
    if (nDevices == 0) {
        String msg = String(nDevices) + " no I2C devices found.";
        logFunc(msg.c_str());
    }
}

bool initAS3935() {
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        if (lightningSensor.begin() == 0) {
            break;
        }
        delay(100);
    }
    if (lightningSensor.begin() != 0) {
        return false; // Initialization failed
    } else {
        lightningSensor.defInit();
        lightningSensor.powerUp();
        lightningSensor.setTuningCaps(AS3935_CAPACITANCE);
        if (AS3935_MODE == AS3935_OUTDOORS) {
            lightningSensor.setOutdoors();
        } else {
            lightningSensor.setIndoors();
        }
        if (AS3935_DISTURBER_DETECTION) {
            lightningSensor.disturberEn();
        } else {
            lightningSensor.disturberDis();
        }
        lightningSensor.setIRQOutputSource(0);
        lightningSensor.setNoiseFloorLvl(AS3935_NOISE_LEVEL);
        lightningSensor.setWatchdogThreshold(AS3935_WATCHDOG_THRESHOLD);
        lightningSensor.setSpikeRejection(AS3935_SPIKE_REJECTION);
        attachInterrupt(digitalPinToInterrupt(IRQ_PIN), AS3935_ISR, RISING);
    }
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Set NTP servers for time synchronization
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(100); // Wait for time to be set
        now = time(nullptr);
    }
    return true; // Initialization successful
}

void readBME280Data(float &temperature, float &humidity, float &pressure) {
    temperature = bme.readTemperature() +  BME280_TEMPERATURE_OFFSET;
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;

    if ((isnan(temperature) || isnan(humidity) || isnan(pressure))) {
        if (initBME280()) { // Try to reinitialize if data is invalid
            temperature = bme.readTemperature() + BME280_TEMPERATURE_OFFSET;
            humidity = bme.readHumidity();
            pressure = bme.readPressure() / 100.0F;
        }
    }
}

void logInterrupt(int interruptSource, void (*publishFunc)(const char*)) {
    String message;
    String interruptTime = String(ctime((const time_t*)&AS3935IsrTrigTime));
    interruptTime.trim(); // Convert time to string and trim whitespace
    message += interruptTime;
    message += " - Interrupt source: ";
    switch (interruptSource) {
        case 0:
            message += "No interrupt detected.";
            break;
        case 1:
            message += "Lightning detected!";
            break;
        case 2:
            message += "Disturber detected!";
            break;
        case 3:
            message += "Noise level too high!";
            break;
        default:
            message += "Unknown interrupt source.";
            break;
    }
    Serial.println(message);
    if (publishFunc) {
        publishFunc(message.c_str());
    }
}
