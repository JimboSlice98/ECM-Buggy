#include <xc.h>
#include "color.h"
#include "dc_motor.h"
#include "hardware.h"
#include "i2c.h"
#include "interrupts.h"
#include "sequence.h"
#include "structures.h"
#include "timers.h"

/************************************************
 *  Function to initialise T2 and CCP for DC motor control
 ***********************************************/
void initDCmotorsPWM(unsigned char PWMperiod) {
    // initialise your TRIS and LAT registers for PWM
    LATEbits.LATE2=0;       // set initial output state (RE2)
    TRISEbits.TRISE2=0;     // set TRIS value for pin (RE2)
    LATEbits.LATE4=0;       // set initial output state (RE4)
    TRISEbits.TRISE4=0;     // set TRIS value for pin (RE4)
    LATCbits.LATC7=0;       // set initial output state (RC7)
    TRISCbits.TRISC7=0;     // set TRIS value for pin (RC7)
    LATGbits.LATG6=0;       // set initial output state (RG6)
    TRISGbits.TRISG6=0;     // set TRIS value for pin (RG6)
    
    // configure PPS to map CCP modules to pins
    RE2PPS=0x05; // CCP1 on RE2
    RE4PPS=0x06; // CCP2 on RE4
    RC7PPS=0x07; // CCP3 on RC7
    RG6PPS=0x08; // CCP4 on RG6

    // timer 2 config
    T2CONbits.CKPS=0b100;   // 1:16 prescaler
    T2HLTbits.MODE=0b00000; // Free Running Mode, software gate only
    T2CLKCONbits.CS=0b0001; // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    // 0.0001s*16MHz/16 -1 = 99
    T2PR=PWMperiod; // Period reg 10kHz base period
    T2CONbits.ON=1;
    
    // setup CCP modules to output PMW signals
    // initial duty cycles 
    CCPR1H=0; 
    CCPR2H=0; 
    CCPR3H=0; 
    CCPR4H=0; 
    
    // use tmr2 for all CCP modules used
    CCPTMRS0bits.C1TSEL=0;
    CCPTMRS0bits.C2TSEL=0;
    CCPTMRS0bits.C3TSEL=0;
    CCPTMRS0bits.C4TSEL=0;
    
    // configure each CCP
    CCP1CONbits.FMT=1;           // left aligned duty cycle (we can just use high byte)
    CCP1CONbits.CCP1MODE=0b1100; // PWM mode  
    CCP1CONbits.EN=1;            // turn on
    
    CCP2CONbits.FMT=1;           // left aligned
    CCP2CONbits.CCP2MODE=0b1100; // PWM mode  
    CCP2CONbits.EN=1;            // turn on
    
    CCP3CONbits.FMT=1;           // left aligned
    CCP3CONbits.CCP3MODE=0b1100; // PWM mode  
    CCP3CONbits.EN=1;            // turn on
    
    CCP4CONbits.FMT=1;           // left aligned
    CCP4CONbits.CCP4MODE=0b1100; // PWM mode  
    CCP4CONbits.EN=1;            //turn on
    
    // initialise left motor values
    motorL.power=0;                                     // zero power to start
    motorL.direction=1;                                 // set default motor direction
    motorL.brakemode=1;                                 // brake mode (slow decay)
    motorL.posDutyHighByte=(unsigned char *)(&CCPR1H);  // store address of CCP1 duty high byte
    motorL.negDutyHighByte=(unsigned char *)(&CCPR2H);  // store address of CCP2 duty high byte
    motorL.PWMperiod=PWMperiod;                         // store PWMperiod for motor (value of T2PR in this case)
    
    // initialise right motor values
    motorR.power=0;                                     // zero power to start
    motorR.direction=1;                                 // set default motor direction
    motorR.brakemode=1;                                 // brake mode (slow decay)
    motorR.posDutyHighByte=(unsigned char *)(&CCPR3H);  // store address of CCP1 duty high byte
    motorR.negDutyHighByte=(unsigned char *)(&CCPR4H);  // store address of CCP2 duty high byte
    motorR.PWMperiod=PWMperiod;                         // store PWMperiod for motor (value of T2PR in this case)
}

/************************************************
 *  Function to set CCP PWM output from the values in the motor structure
 ***********************************************/
void setMotorPWM(DC_MOTOR *m) {
    unsigned char posDuty, negDuty; // duty cycle values for different sides of the motor
    
    if(m->brakemode) {
        posDuty=m->PWMperiod - ((unsigned int)(m->power)*(m->PWMperiod))/100; // inverted PWM duty
        negDuty=m->PWMperiod; // other side of motor is high all the time
    }
    else {
        posDuty=((unsigned int)(m->power)*(m->PWMperiod))/100; // PWM duty
        negDuty=0; // other side of motor is low all the time
    }
    
    if (m->direction) {
        *(m->posDutyHighByte)=posDuty;  // assign values to the CCP duty cycle registers
        *(m->negDutyHighByte)=negDuty;       
    } else {
        *(m->posDutyHighByte)=negDuty;  // do it the other way around to change direction
        *(m->negDutyHighByte)=posDuty;
    }
}

/************************************************
 *  Function to stop the robot gradually
 ***********************************************/
