#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "mqtt.hpp"

#define DHTPIN 13     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

// Define the actual WiFi credentials in the double quotations.
#define WIFI_SSID "dd-wrt"
#define WIFI_PASSWORD "Admin123"

// MQTT Broker details
#define MQTT_HOST IPAddress(192,168,1,100) /* IP address of the MQTT broker */
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "sensor/temperature" /* Topic */

// Global variables
String deviceId;
float temp;
unsigned long previousMillis = 0;
const long interval = 10000;

void setup() {
  Serial.begin(115200); // initialize serial
  
  dht.begin();
  // use the device MAC address as device id
  deviceId = WiFi.macAddress();

  setTopic(MQTT_PUB_TEMP); // set the MQTT topic to publish temperature readings
  startMqttService(MQTT_HOST, MQTT_PORT,WIFI_SSID, WIFI_PASSWORD); // initialize MQTT service
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && messageAcknowledged) {
    previousMillis = currentMillis;
    // read temperature from DHT sensor
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (!isnan(event.temperature)) {
      temp = event.temperature;

      // build JSON payload: {"device_id":"FF:FF:FF:FF:FF:FF", "temperature":25.0}
      String payload = "{";
      payload += "\"device_id\": \"" + deviceId + "\",";
      payload += " \"temperature\": " + String(temp, 1);
      payload += "}";

      publishMessage(payload);
    }
  }
}