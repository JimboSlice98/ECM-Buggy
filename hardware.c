#include <xc.h>
#include "hardware.h"

/************************************************
 *  Function to initialise all hardware bits
 ***********************************************/
void hardware_init(void) {      
    // set LAT values of hardware
    RED_LED = 0;
    GREEN_LED = 0;
    BLUE_LED = 0;
    
    HEAD_LAMPS = 0;
    MAIN_BEAM = 0;
    INDICATOR_R = 0;
    INDICATOR_L = 0;
    BRAKE_LED = 0;
    TIMER_LED = 0; 
    
    // set TRIS value of hardware
    TRISGbits.TRISG1 = 0;  // red LED in array
    TRISAbits.TRISA4 = 0;  // green LED in array
    TRISFbits.TRISF7 = 0;  // blue LED in array
    
    TRISHbits.TRISH1 = 0;  // head lamps on buggy
    TRISDbits.TRISD3 = 0;  // main beam on buggy
    TRISHbits.TRISH0 = 0;  // right indicator on buggy
    TRISFbits.TRISF0 = 0;  // left indicator on buggy
    TRISDbits.TRISD4 = 0;  // brake lights on buggy
    
    TRISFbits.TRISF2 = 1;  // RF2 button
    TRISFbits.TRISF3 = 1;  // RF3 button
    
    TRISHbits.TRISH3 = 0;  // RH3 LED

    // set ANSEL values of hardware
    ANSELFbits.ANSELF2 = 0;  // RF2 button
    ANSELFbits.ANSELF3 = 0;  // RF3 button
}

/************************************************
 *  Function to enable all LEDs in array
 ***********************************************/
void LED_on(void) {
    RED_LED = 1;
    GREEN_LED = 1;
    BLUE_LED = 1;
}

/************************************************
 *  Function to disable all LEDs in array
 ***********************************************/
void LED_off(void) {
    RED_LED = 0;
    GREEN_LED = 0;
    BLUE_LED = 0;
}

/************************************************
 *  Function to flash the indicators n number of times
 ***********************************************/
void LED_flash(unsigned char num) {
    unsigned char i = 0;
    while (i < num) {
        INDICATOR_L = 1;
        INDICATOR_R = 1;
        __delay_ms(250);    
        INDICATOR_L = 0;
        INDICATOR_R = 0;
        __delay_ms(250);
        i++;
    }
}

