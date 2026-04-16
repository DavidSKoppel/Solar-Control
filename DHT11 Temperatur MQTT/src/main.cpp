#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#define DHTPIN 13     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

// Define the actual WiFi credentials in the double quotations.
#define WIFI_SSID "Tod's Tavern"
#define WIFI_PASSWORD "Lucky1808"

// MQTT Broker details
#define MQTT_HOST IPAddress(192,168,0,190)
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "test/temp" /* Topic */

// Objects
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker publishRetryTimer;

// Global variables
String deviceName = "TempSensor1";
float temp;
unsigned long previousMillis = 0;
const long interval = 10000;
bool messageAcknowledged = true;
uint16_t lastPacketId = 0;

// Function prototypes
void connectToWifi();
void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttPublish(uint16_t packetId);
void publishMessage();
void retryPublish();

//WiFi event handler function
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          connectToMqtt();
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
          wifiReconnectTimer.once(2, connectToWifi);
          break;
      default:
          break;
    }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged by broker on ");
  Serial.print(MQTT_HOST.toString());
  Serial.print("  packetId: ");
  Serial.println(packetId);
  if (packetId == lastPacketId) {
    messageAcknowledged = true;
    publishRetryTimer.detach();
  }
}

void publishMessage() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  temp = event.temperature;

  lastPacketId = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());
  messageAcknowledged = false;
  Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, lastPacketId);
  Serial.printf("Message: %.2f \n", temp);

  publishRetryTimer.once(2, retryPublish);
}

void retryPublish() {
  if (!messageAcknowledged) {
    Serial.println("Message not acknowledged. Retrying...");
    publishMessage();
  }
}

void setup() {
  Serial.begin(115200); // initialize serial
  WiFi.onEvent(WiFiEvent); //Register WiFi event function
  dht.begin();

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi(); // Start the WiFi connection process
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && messageAcknowledged) {
    previousMillis = currentMillis;
    publishMessage();
  }
}