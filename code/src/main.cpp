#include <sensors.h>
#include <webOTA.h>
#include <MQTT.h>
#include "credentials.h"
#include <WiFi.h>

// Pin definitions
#define LED_BUILTIN 2 // Built-in LED pin for ESP32

WiFiClient wifiClient;

// Timer for periodic updates
unsigned long currentTime = 0;
const unsigned long BME280_READ_INTERVAL = 1 * 60 * 1000; // 1 minute
unsigned long lastBME280ReadTime = 0;

String getResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON:
      return "Power on";
    case ESP_RST_EXT:
      return "External reset";
    case ESP_RST_SW:
      return "Software reset";
    case ESP_RST_PANIC:
      return "Panic reset";
    case ESP_RST_INT_WDT:
      return "Internal watchdog reset";
    case ESP_RST_TASK_WDT:
      return "Task watchdog reset";
    case ESP_RST_BROWNOUT:
      return "Brownout reset";
    case ESP_RST_SDIO:
      return "SDIO reset";
    default:
      return "Unknown";
  }
}

void setup() {
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT); // Built-in LED pin
  digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED initially

  Serial.begin(115200);
  Serial.println("Booting");
  
  // Initialize WiFi
  WiFi.mode(WIFI_STA); // Set WiFi mode to Station
  WiFi.setHostname(host); // Set the hostname for mDNS
  WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.waitForConnectResult() != WL_CONNECTED && (millis() - startTime < 15000)) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    // Flash the built-in LED to indicate failure
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, LOW); // Turn off the LED
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
      delay(500);
    }
    ESP.restart();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setupMQTT(host, wifiClient); // Initialize MQTT client
  logToMQTT("ESP32 Weather Station starting...");
  logToMQTT(WiFi.localIP().toString().c_str());

  // log the reason for the last reset
  String resetReason = getResetReason();
  logToMQTT("Last reset reason: ");
  logToMQTT(resetReason.c_str());
  
  logToMQTT("Initializing I2C and sensors...");

  // Initialize I2C
  initI2C();
  logToMQTT("I2C initialized");
  //scanI2CBus(logToMQTT); // Scan I2C bus and log results
  
  // Initialize BME280 sensor
  if (!initBME280()) {
    logToMQTT("BME280 initialization failed");
  } else {
    logToMQTT("BME280 sensor initialized successfully");
  }

  // Initialize AS3935 Lightning Sensor
  if (!initAS3935()) {
    logToMQTT("AS3935 initialization failed");
  } else {
    logToMQTT("AS3935 sensor initialized successfully");
  }

  logToMQTT("Initializing web server and OTA updates...");

  // Set up the webOTA server
  setupWebOTA(host, logToMQTT); // Pass the logging function to webOTA

  logToMQTT("Weather station is ready to go!");

  // Turn off the built-in LED after setup
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  // Code must be non-blocking to allow the server to handle requests

  handleWebServer(); // Handle web server requests

  // Ensure WiFi is connected
  static unsigned long lastWiFiCheck = 0;
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastWiFiCheck > 10000) { 
      Serial.println("WiFi disconnected, attempting to reconnect...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  // Periodically read BME280 sensor data and publish to MQTT
  currentTime = millis();
  if (currentTime - lastBME280ReadTime > BME280_READ_INTERVAL) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED to indicate reading
    Serial.println("Reading BME280 sensor data...");

    lastBME280ReadTime = currentTime;
    float temperature, humidity, pressure;
    readBME280Data(temperature, humidity, pressure);
    publishSensorData(temperature, mqtt_topic_temperature);
    publishSensorData(humidity, mqtt_topic_humidity);
    publishSensorData(pressure, mqtt_topic_pressure);

    digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED after reading
    Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%, Pressure: %.2f hPa\n", temperature, humidity, pressure);
  }

  // Check for lightning events
  if (AS3935IsrTrig) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED to indicate lightning detection
    delay(5);
    AS3935IsrTrig = 0; // Reset the interrupt flag
    int interruptSource = lightningSensor.getInterruptSrc();
    logInterrupt(interruptSource, logToMQTT); // Handle the interrupt and publish data

    switch (interruptSource)  // Using switch-case to handle different interrupt sources
    {
    case 1: // Lightning detected
      {
        Serial.println("Lightning detected!");
        uint8_t lightningPower = lightningSensor.getStrikeEnergyRaw();
        uint32_t lightningDistance = lightningSensor.getLightningDistKm();
        Serial.printf("Lightning Power: %d, Distance: %d\n", lightningPower, lightningDistance);
        // Publish lightning data to MQTT
        publishSensorData(lightningPower, mqtt_topic_lightning_power);
        publishSensorData(lightningDistance, mqtt_topic_lightning_distance);
        break;
      }
    case 2: // Disturber detected
      Serial.println("Disturber detected!");
      publishSensorData("Disturber detected", mqtt_topic_lightning_error);
      break;
    case 3: // Noise level too high
      Serial.println("Noise level too high!");
      publishSensorData("Noise level too high", mqtt_topic_lightning_error);
      break;
    default:
      break;
    }
    digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED after handling the interrupt
  }


}
