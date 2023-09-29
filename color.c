#include <xc.h>
#include "color.h"
#include "dc_motor.h"
#include "hardware.h"
#include "i2c.h"
#include "structures.h"

/************************************************
 *  Function to initialise the colour click module using I2C
 ***********************************************/
void color_click_init(void) {   
    // setup colour sensor via i2c interface
    I2C_2_Master_Init();  // initialise i2c Master

    // set device PON
	 color_writetoaddr(0x00, 0x01);
    __delay_ms(3);        // need to wait 3 ms for everything to start up
    
    // turn on device ADC
	color_writetoaddr(0x00, 0x03);

    // set integration time
	color_writetoaddr(0x01, 0xD5);   
}

/************************************************
 *  Function to write to the colour click module
 *  'address' is the register address within the colour click to write to
 *	'value' is the value that will be written to that address
 ***********************************************/
void color_writetoaddr(char address, char value) {
    I2C_2_Master_Start();                // start condition
    I2C_2_Master_Write(0x52 | 0x00);     // 7 bit device address + Write mode
    I2C_2_Master_Write(0x80 | address);  // command + register address
    I2C_2_Master_Write(value);    
    I2C_2_Master_Stop();                 // stop condition
}

/************************************************
 *  Function to read the colour channels based on addresses
 *  RED: 0x16, GREEN: 0x18, BLUE: 0x1A, WHITE: 0x14 (R, G, B, W)
 *	Returns a 16 bit ADC value representing colour intensity
 ***********************************************/
unsigned int color_read(char address) {
	unsigned int tmp;
	I2C_2_Master_Start();                 // start condition
	I2C_2_Master_Write(0x52 | 0x00);      // 7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | address);   // command (auto-increment protocol transaction) + start at COLOR low register
	I2C_2_Master_RepStart();		      // start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);      // 7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);		      // read the COLOR LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8);  // read the COLOR MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();                  // stop condition
	return tmp;
}

/************************************************
 *  Function to convert RGB data to pseudo HSV data
 *  Note, this uses the HSV formulae to generate the HSV formula 
 *  but as the RGB data is not normalised this is not true HSV
 ***********************************************/
HSV rgb2hsv(struct RGB rgb) {
    HSV hsv;                            // declare structure to return HSV values
    hsv.c = rgb.c;                      // assign clear value to clear channel reading
    unsigned int rgbMin, rgbMax, diff;  // initialise variables to store them as unsigned int

    // logic to find the numerical max/min RGB value
    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    diff = rgbMax - rgbMin;     // compute the difference between max and min

    // logic to find value and saturation according to conversion formulae
    hsv.v = rgbMax;
    if (hsv.v == 0) {
        hsv.s = 0; 
    } else {
        hsv.s = diff;
    }

    // logic to find the Hue scaled to 3600
    if (!diff) {
        hsv.h = 0;
    } else if (rgb.r == rgbMax) {
        hsv.h = (3600 + 600 * (rgb.g - rgb.b) / diff) % 3600;
    } else if (rgb.g == rgbMax) {
        hsv.h = (1200 + 600 * (rgb.b - rgb.r) / diff) % 3600;
    } else {
        hsv.h = (2400 + 600 * (rgb.r - rgb.g) / diff) % 3600;
    }

    return hsv;
}

/************************************************
 *  Function to return the RGB sensor data as an RGB structure
 ***********************************************/
RGB getRGB(void) {
    RGB rgb;                   // temporary structure to return RGB values 
    rgb.r = color_read(0x16);  // read and store red value
    rgb.g = color_read(0x18);  // read and store green value
    rgb.b = color_read(0x1A);  // read and store blue value
    rgb.c = color_read(0x14);  // read and store clear value
    
    return rgb;
}

/************************************************
 *  Function to store the sensor data in the data structure
 ***********************************************/
void storeColor(DATA *data) {
    data->hsv = rgb2hsv(getRGB());  // convert and store HSV from RGB value
}

/************************************************
 *  Function to store clear channel data to data structure
 ***********************************************/
void storeAmbient(DATA *data) {
    data->ambLight = color_read(0x14);  // read the clear channel from sensor
}

/************************************************
 *  Function to store reference calibration data for each color
 ***********************************************/
void storeCalibration(DATA *data) {
    unsigned char i = 0;
    
    while (i < 9) {
        LED_flash(i + 1);      // flash indicators to show what color to calibrate
        while (BUTTON_RF2) {}  // wait for button press to store calibration
        
        LED_on();
        __delay_ms(1500);   
        
        // store the calibration color in the data structure
        data->cal[i] = rgb2hsv(getRGB());        
        LED_off();
        i++;
    }   
}

/************************************************
 *  Function to return the numerical difference between two HSV values
 ***********************************************/
unsigned int hsvDiff(struct HSV h1, struct HSV h2) {
    // find the absolute difference for each HSV component
    unsigned int h = h1.h > h2.h ? h1.h - h2.h : h2.h - h1.h;
    unsigned int s = h1.s > h2.s ? h1.s - h2.s : h2.s - h1.s;
    unsigned int v = h1.v > h2.v ? h1.v - h2.v : h2.v - h1.v;
    unsigned int c = h1.c > h2.c ? h1.c - h2.c : h2.c - h1.c;
    
    // obtain the sum based on the absolute difference of each channel
    return h + s + v + c;
}

/************************************************
 *  Function to return an integer value based on the detected color
 *  Iterates through each color and finds the numerical difference
 *  between the detected color and calibration color and returns the lowest color
 ***********************************************/
unsigned char detectColor(DATA *data) {
    storeColor(data);                 // read the color of the card/wall
    
    char decision = 9;                // declare a decision output variable
    unsigned int difference = 20000;  // declare an HSV difference variable at max difference
    
    // iterate through the list of calibrated value and computing the difference to determine the value with the smallest difference
    for (char i = 0; i < 9; i++) {
        unsigned int tmp = hsvDiff(data->hsv, data->cal[i]);
        if (tmp < difference) {
            difference = tmp;         // set the difference if it is smaller than the current value
            decision = i;             // select the color with the lowest difference
        }
    }
    
    // return the index of the best guess (smallest difference) for the buggy to perform the action
    return decision;
}
