#include "timerISR.h"
#include "helper.h"
#include "periph.h"

/* Your Name & E-mail: Sharon Lee ~ slee900@ucr.edu

   Discussion Section: 024
   
   Assignment: Lab #7  Exercise #2
   
   Exercise Description: 
    When the joystick is centered (idle), the stepper motor should not rotate.
        There should be NO movement of the stepper motor while the joystick is idle.
    If the joystick is moved up, the stepper motor rotates clockwise and its rotational velocity should be proportional to the displacement of the joystick in the upward direction. 
    If the joystick is moved down, the stepper motor rotates counterclockwise and its rotational velocity should be proportional to the displacement of the joystick in the downward direction
        Whenever the joystick is moved down, the buzzer should beep every 2 seconds. Each beep should last 1 second.
        Use a different buzzer tone for beeping than for the Honk. You can choose your own tone, but beeps and honks should be distinguishable in your video demos. 
    The maximum rotational velocity of the stepper motor should be 2 ms per phase.
    The minimum rotational velocity of the stepper motor should be 30 ms per phase.

   I acknowledge all content contained herein, excluding template or example code, 
   is my own original work.
*/

#define NUM_TASKS 4 //TODO: Change to the number of tasks being used

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
unsigned long t3_period = 100;
unsigned long t4_period = 1000;

const unsigned long GCD_PERIOD = findGCD(t1_period, t4_period); //TODO:Set the GCD Period

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
    TCCR0B = (TCCR0B & 0xF8) | 0x04;//set prescaler to 256
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

enum step_motor_states {read, up, down};
int tf_step(int state){
    unsigned char cnt, speed, i = 0;
    switch(state){
        case read:
            if(ADC_read(0) > 550){
                state = up;
            }
            if(ADC_read(0) < 500){
                state = down;
            }
            break;
        
        case up:
            if(ADC_read(0) > 550){
                state = up;
            }
            else{
                state = read;
            }
            break;

        case down:
            if(ADC_read(0) < 500){
                state = down;
            }
            else{
                state = read;
            }
            break;

        default:
            state = read;
            break;
    }

    switch(state){
        case read:
            break;
        
        case up:
            speed = 32 - map_value(550, 1023, 2, 30, ADC_read(0));
            cnt++;
            if(cnt >= speed){
                PORTB = ((PORTB & 0x03) | stages[i] << 2);
                i++;
                if(i > 7){
                    i = 0;
                }
                cnt = 0;
            }
            break;

        case down:
            speed = map_value(0, 500, 2, 30, ADC_read(0));
            cnt++;
            if(cnt >= speed){
                PORTB = ((PORTB & 0x03) | stages[i] << 2);
                i--;
                if(i < 0){
                    i = 7;
                }
                cnt = 0;
            }
            break;

        default:
            break;
    }
    return state;
}

enum backup_buzz {wait2, buzz_on, buzz_off};
int tf_backup(int state){
    static unsigned char i = 0;
    TCCR0B = (TCCR0B & 0xF8) | 0x03;//set prescaler to 64
    switch(state){
        case wait2:
            if(ADC_read(0) < 450){
                i = 0;
                state = buzz_on;
            }
            break;

        case buzz_on:
            ++i;
            if(ADC_read(0) < 450 && i <= 2){
                state = buzz_on;
            }
            if(ADC_read(0) < 450 && i > 2){
                i = 0;
                state = buzz_off;
            }
            else{
                state = wait;
            }
            break;

        case buzz_off:
            ++i;
            if(ADC_read(0) < 450 && i <= 1){
                state = buzz_off;
            }
            if(ADC_read(0) < 450 && i > 1){
                i = 0;
                state = buzz_on;
            }
            else{
                state = wait;
            }
            break;

        default:
            state = wait2;
            break;
    }

    switch(state){
        case wait2:
            OCR0A = 255;
            break;

        case buzz_on:
            OCR0A = 128;
            break;

        case buzz_off:
            OCR0A = 255;
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
    // TCCR0B = (TCCR0B & 0xF8) | 0x04;//set prescaler to 256
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

    tasks[2].period = t3_period;
    tasks[2].state = read;
    tasks[2].elapsedTime = tasks[2].period;
    tasks[2].TickFct = &tf_step;

    tasks[3].period = t4_period;
    tasks[3].state = wait2;
    tasks[3].elapsedTime = tasks[3].period;
    tasks[3].TickFct = &tf_backup;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}
