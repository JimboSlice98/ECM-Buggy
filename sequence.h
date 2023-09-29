#ifndef _sequence_H
#define _sequence_H

#include <xc.h>
#include "structures.h"

#define _XTAL_FREQ 64000000

void addMove(DATA *data, unsigned char type, unsigned char direction, unsigned char power, unsigned int time);
void backtrack(DATA *data);

#endif
