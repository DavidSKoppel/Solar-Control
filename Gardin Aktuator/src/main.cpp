#include <Arduino.h>
#include <Stepper.h>
#include <ArduinoJson.h>
#include "mqtt.hpp"

// --------------------------------------------------
// WiFi
// --------------------------------------------------

#define WIFI_SSID     "dd-wrt"
#define WIFI_PASSWORD "Admin123"

// --------------------------------------------------
// MQTT
// --------------------------------------------------

#define MQTT_HOST IPAddress(192,168,1,100)
#define MQTT_PORT 1883

#define MQTT_TOPIC_ENV "sensors/environment"
#define MQTT_TOPIC_VAR "sensors/variables"

// --------------------------------------------------
// Stepper Configuration
// --------------------------------------------------

const int STEPS_PER_REVOLUTION = 2048;
const int MAX_STEPS = STEPS_PER_REVOLUTION * 10.25;

Stepper myStepper(
    STEPS_PER_REVOLUTION,
    27,   // IN1
    12,   // IN3
    13,   // IN2
    14    // IN4
);

// --------------------------------------------------
// Device State
// --------------------------------------------------

String deviceId;

float temperatureThreshold = 20.0;
float lightThreshold = 700.0;

bool variablesRequested = false;

bool moveRequested = false;
int stepsToMove = 0;

int currentSteps = 0;

// --------------------------------------------------
// Helper Functions
// --------------------------------------------------

void requestVariables()
{
    JsonDocument doc;

    doc["request"] = "variables";
    doc["device"] = deviceId;

    String payload;
    serializeJson(doc, payload);

    mqttClient.publish(
        MQTT_TOPIC_VAR,
        1,
        false,
        payload.c_str());

    Serial.println("Requested variables");
}

bool isTargetDevice(const String& target)
{
    return target.equalsIgnoreCase("all") ||
           target.equalsIgnoreCase(deviceId);
}

void scheduleClose()
{
    if (currentSteps >= MAX_STEPS)
        return;

    moveRequested = true;
    stepsToMove = MAX_STEPS;
}

void scheduleOpen()
{
    if (currentSteps <= 0)
        return;

    moveRequested = true;
    stepsToMove = -MAX_STEPS;
}

// --------------------------------------------------
// Variables Topic Handler
// --------------------------------------------------

void handleVariables(JsonDocument& doc)
{
    String target = doc["device"] | "";

    if (!isTargetDevice(target))
    {
        Serial.println("Configuration not for this device");
        return;
    }

    if (doc["temperatureThreshold"].is<float>())
    {
        temperatureThreshold =
            doc["temperatureThreshold"];

        Serial.print("Temperature threshold set to: ");
        Serial.println(temperatureThreshold);
    }

    if (doc["lightThreshold"].is<float>())
    {
        lightThreshold =
            doc["lightThreshold"];

        Serial.print("Light threshold set to: ");
        Serial.println(lightThreshold);
    }

    publishMessage(
        String("Variables applied on ") + deviceId);
}

// --------------------------------------------------
// Environment Topic Handler
// --------------------------------------------------

void handleEnvironment(JsonDocument& doc)
{
    float temperature =
        doc["temperature"] | NAN;

    float light =
        doc["light"] | NAN;

    Serial.println("----- Sensor Reading -----");

    if (!isnan(temperature))
    {
        Serial.print("Temperature: ");
        Serial.println(temperature);
    }

    if (!isnan(light))
    {
        Serial.print("Light: ");
        Serial.println(light);
    }

    bool tempExceeded =
        !isnan(temperature) &&
        temperature > temperatureThreshold;

    bool lightExceeded =
        !isnan(light) &&
        light > lightThreshold;

    if (tempExceeded || lightExceeded)
    {
        Serial.println("Threshold exceeded -> CLOSE");

        scheduleClose();
    }
    else
    {
        Serial.println("Threshold normal -> OPEN");

        scheduleOpen();
    }
}

// --------------------------------------------------
// MQTT Callback
// --------------------------------------------------

void OnMessageReceived(
    char* topic,
    char* payload,
    AsyncMqttClientMessageProperties properties,
    size_t len,
    size_t index,
    size_t total)
{
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, payload, len);

    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    Serial.println();
    Serial.print("Message received on: ");
    Serial.println(topic);

    if (strcmp(topic, MQTT_TOPIC_VAR) == 0)
    {
        handleVariables(doc);
    }
    else if (strcmp(topic, MQTT_TOPIC_ENV) == 0)
    {
        handleEnvironment(doc);
    }
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    addSubscriptionTopic(MQTT_TOPIC_ENV);
    addSubscriptionTopic(MQTT_TOPIC_VAR);

    startMqttService(
        MQTT_HOST,
        MQTT_PORT,
        WIFI_SSID,
        WIFI_PASSWORD);

    mqttClient.onMessage(OnMessageReceived);

    myStepper.setSpeed(10);

    deviceId = WiFi.macAddress();

    Serial.print("Device ID: ");
    Serial.println(deviceId);
}

// --------------------------------------------------
// Main Loop
// --------------------------------------------------

void loop()
{
    // Request configuration once after MQTT connects
    if (!variablesRequested &&
        mqttClient.connected())
    {
        requestVariables();
        variablesRequested = true;
    }

    // Execute motor movement outside MQTT callback
    if (moveRequested)
    {
        moveRequested = false;

        Serial.print("Moving stepper: ");
        Serial.println(stepsToMove);

        myStepper.step(stepsToMove);

        currentSteps += stepsToMove;

        Serial.print("Current position: ");
        Serial.println(currentSteps);

        stepsToMove = 0;
    }
}