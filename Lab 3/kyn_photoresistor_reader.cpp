#include "Arduino.h"
int photoresistor = A4;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int sensorValue = analogRead(photoresistor); 
  Serial.println(sensorValue);
  delay(1000); 
}
