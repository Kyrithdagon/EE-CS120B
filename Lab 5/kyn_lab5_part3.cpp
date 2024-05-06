#include "timerISR.h"
#include "helper.h"
#include "periph.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #5  Exercise #3
   
   Exercise Description: 
    This exercise introduces the joystick and displays multiple numbers using the 4D7S display. 
    Task 5: Joystick - Period: 200ms
      When the joystick button is pressed, toggle the 4D7S display to show the distance in cm and inches.
          The RGB LED should still report the colors based on cm values (i.e., no changes to Tasks 3 and 4).
          Only the 4D7S display changes in response to the button being pressed. 
      Moving the joystick UP/DOWN adjusts the thresholds for changing the colors on the RGB LED.
          Close (5 cm, 10 cm)
          Moderate (10 cm, 20 cm) -- This is the default threshold when the system is initialized(The thresholds used for exercise 2)
          Far (15 cm, 30 cm)
          The default threshold is Moderate 
      Moving the joystick UP increases the threshold: Close → Moderate, Moderate → Far, Far → Far
      Moving the joystick DOWN decreases the threshold Far → Moderate, Moderate → Close, Close → Close

   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

//TODO: declare variables for cross-task communication
unsigned short measure_Dis = 0;

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed char state; 		    //Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long T1_PERIOD = 1000;
const unsigned long T2_PERIOD = 1;
const unsigned long T3_PERIOD = 1;  //PMW period is 10 ms
const unsigned long T4_PERIOD = 1;  //PMW period is 10 ms
const unsigned long T5_PERIOD = 200;

const unsigned long GCD_PERIOD = findGCD(T2_PERIOD, T5_PERIOD);

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions
enum Sonar_States {detect};
int TickFct_Sonar(int state){
    switch(state){
        case detect:
            measure_Dis = read_sonar();
            return detect;
    }
}

enum Display_States {display};
int TickFct_Display(int state){
    switch(state) { 
        case display:
            PORTB = 0b11111101; //pin for D1
            outNum(measure_Dis % 10); //gets remainder

            PORTB = 0b11111011; //pin for D2
            outNum((measure_Dis / 10) % 10); //gets second digit if there is and gets "remainder"

            PORTB = 0b11110111; //pin for D3
            outNum((measure_Dis / 100) % 10); //same here but for third digit display

            PORTB = 0b11001111; //pin for D4
            outNum((measure_Dis / 1000) % 10); //same here but for fourth digit display
            return display;
    }
}

unsigned char up_range[] = {10, 20, 30}; //changed this area to an array so now that the range can be adjusted based on the joystick control
unsigned char down_range[] = {5, 10, 15};
unsigned char i = 1; // to iterate through arrays above

enum Red_PMW_States {red_dc};
int TickFct_Red(int state){
    unsigned char redi = 0;
    switch(state){
        case red_dc:
            if(measure_Dis < down_range[i]){
                PORTC = PORTC | (1 << 5);
            }
            else if(measure_Dis >= down_range[i] && measure_Dis <= up_range[i]){
                if(redi < 10){
                    PORTC = PORTC | (1 << 5); 
                }
                else{
                    PORTC = PORTC & (1 << 4);
                }
            }
            else{
                PORTC = PORTC & (1 << 5);
            }
            redi = (redi + 1) % 10;
            return red_dc;
    }
}

enum Green_PMW_States {green_dc};
int TickFct_Green(int state){
    unsigned char greeni = 0;
    switch(state){
        case green_dc:
            if(measure_Dis < down_range[i]){
                PORTC = PORTC | (1 << 4);
            }
            else if(measure_Dis >= down_range[i] && measure_Dis <= up_range[i]){
                if(greeni < 3){
                    PORTC = PORTC | (1 << 4);
                }
                else{
                    PORTC = PORTC & (1 << 5);
                }
            }
            else{
                PORTC = PORTC & (1 << 4);
            }
            greeni = (greeni + 1) % 10;
            return green_dc;
    }
}

enum Joystick_States {joystick_Read, joystick_Button, joystick_UpDown};
int TickFct_Joystick(int state){
    unsigned char readIn;
    unsigned char checkPress = 1;
    switch(state){
        case joystick_Read:
            if(!GetBit(PINC, 1)){
                return joystick_Button;
            }
            if(ADC_read(0) > 600 && GetBit(PINC, 1)){
                readIn = 1;
                return joystick_UpDown;
            }
            if(ADC_read(0) < 400 && GetBit(PINC, 1)){
                readIn = 2;
                return joystick_UpDown;
            }
            break;

        case joystick_Button:
            state = joystick_Read;
            break;

        case joystick_UpDown:
            state = joystick_Read;
            break;

        default:
            state = joystick_Read;
            break;
    }

    switch(state){
        case joystick_Read:
            break;

        case joystick_Button:
            if(checkPress == 1){
                measure_Dis = measure_Dis * 0.394;
                checkPress = 2;
                return joystick_Read;
            }
            if(checkPress == 2){
                measure_Dis = measure_Dis * 2.54;
                checkPress = 1;
                return joystick_Read;
            }
            break;

        case joystick_UpDown:
            if(readIn == 1){
                i++;
                return joystick_Read;
            }
            if(readIn == 2){
                i--;
                return joystick_Read;
            }
            break;
        
        default:
            break;
    }
}

void TimerISR() {
    //TODO: sample inputs here
	for (unsigned int i = 0; i < NUM_TASKS; i++) {             // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {       // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                    // Increment the elapsed time by GCD_PERIOD
	}
}

int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRB = 0b00111110; PORTB = 0b11000001;
    DDRC = 0b00111100; PORTC = 0b11000011;
    DDRD = 0xFF; PORTD = 0x00;

    ADC_init();   // initializes ADC
    init_sonar(); // initializes sonar

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].timeElapsed = ...
    // tasks[0].TickFct = &task1_tick_function;
    tasks[0].period = T1_PERIOD;
    tasks[0].state = detect;
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &TickFct_Sonar;

    tasks[1].period = T2_PERIOD;
    tasks[1].state = display;
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &TickFct_Display;

    tasks[2].period = T3_PERIOD;
    tasks[2].state = red_dc;
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &TickFct_Red;

    tasks[3].period = T4_PERIOD;
    tasks[3].state = green_dc;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &TickFct_Green;

    tasks[4].period = T5_PERIOD;
    tasks[4].state = joystick_Read;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &TickFct_Joystick;

    TimerSet(1);
    TimerOn();

    while (1) {}

    return 0;
}
