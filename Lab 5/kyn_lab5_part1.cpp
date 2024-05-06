#include "timerISR.h"
#include "helper.h"
#include "periph.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #5  Exercise #1
   
   Exercise Description: 
    Task 1: Sonar - SM Period: 1000 ms:
        Use the ultrasonic sensor to measure the distance in cm.
        For the purpose of the demo, you can place an object of your choice in front of the sensor at an appropriate distance.
    Task 2: Display - Period: 1 ms
        Output the distance obtained from the ultrasonic sensor onto the 4D7S display.

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
// const unsigned long T3_PERIOD = 10;
// const unsigned long T4_PERIOD = 10;
// const unsigned long T5_PERIOD = 200;

const unsigned long GCD_PERIOD = findGCD(T1_PERIOD, T2_PERIOD);

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

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}
