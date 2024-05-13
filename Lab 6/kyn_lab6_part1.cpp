
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #6  Exercise #1
   
   Exercise Description: 
    On start up, display “Mode: MANUAL” on the first row of the LCD Screen.
    When the joystick's button is pressed, display "Mode: AUTO" on the first row of the LCD Screen. 
    Each subsequence press of the joystick switches between MANUAL and AUTO mode. 
    Whenever the user moves the joystick up, down, or presses the joystick button, sound a tone for 10ms on the active buzzer. 

   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

//TODO: declare variables for cross-task communication

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

//TODO: Define Periods for each task
// e.g. const unsigned long TASK1_PERIOD = <PERIOD>
unsigned long T1_Period = 1;
unsigned long T2_Period = 100;

const unsigned long GCD_PERIOD = findGCD(T1_Period, T2_Period);

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions
enum start_up_states {start, manualmode, automode};
int tf_start(int state){
    switch(state){
        case start:
            if (!GetBit(PINC, 2)) {
                state = automode;
            }
            break;

        case manualmode:
            if(!GetBit(PINC, 2)){
                state = automode;
            }
            else {
                state = manualmode;
            }
            break;

        case automode:
            if(!GetBit(PINC, 2)){
                state = manualmode;
            }
            else {
                state = automode;
            }
            break;
        
        default:
            state = start;
            break;
    }

    switch(state){
        case start:
            lcd_write_str("Mode: MANUAL");
            break;

        case manualmode:
            lcd_clear();
            lcd_write_str("Mode: MANUAL");
            break;
        
        case automode:
            lcd_clear();
            lcd_write_str("Mode: AUTO");
            break;

        default:
            break;
    }
    return state;
}

enum buzzer_states {wait, buzz};
int tf_buzzer(int state){
    switch(state){
        case wait:
            if(ADC_read(1) > 600 || ADC_read(1) < 400 || !GetBit(PINC, 2)){
                state = buzz;
            }
            else{
                state = wait;
            }
            break;

        case buzz:
            state = wait;
            break;

        default:
            state = wait;
            break;
    }

    switch(state){
        case wait:
            PORTB = SetBit(PORTB, 0, 0); //buzzer is off
            break;
        
        case buzz:
            PORTB = SetBit(PORTB, 0, 1); //buzzer is on for a short bit
            break;
        
        default:
            break;
    }
    return state;
}

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {           // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {       // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                    // Increment the elapsed time by GCD_PERIOD
	}
}

int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x00; PORTC = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;

    ADC_init();   // initializes ADC
    init_sonar(); // initializes sonar
    lcd_init();   // initalizes lcd

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].timeElapsed = ...
    // tasks[0].TickFct = &task1_tick_function;
    tasks[0].period = T1_Period;
    tasks[0].state = start;
    tasks[0].elapsedTime = tasks[0].period;
    tasks[0].TickFct = &tf_start;

    tasks[1].period = T2_Period;
    tasks[1].state = wait;
    tasks[1].elapsedTime = tasks[1].period;
    tasks[1].TickFct = &tf_buzzer;

    TimerSet(100);
    TimerOn();

    while (1) {}

    return 0;
}
