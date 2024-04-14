#include <Arduino.h>
/* Your Name & E-mail:...

   Discussion Section: 024
   
   Assignment: Lab #2  Exercise #1
   
   Exercise Description: In this exercise you will convert an integer value provided by the user 
   (via the Serial Monitor) to its 4-bit binary representation using the four LEDs on your breadboard. 
   This exercise is identical to Laboratory Assignment #1, except for the fact that you will be programming 
   your microcontroller and lighting up LEDs on your breadboard, rather than using TinkerCAD.
   
   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work. (For this lab, however, I will be using my code from lab 1.)
*/

int LED_PINS[4] = {5, 4, 3, 2}; //assigned pins
int inputValue; //user's input
int bin[4];

void setup(){
  for(int i = 0; i < 4; i++){  
  	pinMode(LED_PINS[i], OUTPUT);
  }
  Serial.begin(9600);
}

void loop(){
  Serial.println("Enter a value from 0-15: ");
  while(Serial.available() == 0){ }
  
  if(Serial.available() > 0){
  	inputValue = Serial.parseInt();
  	Serial.print("Entered value: ");
    Serial.println(inputValue);
  
    if(inputValue >= 0 && inputValue <= 15){
      for(int i = 0; i < 4; i++){
        bin[i] = inputValue & 0x08;
        inputValue <<= 1;
      }
  	  for(int i = 0; i < 4; i++){
  		digitalWrite(LED_PINS[i], bin[i]);
  	  }
    }
  }
  delay(1000); // Wait for 1000 millisecond(s)
}
