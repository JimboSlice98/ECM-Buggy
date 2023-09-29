#ifndef _color_H
#define _color_H

#include <xc.h>
#include "structures.h"

#define _XTAL_FREQ 64000000 // note intrinsic _delay function is 62.5ns at 64,000,000Hz  

void color_click_init(void);
void color_writetoaddr(char address, char value);
unsigned int color_read(char address);
HSV rgb2hsv(struct RGB rgb);
RGB getRGB(void);
void storeColor(DATA *data);
void storeAmbient(DATA *data);
void storeCalibration(DATA *data);
unsigned int hsvDiff(struct HSV h1, struct HSV h2);
unsigned char detectColor(DATA *data);

#endif
