#include <Arduino.h>
#include <Stepper.h>
#include "mqtt.hpp"

// Defines the number of steps per rotation
const int stepsPerRevolution = 2048;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 27, 12, 13, 14);

void setup() {
  Serial.begin(115200); // initialize serial
  setWiFi("Emil's Galaxy S22 Ultra", "gruppe7!"); // set WiFi credentials
  startMqttService(); // initialize MQTT service
  
  myStepper.setSpeed(10); // 15 RPM is the maximum speed for this motor, but it can be set lower to reduce noise and increase torque
  myStepper.step(stepsPerRevolution);
}

void loop() {
  //unsigned long currentMillis = millis();

  /*
  if (currentMillis - previousMillis >= interval && messageAcknowledged) {
    previousMillis = currentMillis;
    publishMessage(lastMessage);
  }*/

  delay(1000);
  /*// Rotate CW slowly at 5 RPM
  myStepper.setSpeed(5);
  myStepper.step(stepsPerRevolution);
  delay(1000);

  // Rotate CCW quickly at 10 RPM
  myStepper.setSpeed(10);
  myStepper.step(-stepsPerRevolution);
  delay(1000);*/
}