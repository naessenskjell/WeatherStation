#include <MQTT.h>
#include <Arduino.h>

// These should be defined in main.cpp, so only declare them here if needed
// const char* mqtt_server = "192.168.0.21";
// int mqtt_port = 1883;
// const char* mqtt_user = "mqtt_user";
// const char* mqtt_password = "password";
// const char* mqtt_topic_log = "weather/log";

PubSubClient mqttClient;

void setupMQTT(const char* host, WiFiClient& wifiClient) {
    mqttClient.setClient(wifiClient);
    mqttClient.setServer(mqtt_server, mqtt_port);
}

void reconnectToMQTT(const char* host) {
    if (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT server...");
        if (mqttClient.connect(host, mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again");
            delay(100);
        }
    }
}

void publishSensorData(float value, const char* topic) {
    Serial.printf("Publishing to topic %s: %f\n", topic, value);
    char payload[32];
    dtostrf(value, 6, 2, payload);

    if (!mqttClient.connected()) {
        reconnectToMQTT("WeatherStation");
    }
    if (mqttClient.connected()) {
        if (mqttClient.publish(topic, payload)) {
            Serial.printf("Published to %s: %s\n", topic, payload);
        } else {
            Serial.println("MQTT publish failed");
        }
    } else {
        Serial.println("MQTT not connected");
    }
}

void publishSensorData(const char* value, const char* topic) {
    Serial.printf("Publishing to topic %s: %s\n", topic, value);

    if (!mqttClient.connected()) {
        reconnectToMQTT("WeatherStation");
    }
    if (mqttClient.connected()) {
        if (mqttClient.publish(topic, value)) {
            Serial.printf("Published to %s: %s\n", topic, value);
        } else {
            Serial.println("MQTT publish failed");
        }
    } else {
        Serial.println("MQTT not connected");
    }
}

void logToMQTT(const char* message) {
    Serial.printf("Logging to MQTT: %s\n", message);

    if (!mqttClient.connected()) {
        reconnectToMQTT("WeatherStation");
    }
    if (mqttClient.connected()) {
        if (mqttClient.publish(mqtt_topic_log, message)) {
            Serial.printf("Logged: %s\n", message);
        } else {
            Serial.println("MQTT log publish failed");
        }
    } else {
        Serial.println("MQTT not connected");
    }
}