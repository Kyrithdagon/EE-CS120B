#include "timerISR.h"
#include "helper.h"
#include "periph.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #5  Exercise #2
   
   Exercise Description: 
    We will add the RGB LED for this exercise, as well as two additional tasks. You will implement your own Pulse Width Modulation (PWM) signal generator in two separate Tasks. The two tasks below vary the color illuminated by the RGB LED in response to the distance measurement reported by the ultrasonic sensor in Task 1, above. 
    Task 3: Red - Task Period: 1 ms; PWM period: 10 ms
      Set the duty cycle to 100% if the distance measured is less than 10 cm
      Set the duty cycle to 90% if the distance measured is between 10 and 20 cm
      Set the duty cycle to 0% if the distance measured is greater than 20 cm
    Task 4: Green - Period: 1 ms; PWM period: 10 ms
      Set the duty cycle to 0% if the distance measured is less than 10 cm.
      Set the duty cycle to 30% if the distance measured is between 10 and 20 cm
      Set the duty cycle to 100% if the distance measured is greater than 20 cm
    Collectively, you should observe the following colors on the RGB LED:
      RED if the distance measured is less than 10 cm
      YELLOW if the distance measured is between 10 and 20 cm
      GREEN if the distance measured is greater than 20 cm

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
//const unsigned long T5_PERIOD = 200;

const unsigned long GCD_PERIOD = findGCD(T2_PERIOD, T3_PERIOD);

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

enum Red_PMW_States {red_dc};
int TickFct_Red(int state){
    unsigned char redi = 0;
    switch(state){
        case red_dc:
            if(measure_Dis < 10){
                PORTC = PORTC | (1 << 5);
            }
            else if(measure_Dis >= 10 && measure_Dis <= 20){
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
            redi = (redi++) % 10;
            return red_dc;
    }
}

enum Green_PMW_States {green_dc};
int TickFct_Green(int state){
    unsigned char greeni = 0;
    switch(state){
        case green_dc:
            if(measure_Dis < 10){
                PORTC = PORTC | (1 << 4);
            }
            else if(measure_Dis >= 10 && measure_Dis <= 20){
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
            greeni = (greeni++) % 10;
            return green_dc;
    }
}

void TimerISR() {
    //TODO: sample inputs here
	for (unsigned int i = 0; i < NUM_TASKS; i++) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
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

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}
