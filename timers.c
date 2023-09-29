#include <xc.h>
#include "timers.h"

/************************************
 * Function to set up timer 0
************************************/
void Timer0_init(void) {
    T0CON1bits.T0CS = 0b010;      // Fosc/4
    T0CON1bits.T0ASYNC = 1;       // see datasheet errata - needed to ensure correct operation when Fosc/4 used as clock source
    T0CON1bits.T0CKPS = 0b1110;   // 1:2^14 PS; Overflow time = 67.1s; T_int = 1.024ms
    T0CON0bits.T016BIT = 1;       // 16bit mode
       
    TMR0H = 0;                    //write High reg first, update happens when low reg is written to
    TMR0L = 0; 
    T0CON0bits.T0EN = 1;	      //start the timer
}

/************************************
 * Function to reset the timer
************************************/
void resetTimer(void) {
	TMR0H = 0;
    TMR0L = 0; 
}

/************************************
 * Function to return the full 16bit timer value
 * Note TMR0L and TMR0H must be read in the correct order, or TMR0H will not contain the correct value
************************************/
unsigned int get16bitTMR0val(void) {
	// must ensure each register is read as an unsigned int for correct functionality
    unsigned int timer_l = TMR0L;
    unsigned int timer_h = TMR0H;
    
    return timer_l | timer_h << 8; // TMR0L for bits 0 - 7 and TMR0H for bits 8-15
}

