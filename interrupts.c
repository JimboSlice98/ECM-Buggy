#include <xc.h>
#include "interrupts.h"

/************************************
 * Function to turn on interrupts and set if priority is used
************************************/
void Interrupts_init(void) {    
    PIE0bits.TMR0IE = 1;  // enable timer overflow interrupt source
    INTCONbits.PEIE = 1;  // turn on peripheral interrupts
    INTCONbits.GIE = 1;   // turn on interrupts globally - KEEP LAST
}

/************************************
 * High priority interrupt service routine
************************************/
void __interrupt(high_priority) HighISR() {
    // timer interrupt flag
    if (PIR0bits.TMR0IF) {                  // check the timer interrupt source
        LATHbits.LATH3 = !LATHbits.LATH3;   // toggle LED
        TMR0H = 0;                          // reset the timer
        TMR0L = 0;
        PIR0bits.TMR0IF = 0;                // clear the interupt flag
    }
}
