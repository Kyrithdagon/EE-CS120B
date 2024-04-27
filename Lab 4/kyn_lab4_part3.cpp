#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"


//WARNING: THIS CODE IS INCOMPLETE AND DOES NOT WORK


//#include "serialATmega.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #4  Exercise #3
   
   Exercise Description: This exercise extends the previous exercise with functionality to change the stored passcode. 
    The passcode can only be changed when the lock is unlocked
    Press the joystick's button to change the password
    The LED turns on to indicate that the lock will now accept a new passcode
    The next four cardinal direction joystick moves {up, down, left, right} will overwrite the old passcode.
    The LED turns off when the passcode has been updated. 

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
int directions[4] = {0b0111110, 0b0111101, 0b0000101, 0b0001110}; //TODO: copmlete the array containg the values needed for the 7 sgements for each of the 4 directions
// a  b  c  d  e  f  g
//TODO: display the direction to the 7-seg display. HINT: will be very similar to outNum()
void outDir(int dir){
  PORTD = directions[dir] << 1;
  PORTB = SetBit(PORTB, 0, directions[dir] & 0x01);
}

int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //8 phases of the stepper motor step
int phases2[8] = {0b1001, 0b1000, 0b1100, 0b0100, 0b0110, 0b0010, 0b0011, 0b0001};

enum states {INIT, U, D, L, R, CHECK, UNLOCK, LOCK, NEW_PASS} state; //TODO: finish the enum for the SM

unsigned char passcode[4] = {1, 2, 3, 4}; //basically will be up, down, left, right

unsigned char inputArray[4] = {}; //will be used to store user inputs via joystick 
unsigned char input = 0; //to increment through inputArray
unsigned char i; //used to go through phases
unsigned short limit_Rota = 0;
unsigned char read_again = 0;
unsigned char passcode_changed = 0;

void Tick() {
  // State Transistions
  //TODO: complete transitions 
  switch(state) {
    case INIT:
      if((PORTC = PORTC & ~(1 << 4)) && !GetBit(PINC, 2)){
        state = NEW_PASS;
      }
      if(input == 4){
        if(passcode_changed) {
          state = UNLOCK;
          passcode_changed = 0;
        } 
        else {
          state = CHECK;
        }
      }
      else if(ADC_read(0) >= 600){ //reads up
        state = U;  
      }
      else if(ADC_read(0) <= 450){ //reads down
        state = D;
      }
      else if(ADC_read(1) <= 450){ //reads left
        state = L;
      }
      else if(ADC_read(1) >= 600){ //reads right
        state = R;
      }
      break;

    case U:
      if(ADC_read(0) >= 600){
        state = U;
      }
      else{
        inputArray[input++] = 1;
        state = INIT;
      }
      break;

    case D:
      if(ADC_read(0) <= 450){
        state = D;
      }
      else{
        inputArray[input++] = 2;
        state = INIT;
      }
      break;

    case L:
      if(ADC_read(1) <= 450){
        state = L;
      }
      else{
        inputArray[input++] = 3;
        state = INIT;
      }
      break;

    case R:
      if(ADC_read(1) >= 600){
        state = R;
      }
      else{
        inputArray[input++] = 4;
        state = INIT;
      }
      break;

    case CHECK:
      int pass = 1;
      for(int i = 0; i < 4; i++){
        if(passcode[i] != inputArray[i]){
          pass = 0;
          break;
        }
      }
      if(read_again == 1 && pass == 1){
        input = 0;
        state = LOCK;
      }
      else if(read_again == 0 && pass == 1){
        input = 0;
        state = UNLOCK;
      }
      else{
        input = 0;
        state = INIT;
      }
      break;

    case UNLOCK:
      input = 0;
      read_again = 1;
      limit_Rota = 0;
      state = INIT;
      break;

    case LOCK:
      input = 0;
      read_again = 0;
      limit_Rota = 0;
      state = INIT;
      break;

    case NEW_PASS:
      for(input = 1; i < 5; i++){
        if(ADC_read(0) >= 600){ //reads up
          passcode[input] = 1;
          outDir(0);
        }
        else if(ADC_read(0) <= 450){ //reads down
          passcode[input] = 2;
          outDir(1);
        }
        else if(ADC_read(1) <= 450){ //reads left
          passcode[input] = 3;
          outDir(3);
        }
        else if(ADC_read(1) >= 600){ //reads right
          passcode[input] = 4;
          outDir(2);
        }
      }
        passcode_changed = 1;
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
      if(read_again == 1){
        PORTC = PORTC & ~(1 << 4);
      }
      else{
        PORTC = PORTC | (1 << 4);
      }
      outNum(input);
      break;

    case U:
      outDir(0);
      break;

    case D:
      outDir(1);
      break;

    case L:
      outDir(3);
      break;

    case R:
      outDir(2);
      break;

    case CHECK:
      PORTC = PORTC | (1 << 4);
      break;

    case UNLOCK:
      PORTC = PORTC & ~(1 << 4);
      PORTB = (PORTB & 0x03) | phases[i] << 2; // Write the current phase to PORTB
      i++; // Increment to the next phase
      if (i > 7) { // If all phases are completed, reset
        i = 0;
        if(limit_Rota++ >= 128){
          input = 0;
          read_again = 1;
          limit_Rota = 0; 
          state = INIT;
          break;
        }
      }
      break;

    case LOCK:
      PORTB = (PORTB & 0x03) | phases2[i] << 2; // Write the current phase to PORTB
      i++; // Increment to the next phase
      if (i > 7) { // If all phases are completed, reset
        i = 0;
        limit_Rota++; 
        if (limit_Rota >= 128) {
          input = 0;
          read_again = 0;
          limit_Rota = 0;
          state = INIT;
          break;
        }
      }
      PORTC = PORTC | (1 << 4);
      break;
    
    case NEW_PASS:
      PORTC = PORTC | (1 << 4);
      outNum(input);
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

  TimerSet(1); //period of 1 ms. good period for the stepper mottor
  TimerOn();

    while (1)
    {
		  Tick();      // Execute one synchSM tick
      while (!TimerFlag){}  // Wait for SM period
      TimerFlag = 0;        // Lower flag
     }

    return 0;
}
