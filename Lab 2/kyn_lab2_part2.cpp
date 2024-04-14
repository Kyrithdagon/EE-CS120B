#include <Arduino.h>
/* Your Name & E-mail:...

   Discussion Section: 024
   
   Assignment: Lab #2  Exercise #2
   
   Exercise Description: Here, your program will prompt the user to select a bit (LED) 
   from the output LEDs to mask and shift to the right-most LED.
   
   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work. (For this lab, however, I will be using my code from lab 1.)
*/

int LED_PINS[4] = {2, 3, 4, 5}; //assigned pins (switched)
int inputValue; //user's input
int inputBitValue; //user's input bit value
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
        bin[i] = inputValue & 0x01; //had to switch order otherwise it kept not working
        inputValue >>= 1;
      }
  	  for(int i = 0; i < 4; i++){
  		  digitalWrite(LED_PINS[i], bin[i]);
  	  }
    }
  }
  delay(1000); 

  Serial.println("Which bit would you like to right shift? (Enter a value from 0-3)");
  while(Serial.available() == 0){ }

  if(Serial.available() > 0){
    inputBitValue = Serial.parseInt();
    Serial.print("Entered value: ");
    Serial.println(inputBitValue);

    if(inputBitValue >= 0 && inputBitValue <= 3){
      int state = digitalRead(LED_PINS[inputBitValue]);
      if(state == 0){
        for(int i = 0; i < 4; i++){
          digitalWrite(LED_PINS[i], inputValue);
          inputValue >>= 1;
          delay(600);
        }
      }
      else{
        if(inputBitValue == 0){
          return;
        }
        else{
          for(int i = 0; i < 4; i++){
            digitalWrite(LED_PINS[i], i == inputBitValue);
          }
          for(int i = inputBitValue; i >= 0; i--){
            delay(600);
            for(int k = 0; k < 4; k++){
              digitalWrite(LED_PINS[k], k == i);
            }
          }
        }
      }
    }
  }
  delay(1000);
}
