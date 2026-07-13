#include "mqtt.hpp"

// Define the actual WiFi credentials in the double quotations.
#define WIFI_SSID "dd-wrt"
#define WIFI_PASSWORD "Admin123"

// MQTT Broker details
#define MQTT_HOST IPAddress(192,168,1,100) /* IP address of the MQTT broker */
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "sensors/light" /* Topic */

// Global variables
String deviceId;
float temperature;
unsigned long previousMillis = 0;
const long interval = 10000;

// --- Hardware ---
const int LDR_PIN = 33;  // GPIO33 (ADC1_CH6) — analog input

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);

  setTopic(MQTT_PUB_TEMP); // set the MQTT topic to publish temperature readings
  startMqttService(MQTT_HOST, MQTT_PORT,WIFI_SSID, WIFI_PASSWORD); // initialize MQTT service

  // use the device MAC address as device id
  deviceId = WiFi.macAddress();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && messageAcknowledged) {
    previousMillis = currentMillis;
    // read light level from LDR
    int   rawValue = analogRead(LDR_PIN);           // 0–4095
    //float voltage  = rawValue * (3.3f / 4095.0f);

    // build JSON payload: {"device_id":"FF:FF:FF:FF:FF:FF", "light":700.0}
    String payload = "{";
    payload += "\"device_id\": \"" + deviceId + "\",";
    payload += " \"light\": " + String(rawValue);
    payload += "}";

    publishMessage(payload);
  }
}