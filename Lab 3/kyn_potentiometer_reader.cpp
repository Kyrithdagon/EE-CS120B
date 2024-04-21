#include "Arduino.h"
int potentiometer = A5;

void setup(){
  Serial.begin(9600);
}

void loop(){
  int potVal = analogRead(potentiometer);
  Serial.println(potVal);
  delay(2000);
}
