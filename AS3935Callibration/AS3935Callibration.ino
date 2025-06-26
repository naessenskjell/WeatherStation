#include "DFRobot_AS3935_I2C.h"

#if defined(ESP32) || defined(ESP8266)
#define IRQ_PIN 4 // Use a valid interrupt pin (e.g., GPIO4 for ESP32/ESP8266)
#else
#define IRQ_PIN 2 // Pin 2 for Arduino Uno
#endif

#define AS3935_CAPACITANCE 0
#define AS3935_I2C_ADDR AS3935_ADD3

DFRobot_AS3935_I2C lightning0((uint8_t)IRQ_PIN);

uint8_t currentCap = AS3935_CAPACITANCE;
volatile unsigned long pulseCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("DFRobot AS3935 lightning sensor calibration begin!");

  // Set I2C address
  lightning0.setI2CAddress(AS3935_I2C_ADDR);
  
  // Initialize sensor
  int beginResult = lightning0.begin();
  if (beginResult != 0) {
    Serial.print("I2C initialization failed! Error code: ");
    Serial.println(beginResult);
    Serial.println("Check I2C wiring, pull-up resistors (4.7kΩ to 3.3V), and address pins (A0=1, A1=1 for ADD3).");
    while (1);
  }

  // Configure sensor
  lightning0.defInit();
  lightning0.powerUp();
  lightning0.setOutdoors(); // or setIndoors() if needed
  lightning0.setTuningCaps(currentCap);

  // Configure for frequency measurement
  lightning0.setLcoFdiv(0); // Divide by 16 (~31.25 kHz)
  delay(10); // Allow LCO to stabilize
  lightning0.setIRQOutputSource(3); // Output LCO frequency on IRQ pin
  delay(10); // Allow register to take effect

  // Set up interrupt for pulse counting
  pinMode(IRQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), countPulse, RISING);

  Serial.println("Calibration mode: Measuring frequency on IRQ pin (div/16).");
  Serial.println("Target frequency: 31.25 kHz ± 3.5% (30.156–32.344 kHz)");
}

// Interrupt service routine to count pulses
void countPulse() {
  pulseCount++;
}

void loop() {
  // Take 15 measurements for the current capacitance
  unsigned long totalPulses = 0;
  int measurements = 15;
  
  for (int i = 0; i < measurements; i++) {
    // Reset pulse count and start 500ms measurement window
    noInterrupts();
    pulseCount = 0;
    interrupts();

    unsigned long startMillis = millis();
    while (millis() - startMillis < 500) {
      // Wait for 500ms to count pulses
    }

    // Read pulse count
    noInterrupts();
    totalPulses += pulseCount;
    interrupts();

    // Short delay between measurements
    delay(10);
  }

  // Calculate average pulses and frequency
  float avgPulses = totalPulses / (float)measurements;
  float frequencyHz = avgPulses * 2.0; // pulses / 0.5s = pulses * 2 Hz

  // Report average frequency
  Serial.print("Capacitance: ");
  Serial.print(currentCap * 8);
  Serial.print(" pF, Average Pulses: ");
  Serial.print(avgPulses);
  Serial.print(", Average Frequency: ");
  Serial.print(frequencyHz);
  Serial.print(" Hz");
  if (frequencyHz >= 30156 && frequencyHz <= 32344) {
    Serial.println(" (Within target range)");
  } else if (totalPulses == 0) {
    Serial.println(" (No pulses detected! Check IRQ pin wiring, antenna, or sensor power (3.3V))");
  } else {
    Serial.println(" (Outside target range)");
  }

  // Adjust capacitance
  currentCap++;
  if (currentCap > 15) {
    currentCap = 0;
    Serial.println("Max capacitance reached, resetting to 0.");
    Serial.println("-------------------");
  }
  lightning0.setTuningCaps(currentCap);
  delay(200); // Allow LCO to stabilize

}