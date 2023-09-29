#ifndef _structures_H
#define _structures_H

#include <xc.h>

#define _XTAL_FREQ 64000000

typedef struct RGB {          // definition of RGB structure
    unsigned int r;           // read value
    unsigned int g;           // green value
    unsigned int b;           // blue value
    unsigned int c;           // clear value
} RGB;

typedef struct HSV {          // definition of HSV structure
    unsigned int h;           // hue value
    unsigned int s;           // saturation value
    unsigned int v;           // value value
    unsigned int c;           // clear value
} HSV;

typedef struct MOVE {         // definition of MOVE structure
    unsigned char type;       // 0/1: straight/rotate
    unsigned char direction;  // 0/1: backward/forward | 0/1: left/right 
    unsigned char power;      // power of straight | angle of rotate
    unsigned int time;        // time taken for move
} MOVE;

typedef struct SEQUENCE {     // definition of SEQUENCE structure
    unsigned int index;       // counter of number of moves remembered
    MOVE moves[50];           // array of MOVE structures remembered
} SEQUENCE;

typedef struct DATA {         // definition of overall DATA structure
    HSV cal[9];               // nested structure to store calibration data
    HSV hsv;                  // nested structure to store instantaneous color
    unsigned int ambLight;    // integer to store clear channel data for wall detection
    unsigned char backtrack;  // variable to store if the backtrack functionality is to be executed
    unsigned char count;      // variable to count the number of failed color detections
    SEQUENCE *sequence;       // nested structure to store the sequence of moves
} DATA;

typedef struct DC_motor {           // definition of DC_motor structure
    char power;                     // motor power, out of 100
    char direction;                 // motor direction, forward(1), reverse(0)
    char brakemode;		            // short or fast decay (brake or coast)
    unsigned int PWMperiod;         // base period of PWM cycle
    unsigned char *posDutyHighByte; // PWM duty address for motor +ve side
    unsigned char *negDutyHighByte; // PWM duty address for motor -ve side
} DC_MOTOR;

#endif
