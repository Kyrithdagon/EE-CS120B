#include "Arduino.h"
int photoresistor = A4; // Define the analog pin the photoresistor is connected to

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
}

void loop() {
  int sensorValue = analogRead(photoresistor); // Read the analog value from the photoresistor
  Serial.println(sensorValue); // Print the sensor value to the serial monitor
  delay(1000); // Wait for a short time to avoid flooding the serial monitor
}
