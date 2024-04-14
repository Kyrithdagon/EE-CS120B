#include <Arduino.h>
/* Your Name & E-mail:...

   Discussion Section: 024
   
   Assignment: Lab #2  Exercise #3
   
   Exercise Description: The program will display the user input in hexadecimal form on a 
   one-digital 7-segment display.
   
   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work. (For this lab, however, I will be using my code from lab 1.)
*/

//Define digital pins here:
int LED_PINS[4] = {2, 3, 4, 5}; //assigned pins (switched)
int inputValue; //user's input
int inputBitValue; //user's input bit value
int bin[4];

//7-segment display pins and implementation (change pin values if necessary):
int a = 8;
int b = 9;
int c = 11;
int d = 12;
int e = 13;
int f = 7;
int g = 6;
int dp = 10;
int LEDS[7] {a, b, c, d, e, f, g};
//Some values for a-g are provided as examples, implement the remaining a-g values corresponding to the comments below:
//1 = Segment ON, 0 = Segment OFF
int nums[17][7] {
  {1, 1, 1, 1, 1, 1, 0}, //0
  {0, 1, 1, 0, 0, 0, 0}, //1
  {1, 1, 0, 1, 1, 0, 1}, //2
  {1, 1, 1, 1, 0, 0, 1}, //3
  {0, 1, 1, 0, 0, 1, 1}, //4 		<- Implement!
  {1, 0, 1, 1, 0, 1, 1}, //5 		<- Implement!
  {1, 0, 1, 1, 1, 1, 1}, //6 		<- Implement!
  {1, 1, 1, 0, 0, 0, 0}, //7 		<- Implement!
  {1, 1, 1, 1, 1, 1, 1}, //8 		<- Implement!
  {1, 1, 1, 1, 0, 1, 1}, //9 		<- Implement!
  {1, 1, 1, 0, 1, 1, 1}, //10 (A)
  {0, 0, 1, 1, 1, 1, 1}, //11 (b)
  {1, 0, 0, 1, 1, 1, 0}, //12 (C)	<- Implement in hex!
  {0, 1, 1, 1, 1, 0, 1}, //13 (d)	<- Implement in hex!
  {1, 0, 0, 1, 1, 1, 1}, //14 (E)	<- Implement in hex!
  {1, 0, 0, 0, 1, 1, 1}, //15 (F)	<- Implement in hex!
  {0, 0, 0, 0, 0, 0, 0} //off
// a  b  c  d  e  f  g
};

//Call this function in your code to output the integer x on the 7-segment display in hex
void outNum(int x) {
  for (int i = 0; i < 7; i++) {
    if (nums[x][i] == 1) {
      digitalWrite(LEDS[i], 1);
    }
    else {
      digitalWrite(LEDS[i], 0);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < 4; i++){  
  	pinMode(LED_PINS[i], OUTPUT);
  }
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Enter a value from 0-15: ");
  while(Serial.available() == 0){ }
  
  if(Serial.available() > 0){
  	inputValue = Serial.parseInt();
  	Serial.print("Entered value: ");
    Serial.println(inputValue);
  
    if(inputValue >= 0 && inputValue <= 15){
      outNum(inputValue);
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
        outNum(inputValue);
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
            outNum(inputBitValue);
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
