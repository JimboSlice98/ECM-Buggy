#ifndef _hardware_H
#define _hardware_H

#include <xc.h>

#define _XTAL_FREQ 64000000         // note intrinsic _delay function is 62.5ns at 64,000,000Hz  
#define RED_LED LATGbits.LATG1      // set name for red LED in array
#define GREEN_LED LATAbits.LATA4    // set name for green LED in array
#define BLUE_LED LATFbits.LATF7     // set name for blue LED in array
#define HEAD_LAMPS LATHbits.LATH1   // set name for head lamps on buggy
#define MAIN_BEAM LATDbits.LATD3    // set name for main beam on buggy
#define INDICATOR_R LATHbits.LATH0  // set name for right indicator on buggy
#define INDICATOR_L LATFbits.LATF0  // set name for left indicator on buggy
#define BRAKE_LED LATDbits.LATD4    // set name for brake lights on buggy
#define TIMER_LED LATHbits.LATH3    // set name for timer LED
#define BUTTON_RF2 PORTFbits.RF2    // set name for RF2 button
#define BUTTON_RF3 PORTFbits.RF3    // set name for RF3 button

void hardware_init(void);
void LED_on(void);
void LED_off(void);
void LED_flash(unsigned char num);

#endif
