#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"

//#include "serialATmega.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #4  Exercise #1
   
   Exercise Description: For the first exercise, you will implement a counter:
      The value of the counter is shown on the 4-digit 7-segment display.
      The value of the count is controlled by the joystick.
      The counter should display values between 0 and 15, with any value greater than 9 encoded in hex (e.g. A-F).
      The counter should rollover (return to 0) if the count exceeds 15.
      The counter should roll back to 15, if the value falls below 0.
      
      The joystick can be moved in 4 cardinal directions: {up, down, left, right}. For this exercise, we only focus on the up and down directions.
        If the joystick is moved up, the counter increments
        If the joystick is moved down, the counter decrements.
        If the button is pressed, the count resets to 0.

   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

void ADC_init() {
  ADMUX = (1<<REFS0);
	ADCSRA|= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

unsigned int ADC_read(unsigned char chnl){
  uint8_t low, high;

  ADMUX  = (ADMUX & 0xF8) | (chnl & 7);
  ADCSRA |= 1 << ADSC ;
  while((ADCSRA >> ADSC) & 0x01){}

	low  = ADCL;
	high = ADCH;

	return ((high << 8) | low) ;
}

int nums[16] = {0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011, 0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011, 0b1110111, 0b0011111, 0b1001110, 0b0111101, 0b1001111, 0b1000111 }; 
// a  b  c  d  e  f  g

void outNum(int num){
	PORTD = nums[num] << 1; //assigns bits 1-7 of nums(a-f) to pins 2-7 of port d
  PORTB = SetBit(PORTB, 0 ,nums[num] & 0x01); // assigns bit 0 of nums(g) to pin 0 of port b
}

//directions[] and outDir() will be neeeded for ex 2 and 3
int directions[4] = {0b0111110, 0b0111101, 0b0000101, 0b0001110}; //TODO: complete the array containg the values needed for the 7 segments for each of the 4 directions
// a  b  c  d  e  f  g
//TODO: display the direction to the 7-seg display. HINT: will be very similar to outNum()
void outDir(int dir){
  PORTD = directions[dir] << 1;
  PORTB = SetBit(PORTB, 0, directions[dir] & 0x01);
}

int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //8 phases of the stepper motor step

enum states {INIT, HOLD_INC, HOLD_DEC, INC, DEC, buttonPress} state; //TODO: finish the enum for the SM

unsigned char count = 0;
unsigned char timer;

void Tick() {
  //State Transistions
  //TODO: complete transitions 
  switch(state) {
    case INIT:
      if(!GetBit(PINC, 2)){
        state = buttonPress;
      }
      else if(ADC_read(0) >= 600 && GetBit(PINC, 2)){
        state = HOLD_INC;
      }
      else if(ADC_read(0) <= 400 && GetBit(PINC, 2)){
        state = HOLD_DEC;
      }
      break;

    case HOLD_INC:
      if(ADC_read(0) >= 600){
        state = HOLD_INC;
      }
      else{
        state = INC;
      }
      break;

    case HOLD_DEC:
      if(ADC_read(0) <= 400){
        state = HOLD_DEC;
      }
      else{
        state = DEC;
      }
      break;

    case INC: //it will just return to INIT waiting for next repsonse
      state = INIT;
      break;

    case DEC: //it will just return to INIT waiting for next repsonse
      state = INIT;
      break;

    case buttonPress: //it will just return to INIT waiting for next repsonse
      state = INIT;
      break;

    default:
      state = INIT;
      break;
  }

  // State Actions
  //TODO: complete transitions
  switch(state) {
    case INIT:
      outNum(count);
      break;

    case INC:
      if(count == 15){
        count = 0;
      }
      else{
        count++;
      }
      outNum(count & 0xFF);
      break;

    case DEC:
      if(count == 0){
        count = 15;
      }
      else{
        count--;
      }
      outNum(count & 0xFF);
      break;

    case buttonPress:
      count = 0;
      outNum(count & 0xFF);
      break;

    default:
      break;
  }
}

int main(void)
{
	//TODO: initialize all outputs and inputs
  DDRB = 0b11111111; PORTB = 0b00000000; //turning all of port B into outputs
  DDRC = 0b00111000; PORTC = 0b11000111; //turning used pins of port C into outputs or inputs
  DDRD = 0b11111111; PORTD = 0b00000000; //turning all of port D into outputs

  ADC_init();//initializes the analog to digital converter
	
  state = INIT;

  TimerSet(250); //period of 1 ms. good period for the stepper mottor
  TimerOn();

    while (1)
    {
		  Tick();      // Execute one synchSM tick
      while (!TimerFlag){}  // Wait for SM period
      TimerFlag = 0;        // Lower flag
     }

    return 0;
}
