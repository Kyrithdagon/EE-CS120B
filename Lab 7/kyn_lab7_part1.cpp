#include "timerISR.h"
#include "helper.h"
#include "periph.h"

/* Your Name & E-mail: ...

   Discussion Section: 024
   
   Assignment: Lab #7  Exercise #1
   
   Exercise Description: 
    Create an Indicator with 2 buttons + 6 LEDs.
    Use 1 button for 3 LEDs and the other button for the remaining 3 LEDs. Replicate the indicator pattern as seen on this Ford MustangLinks to an external site..
    Only one Indicator can be active at a time.
    Implement the horn using the Joystick button and a passive buzzer (using PWM).
        While the joystick’s button is pressed, the passive buzzer sounds a tone.
        You may select any timbre (frequency) you see fit.

   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

#define NUM_TASKS 2 //TODO: Change to the number of tasks being used

// Task struct for concurrent synchSMs implementations
typedef struct _task {
    signed char state;          // Task's current state
    unsigned long period;       // Task period
    unsigned long elapsedTime;  // Time elapsed since last task tick
    int (*TickFct)(int);        // Task tick function
} task;

//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
unsigned long t1_period = 400;
unsigned long t2_period = 300;

const unsigned long GCD_PERIOD = findGCD(t1_period, t2_period); //TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 task

void TimerISR() {
    for (unsigned int i = 0; i < NUM_TASKS; i++) {                // Iterate through each task in the task array
        if (tasks[i].elapsedTime >= tasks[i].period) {            // Check if the task is ready to tick
            tasks[i].state = tasks[i].TickFct(tasks[i].state);    // Tick and set the next state for this task
            tasks[i].elapsedTime = 0;                             // Reset the elapsed time for the next tick
        }
        tasks[i].elapsedTime += GCD_PERIOD;                       // Increment the elapsed time by GCD_PERIOD
    }
}

int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //Stepper motor phases

//TODO: Create your tick functions for each task
enum button_light_states {buttons, left, left_1, left_2, left_3, right, right_1, right_2, right_3};
int tf_button(int state) {
    switch(state) {
        case buttons:
            if (GetBit(PINC, 3) && !GetBit(PINC, 4)) {
                state = left_1;
            }
            if (GetBit(PINC, 4) && !GetBit(PINC, 3)) {
                state = right_1;
            }
            break;

        case left_1:
            if (GetBit(PINC, 3)) {
                state = left_2;
            } else {
                state = buttons;
            }
            break;

        case left_2:
            if (GetBit(PINC, 3)) {
                state = left_3;
            } else {
                state = buttons;
            }
            break;

        case left_3:
            if (GetBit(PINC, 3)) {
                state = left;
            }
            else{
                state = buttons;
            }

        case left:      
            state = buttons;
            break;

        case right_1:
            if (GetBit(PINC, 4)) {
                state = right_2;
            } else {
                state = buttons;
            }
            break;

        case right_2:
            if (GetBit(PINC, 4)) {
                state = right_3;
            } else {
                state = buttons;
            }
            break;

        case right_3:
            if (GetBit(PINC, 4)) {
                state = right;
            }
            else{
                state = buttons;
            }

        case right:      
            state = buttons;
            break;    

        default:
            state = buttons;
            break;
    }
    
    switch(state) {
        case buttons:
            PORTB = 0x00; 
            PORTD = 0x00; 
            break;

        case left_1:
            PORTB = 0x01; //1st
            PORTD = 0x00;
            break;

        case left_2:
            PORTB = 0x01; //1st
            PORTD = 0x80; //2nd
            break;

        case left_3:
            PORTB = 0x01; //1st
            PORTD = 0xA0; //2nd & 3rd
            break;

        case right_1:
            PORTD = 0x10; //1st
            break;

        case right_2:
            PORTD = 0x18; //1st & 2nd
            break;

        case right_3:
            PORTD = 0x1C; //1st, 2nd & 3rd
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
            if(!GetBit(PINC, 2)){
                state = buzz;
            }
            break;

        case buzz:
            if(!GetBit(PINC, 2)){
                state = buzz;
            }
            else{
                state = wait;
            }
            break;
        
        default:
            state = wait;
            break;
    }

    switch(state){
        case wait:
            OCR0A = 255; 
            break;

        case buzz:
            OCR0A = 128; 
            break;

        default:
            break;
    }
    return state;
}

int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x00; PORTC = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;

    ADC_init(); // initializes ADC

    //TODO: Initialize the buzzer timer/pwm(timer0)
    //OCR0A = 128; //sets duty cycle to 50% since TOP is always 256
    TCCR0A |= (1 << COM0A1);// use Channel A
    TCCR0A |= (1 << WGM01) | (1 << WGM00);// set fast PWM Mode
    // TCCR0B = (TCCR0B & 0xF8) | 0x02; //set prescaler to 8
    // TCCR0B = (TCCR0B & 0xF8) | 0x03;//set prescaler to 64
    TCCR0B = (TCCR0B & 0xF8) | 0x04;//set prescaler to 256
    // TCCR0B = (TCCR0B & 0xF8) | 0x05;//set prescaler to 1024

    //TODO: Initialize the servo timer/pwm(timer1)
    //..

    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;
    tasks[0].period = t1_period;
    tasks[0].state = buttons;
    tasks[0].elapsedTime = tasks[0].period;
    tasks[0].TickFct = &tf_button;

    tasks[1].period = t2_period;
    tasks[1].state = wait;
    tasks[1].elapsedTime = tasks[1].period;
    tasks[1].TickFct = &tf_buzzer;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}