void stop(void) {
    // decrease the left and right motor power until it is zero
    while (motorL.power > 0 || motorR.power > 0) {
        if (motorL.power > 0) { motorL.power--; }
        if (motorR.power > 0) { motorR.power--; }
        
        setMotorPWM(&motorL);
        setMotorPWM(&motorR);
        __delay_us(50);
    }
}

/************************************************
 *  Function to make the robot go straight
 *  Direction: backward -> 0; forward -> 1
 ***********************************************/
void straight(unsigned char direction, unsigned char power) {
    // assign direction to each motor
    motorL.direction = direction;
    motorR.direction = direction;
    
    // set motor PWM for direction change
    setMotorPWM(&motorL);
    setMotorPWM(&motorR);
    
    // gradually increase the power of the motors
    increasePower(power);
}

/************************************************
 *  Function to rotate the buggy
 *  Direction: left -> 0; right -> 1
 ***********************************************/
void rotate(unsigned char direction, unsigned char angle) {
    // assign direction to each motor
    motorL.direction = direction;
    motorR.direction = !direction;
    
    // set motor PWM for direction change
    setMotorPWM(&motorL);
    setMotorPWM(&motorR);
    
    // buggy will rotate in 45 degree increments to account for 45, 90, 135 and 180 deg turns
    for (unsigned char i = 0; i < angle/45; i++) {
        increasePower(100);  // high power has more accuracy
        __delay_ms(75);
        stop();
        __delay_ms(250);     // delay between multiple 45 deg turns
    }
}

/************************************************
 *  Function to increase the motor power to the required level
 ***********************************************/
void increasePower(unsigned char power) {
    while (motorL.power < power || motorR.power < power) {
        // increase both motorL and motorR power to desired power
        if (motorL.power < power) { motorL.power++; }
        if (motorR.power < power) { motorR.power++; }

        // set motor PWM to account for power change
        setMotorPWM(&motorL);
        setMotorPWM(&motorR);
        __delay_us(100);
    }
}

/************************************************
 *  Function to move the buggy in a straight line a stop before hitting a wall
 ***********************************************/
void move2wall(DATA *data) {
    // complete calibration of ambient light
    BRAKE_LED = 1;
    LED_on();
    __delay_ms(500);
    storeAmbient(data);
    __delay_ms(500);
    BRAKE_LED = 0;
    
    // reset timer and start moving forward whilst searching for a wall
    resetTimer();               
    while (1) {
        straight(1, 20);
        storeColor(data);

        // stop the buggy if the clear channel exits the threshold
        if (data->hsv.c < data->ambLight - 13 || data->hsv.c > data->ambLight + 30) {
            stop();
            
            // do not store the movement if the color was not previously detected
            if (data->count == 0) {
                addMove(data, 0, 1, 20, get16bitTMR0val() + 400);   // add movement towards the wall in the forward sequence
            };
            
            break;
        }  
    }
}

/************************************************
 *  Function to initialise the colour click module using I2C
 ***********************************************/
void colorAction(DATA *data) {
    // drive into the wall to align buggy
    straight(1, 40);
    __delay_ms(500);
    stop();
    
    // store the color of the wall
    __delay_ms(500);
    char decision = detectColor(data);
    __delay_ms(500);
    
    // move backwards away from the wall
    straight(0, 20);
    __delay_ms(700);
    stop();
    
    // do not store the movement if the color was not previously detected
    if (data->count == 0) {addMove(data, 0, 0, 40, 600);}
    __delay_ms(1000);
    
    // complete action based on color of wall and addMove according to action taken
    switch (decision) {
        case 0:  // red -> turn right 90 deg
            rotate(1, 90);
            addMove(data, 1, 1, 90, 0);
            break;
            
        case 1:  // green -> turn left 90 deg
            rotate(0, 90);
            addMove(data, 1, 0, 90, 0);
            break;
            
        case 2:  // blue -> turn 180 deg
            rotate(0, 180);
            addMove(data, 1, 0, 180, 0);
            break;
            
        case 3:  // yellow -> reverse 1 square and turn right 90 deg
            straight(0, 20);
            __delay_ms(2500);
            stop();
            addMove(data, 0, 0, 20, 2500);
            __delay_ms(500);
            rotate(1, 90);
            addMove(data, 1, 1, 90, 0);
            break;
            
        case 4:  // pink -> reverse 1 square and turn left 90 deg
            straight(0, 20);
            __delay_ms(2500);
            stop();
            addMove(data, 0, 0, 20, 2500);
            __delay_ms(500);
            rotate(0, 90);
            addMove(data, 1, 0, 90, 0);
            break;
            
        case 5:  // orange -> turn right 135 deg
            rotate(1, 135);
            addMove(data, 1, 1, 135, 0);
            break;
            
        case 6:  // light blue -> turn left 135 deg
            rotate(0, 135);
            addMove(data, 1, 0, 135, 0);
            break;
        
        case 7:  // white -> finish, 'trigger return to home'
            data->backtrack = 1;        // update backtrack flag to return to starting position
            break;
            
        case 8:  // no color found (black)
            data->count++;
            if (data->count >= 3) {     // check if the wall has been detected to be black 3 times
                data->backtrack = 1;    // update backtrack flag to return to starting position
            }
            break;
    }
    
    // reset the counter if a color was found
    if (decision != 8) {data->count = 0;}
}