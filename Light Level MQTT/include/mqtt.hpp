#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

// Define the actual WiFi credentials in the double quotations.
String WIFI_SSID = "insert ssid here";
String WIFI_PASSWORD = "insert password here";

// MQTT Broker details
IPAddress MQTT_HOST(192,168,1,100); /* IP address of the MQTT broker */
uint16_t MQTT_PORT = 1883;
auto MQTT_TOPIC = "test"; /* Topic */

// Objects
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker publishRetryTimer;

// Global variables
String deviceName = "insert device name here";
String lastMessage = "insert initial message here";
bool messageAcknowledged = true;
uint16_t lastPacketId = 0;

void startMqttService(IPAddress mqttHost, uint16_t mqttPort, String wifiSSID, String wifiPassword);
void connectToWifi(String wifiSSID, String wifiPassword);
void connectToMqtt();
void WiFiEvent(WiFiEvent_t event);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttPublish(uint16_t packetId);
void retryPublish();
void publishMessage(String message);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void setTopic(const char* topic) {
  MQTT_TOPIC = topic;
}

void startMqttService(IPAddress mqttHost = MQTT_HOST, uint16_t mqttPort = MQTT_PORT, String wifiSSID = WIFI_SSID, String wifiPassword = WIFI_PASSWORD) {
  WiFi.onEvent(WiFiEvent); //Register WiFi event function

  mqttClient.setServer(mqttHost, mqttPort);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);

  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  // mqttClient.onMessage(onMqttMessage);

  connectToWifi(wifiSSID, wifiPassword); // Start the WiFi connection process
}

void connectToWifi(String wifiSSID, String wifiPassword) {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(wifiSSID, wifiPassword);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void wifiReconnect() {
  connectToWifi(WIFI_SSID, WIFI_PASSWORD);
}

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
          wifiReconnectTimer.once(2, wifiReconnect);
          break;
      default:
          break;
    }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe(MQTT_TOPIC, 0);

  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);
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

void retryPublish() {
  if (!messageAcknowledged) {
    Serial.println("Message not acknowledged. Retrying...");
    publishMessage(lastMessage);
  }
}

void publishMessage(String message) {
  lastPacketId = mqttClient.publish(MQTT_TOPIC, 1, true, message.c_str());
  messageAcknowledged = false;
  Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_TOPIC, lastPacketId);
  Serial.printf("Message: %s\n", message.c_str());

  publishRetryTimer.once(2, retryPublish);
  lastMessage = message;
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';
  Serial.println(message);
}
