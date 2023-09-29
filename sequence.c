#include <xc.h>
#include "dc_motor.h"
#include "hardware.h"
#include "sequence.h"
#include "structures.h"
#include "timers.h"

/***********************************************
 *  Function to add move to moves to data structure
 ***********************************************/
void addMove(DATA *data, unsigned char type, unsigned char direction, unsigned char power, unsigned int time) {
    data->sequence->moves[data->sequence->index].type = type;            // add type data to sequence
    data->sequence->moves[data->sequence->index].direction = direction;  // add direction data to sequence
    data->sequence->moves[data->sequence->index].power = power;          // add power data to sequence
    data->sequence->moves[data->sequence->index].time = time;            // add time data to sequence
    data->sequence->index++;                                             // increment index counter
}

/***********************************************
 *  Function to backtrack through the sequence structure
 ***********************************************/
void backtrack(DATA *data) {
    // turn on the indicators to show the backtrack has started
    INDICATOR_L = 1;
    INDICATOR_R = 1;
    
    // iterate back through the sorted movements
    for (unsigned int i = data->sequence->index; i > 0; i--) {
        // turn movements
        if ((data->sequence->moves[i-1].type)) {
            rotate(!data->sequence->moves[i-1].direction, data->sequence->moves[i-1].power);
        }
        
        // traverse movements
        else {
            resetTimer();
            straight(!data->sequence->moves[i-1].direction, data->sequence->moves[i-1].power);
            // keep moving while the timer is less than the time taken by the move
            while (get16bitTMR0val() <= data->sequence->moves[i-1].time) {}
            stop();
        }
        
        __delay_ms(500);        // delay set after each backtrack move
    }
    
    // turn off the indicators once all moves have been executed
    INDICATOR_L = 0;
    INDICATOR_R = 0;
}
