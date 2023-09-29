// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#include <xc.h>
#include <stdio.h>
#include "color.h"
#include "dc_motor.h"
#include "hardware.h"
#include "i2c.h"
#include "interrupts.h"
#include "sequence.h"
#include "structures.h"
#include "timers.h"

void main(void){
    color_click_init();   // initialise the color click board
    hardware_init();      // initialise all other hardware
    I2C_2_Master_Init();  // initialise I2C functionality
    Timer0_init();        // initialise timer0 hardware
    Interrupts_init();    // initialisation of interrupts
    initDCmotorsPWM(99);  // initialise DC motor control
    
    DATA data_struct;                  // declare the data structure to store all information
    SEQUENCE sequence;                 // declare the sequence structure
    data_struct.sequence = &sequence;  // assign the data structure pointer to the sequence structure

    data_struct.sequence->index = 0;   // declare move index zero
    data_struct.backtrack = 0;         // declare backtrack state zero
    data_struct.count = 0;             // declare count state zero
         
    while (1){       
        // main loop for navigating the maze
        if (!BUTTON_RF2) {
            // main navigation loop to find the white wall
            while (data_struct.backtrack == 0) {
                move2wall(&data_struct);
                __delay_ms(1000);
                colorAction(&data_struct);
            }
            
            // backtrack to the start of the maze
            backtrack(&data_struct);
        }
        
        // calibration loop
        if (!BUTTON_RF3) {storeCalibration(&data_struct);}
    }
}
        
       