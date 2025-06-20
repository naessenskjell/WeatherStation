// rename this file to example.h and change the credentials to yours

#pragma once
#include <IPAddress.h>

// WiFi credentials
const char* ssid = "ssid";
const char* password = "password";
const char* host = "WeatherStation";

// Network settings
IPAddress local_IP(192, 168, 0, 100);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

// webOTA credentials
const char* OTAUser = "user";
const char* OTAPassword = "password";

// MQTT credentials
const char* mqtt_server = "mqtt-broker.local";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt_user";
const char* mqtt_password = "password";

// MQTT topics
const char* mqtt_topic_temperature = "weather/data/temperature";
const char* mqtt_topic_humidity = "weather/data/humidity";
const char* mqtt_topic_pressure = "weather/data/pressure";
const char* mqtt_topic_lightning_power = "weather/data/lightning_power";
const char* mqtt_topic_lightning_distance = "weather/data/lightning_distance";
const char* mqtt_topic_lightning_error = "weather/data/lightning_error";
const char* mqtt_topic_log = "weather/log";