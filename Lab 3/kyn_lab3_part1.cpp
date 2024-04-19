#include "Arduino.h"
#include "timerISR.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #3  Exercise #1
   
   Exercise Description: This exercise will implement a bathroom light switch using the button and photoresistor. The RGB LED will act a the bathroom light. 
    The system is initially off.
    Pressing the button turns on the RGB LED. 
    Pressing the button a second time turn off the RGB LED.
    The RGB LED can display any color that you choose. 
    Covering the photoresistor or moving it to a dark area turns the RGB LED on. 
      The RGB LED will remain illuminated for 9 seconds
      The 9-second timer resets if the user covers the photoresistor or moves the breadboard to a dark area. 
   
   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

//Define digital pins here:
int button = 5;
int photoresistor = A4;
int redWireRGB = 11;
int blueWireRGB = 10;
int greenWireRGB = 9;
//int potentiometer = A5;

//You will need to determine the value for the dark_threshold based on the light level in your room
//(Tip: Use Serial.println() to output the photoresistor value in your current room and set the dark_threshold to a value slightly below that)
int dark_threshold = 150; //threshold of my room was 285-289

//4 digit 7-segment display pins and implementation (you will only be using the first digit in this lab):
int a = 8;
int b = 2;
int c = 4;
int d = 12;
int e = 13;
int f = 7;
int g = 6;
int d1 = A1;
int LEDS[7] {a, b, c, d, e, f, g};
int nums[11][7] {
  {1, 1, 1, 1, 1, 1, 0}, //0
  {0, 1, 1, 0, 0, 0, 0}, //1
  {1, 1, 0, 1, 1, 0, 1}, //2
  {1, 1, 1, 1, 0, 0, 1}, //3
  {0, 1, 1, 0, 0, 1, 1}, //4
  {1, 0, 1, 1, 0, 1, 1}, //5
  {1, 0, 1, 1, 1, 1, 1}, //6
  {1, 1, 1, 0, 0, 0, 0}, //7
  {1, 1, 1, 1, 1, 1, 1}, //8
  {1, 1, 1, 1, 0, 1, 1}, //9
  {0, 0, 0, 0, 0, 0, 0} //off
// a  b  c  d  e  f  g
};

//Call this function in your code to output the integer x on the first digit of the 4 digit 7-segment display
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

enum states {INIT, ONHold, OFFHold, ButtonRGB, PRRGB} state; //TODO: decide how many states your SM will use

unsigned char timer = 9;
unsigned char i;

//TODO: Implement your synch SM
void Tick() {
  // State Transistions
  switch(state) {
    case INIT:
      if(digitalRead(button) == HIGH){
        state = ONHold;
      }
      else if(analogRead(photoresistor) < dark_threshold){
        state = PRRGB;
      }
      break;

    case ONHold:
      if(digitalRead(button) == LOW){
        state = ButtonRGB;
      }
      break;

    case OFFHold:
      if(digitalRead(button) == LOW){
        state = INIT;
      }
      break;
    
    case ButtonRGB:
      if(digitalRead(button) == HIGH){
        state = INIT;
      }
      else{
        state = ButtonRGB;
      }
      break;

    case PRRGB:
      if(analogRead(photoresistor) < dark_threshold){
        i = 0;
        state = PRRGB;
      }
      else if(i >= timer){
        state = INIT;
        i = 0;
      }
      else if(digitalRead(button) == HIGH){
        state = INIT;
      }
      break;

    default:
      state = INIT;
      break;
  }
  // State Actions
  switch(state) {
    case INIT:
      analogWrite(redWireRGB, 0);
      analogWrite(blueWireRGB, 0);
      analogWrite(greenWireRGB, 0);
      break;
    
    case ONHold:
      analogWrite(greenWireRGB, 100);
      break;

    case OFFHold:
      analogWrite(greenWireRGB, 0);
      break;

    case ButtonRGB:
      analogWrite(greenWireRGB, 100);
      break;

    case PRRGB:
      analogWrite(redWireRGB, 100);
      i++;
      break;

    default:
      break;
  }
}

void setup() {
	for (int i = 0; i < 7; i++) {
      pinMode(LEDS[i], OUTPUT);
    }
    pinMode(d1, OUTPUT);
	// TODO: setup remaining inputs and outputs
    pinMode(redWireRGB, OUTPUT);
    pinMode(blueWireRGB, OUTPUT);
    pinMode(greenWireRGB, OUTPUT);
    pinMode(button, INPUT);
    pinMode(photoresistor, INPUT);
    //pinMode(potentiometer, ...);

    state = INIT; //initilzes initila state of SM
    TimerSet(1000);//sets the timer to 1 second
    TimerOn();//starts the timer
}

void loop() {
  Tick();//Calls a single SM tick
  while(!TimerFlag){} //Waits for the timer to go off
  TimerFlag = 0; //Resets the timer flag
}
