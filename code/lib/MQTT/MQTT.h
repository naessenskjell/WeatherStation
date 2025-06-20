#pragma once
#include <PubSubClient.h>
#include <WiFiClient.h>

// MQTT credentials and topics (declare as extern, define in .cpp or main)
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* mqtt_topic_log;

extern PubSubClient mqttClient;

// Function declarations
void setupMQTT(const char* host, WiFiClient& wifiClient);
void reconnectToMQTT(const char* host);
void publishSensorData(float value, const char* topic);
void publishSensorData(const char* value, const char* topic);
void logToMQTT(const char* message);