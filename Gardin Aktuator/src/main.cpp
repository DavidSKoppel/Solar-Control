#include <Arduino.h>
#include <Stepper.h>
#include "mqtt.hpp"

// Defines the number of steps per rotation
const int stepsPerRevolution = 2048;

auto temperature = 20.0; // Variable to store the temperature value
auto lightIntensity = 700.0; // Variable to store the light intensity value

bool moveRequested = false;
int currentSteps = 0;
int stepsToMove = 0;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 27, 12, 13, 14);

void driveMotor(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';
  Serial.print("MQTT message arrived on topic ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);

  float receivedTemperature = 0.0;
  float receivedLight = 0.0;
  bool hasTemperature = false;
  bool hasLight = false;

  char* found;
  found = strstr(message, "temperature");
  if (found) {
    found = strchr(found, ':');
    if (found) {
      receivedTemperature = atof(found + 1);
      hasTemperature = true;
    }
  }

  found = strstr(message, "light");
  if (found) {
    found = strchr(found, ':');
    if (found) {
      receivedLight = atof(found + 1);
      hasLight = true;
    }
  }

  Serial.print("Parsed temperature = ");
  Serial.println(receivedTemperature);
  Serial.print("Parsed light = ");
  Serial.println(receivedLight);

  bool exceedTemperature = hasTemperature && receivedTemperature > temperature;
  bool exceedLight = hasLight && receivedLight > lightIntensity;

  if (exceedTemperature || exceedLight) {
    Serial.println("Threshold exceeded -> rotating stepper.");
    moveRequested = true;
    stepsToMove = stepsPerRevolution*12; // Set the number of steps to move
    //myStepper.setSpeed(10);
    //myStepper.step(stepsPerRevolution);
  }
  else {
    Serial.println("Threshold below -> rotating stepper.");
    moveRequested = true;
    stepsToMove = -stepsPerRevolution*12; // Set the number of steps to move
    //myStepper.setSpeed(10);
    //myStepper.step(-stepsPerRevolution);
  }
}

void setup() {
  Serial.begin(115200); // initialize serial
  //setWiFi("Tod's Tavern", "Lucky1808"); // set WiFi credentials
  setWiFi("dd-wrt", "Admin123"); // set WiFi credentials
  startMqttService(); // initialize MQTT service
  mqttClient.onMessage(driveMotor);
}

void loop() {
  if (moveRequested)
  {
    moveRequested = false;
    myStepper.setSpeed(10);
    myStepper.step(stepsToMove);  // now outside MQTT callback
    stepsToMove = 0; // reset steps to move after executing
  }
}