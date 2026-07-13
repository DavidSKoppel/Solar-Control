#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

// Define the actual WiFi credentials in the double quotations.
auto WIFI_SSID = "insert ssid here";
auto WIFI_PASSWORD = "insert password here";

// MQTT Broker details
auto MQTT_HOST = IPAddress(192,168,1,100);
auto MQTT_PORT = 1883;
auto MQTT_TOPIC = "test/temp"; /* Topic */

// Dynamic subscription list
const int MAX_SUB_TOPICS = 12;
String SUB_TOPICS[MAX_SUB_TOPICS];
int subTopicCount = 0;

// Objects
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker publishRetryTimer;

// Global variables
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
void addSubscriptionTopic(const char * topic);

// Register a topic to subscribe to when MQTT connects (stored in memory)
void addSubscriptionTopic(const char * topic) {
  if (topic == nullptr) return;
  if (subTopicCount >= MAX_SUB_TOPICS) {
    Serial.println("Subscription list full, cannot add topic");
    return;
  }
  SUB_TOPICS[subTopicCount++] = String(topic);
}

void startMqttService(IPAddress mqttHost = MQTT_HOST, uint16_t mqttPort = MQTT_PORT, String wifiSSID = WIFI_SSID, String wifiPassword = WIFI_PASSWORD) {
  WiFi.onEvent(WiFiEvent); //Register WiFi event function

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(mqttHost, mqttPort);

  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);

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
  // Subscribe to all registered topics
  for (int i = 0; i < subTopicCount; ++i) {
    const char* t = SUB_TOPICS[i].c_str();
    uint16_t packetIdSub = mqttClient.subscribe(t, 0);
    Serial.print("Subscribing to topic: ");
    Serial.print(t);
    Serial.print("  packetId: ");
    Serial.println(packetIdSub);
  }
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
  Serial.printf("Message: %.2f \n", message);

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
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}
