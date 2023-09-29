#ifndef _dc_motor_H
#define _dc_motor_H

#include <xc.h>
#include "color.h"
#include "sequence.h"
#include "structures.h"

#define _XTAL_FREQ 64000000 // note intrinsic _delay function is 62.5ns at 64,000,000Hz  

DC_MOTOR motorL, motorR;    // declare two DC_motor structures

void initDCmotorsPWM(unsigned char PWMperiod);
void setMotorPWM(DC_MOTOR *m);
void stop(void);
void straight(unsigned char direction, unsigned char power);
void rotate(unsigned char direction, unsigned char angle);
void increasePower(unsigned char power);
void move2wall(DATA *data);
void colorAction(DATA *data);

#endif
