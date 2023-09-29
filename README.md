# Course project - Mine navigation search and rescue

## Contents
- [Challenge Brief](#challenge-brief)
- [Source Control](#source-control)
- [General Overview](#general-overview)
- [File Structure](#file-structure)
- [Code Explanation](#code-explanation)
  - [Data Storage](#data-storage)
  - [Colour Detection and Recognition](#colour-detection-and-recognition)
    - [Detection](#detection)
    - [Recognition](#recognition)
  - [Movement](#movement)
    - [Straight Motion](#straight-motion)
    - [Rotational Motion](#rotational-motion)
  - [Backtracking](#backtracking)
    - [Time Tracking](#time-tracking)
    - [Sequence](#sequence)
- [Operating Procedure](#operating-procedure)
  - [Calibration](#calibration)
  - [Starting](#starting)
  - [Exception Handling](#exception-handling)
- [Further Improvements](#further-improvements)
  - [Hardware Tools](#hardware-tools)
  - [Software Tools](#software-tools)
- [Video Demonstration](#video-demonstration)


## Challenge Brief

A detailed copy of the challenge brief can be found in a [separate readme](./challenge_brief.md)

As a pair, the challenge was to develop an autonomous robot that can navigate a "mine" using a series of instructions coded in coloured cards and return to its starting position. The robot would have to perform the following tasks:

- [x] Navigate towards a coloured card and stop before impacting the card
- [x] Read the card colour
- [x] Interpret the card colour using a predefined code and perform the navigation instruction
- [x] When the final card is reached, navigate back to the starting position
- [x] Handle exceptions and return back to the starting position if final card cannot be found

From the following tasks, the code can be divided into 3 main components: colour detection and recognition; movement; and backtracking to the starting position.

## Source Control

Github was used for source control, with each member working on individual branches to test out different components of the code. The components of the code being worked on are stated in the commit messages. Both members would push code to their individual branches and merge code from the other member's branch when essential portions of code need to be tested together.

## General Overview

Before the autonomous vehicle (buggy) enters the "mine", the buggy is first calibrated to store the Hue, Saturation, Value and Clear (HSV) values of each coloured card that may be encountered in the "mine". When the buggy sets off, it uses the white light from the tri-colour LED and stores the `clear` value of the ambient light. This `clear` value is used by the buggy to determine if it has reached a wall or a coloured card and stop before impacting the card. 

When stopping in front of the card, the buggy moves towards to wall to realign itself and reads the card colour at the wall. This standardises the distance at which the HSV values are read for better colour recognition. The detected HSV values are then compared to the array of HSV values. The closest numerical value indicates the colour of the wall. The buggy would then reverse away from the wall to provide ample space to perform the command.

As the buggy moves, the move action and time is stored in a `SEQUENCE` structure which is used for backtracking when the final card cannot be found or if the buggy has encountered a *white* card. For the time of each move, the built-in `Timer0` is used to monitor the movements that the buggy has taken.

For exception handling, i.e., the final card cannot be found, the buggy will attempt to read the final colour, which is most likely a wall, 3 times. Upon confirmation that the final card cannot be found (reads the wall 3 times), the buggy would then return to the starting position. 

## File Structure

The file structure and how the code is organised in the project is detailed below:

| File Name                    | Function                                         |
|------------------------------|--------------------------------------------------|
| [main.c](main.c)             | Main code for running the program                |
| [structure.h](structure.h)   | Defines all structures used in the program       |
| [color.c](color.c)           | Colour detection and recognition                 |
| [dc_motor.c](dc_motor.c)     | DC motors and movement of the buggy              |
| [sequence.c](sequence.c)     | Adding moves to the sequence and backtracking    |
| [hardware.c](hardware.c)     | Initialise the hardware for the buggy            |
| [timers.c](timers.c)         | Initialise timer0 and reading 16 bit timer value |
| [interrupts.c](interrupts.c) | Initialise interrupts and handle timer overflow  |
| [i2c.c](i2c.c)               | Communication between colour click and clicker   |
| [serial.c](serial.c)         | Serial monitor used for testing                  |
## Code Explanation

### Data Storage

All structures used in the code are defined in [structures.h](structures.h)

For the main portion of the code, the data is stored in a singular data structure that will be passed into different functions in the code via its pointer. The data structure is defined as follows:

```c
// structures.h
typedef struct DATA {       // definition of overall DATA structure
  HSV cal[9];               // nested structure to store calibration data
  HSV hsv;                  // nested structure to store instantaneous color
  unsigned int ambLight;    // integer to store clear channel data for wall detection
  unsigned char backtrack;  // variable to store if the backtrack functionality is to be executed
  unsigned char count;      // variable to count the number of failed color detections
  SEQUENCE *sequence;       // nested structure to store the sequence of moves
} DATA;
```

Note that the sequence structure is not directly contained with in the data structure object but instead a pointer. This is to meet the sequential memory constraints of the PIC whilst still having a single data object to pass between functions.

### Colour Detection and Recognition

#### Detection

The colour click used for colour detection has a tri-colour LED and a 4 channel red, green, blue, clear (RGBC) photodiode sensor. For the tri-colour LED, our team has decided to switch on all three colours, resulting in a white colour emitted from the buggy.

When detecting colour, the 4 channel RGBC photodiode sensor obtains the RGBC value as 16 bit values. When initially using raw RGBC values, there were difficulties in distinguishing between some colours. Hence, our team had opted to use hue, saturation, value, clear (HSVC) values for colour recognition instead. The standard formula of obtaining HSV values from RGB values are shown below:

```c
Normalise r, g, b to be from 0 to 1
Compute cmax, cmin, difference
// Hue Calculation
if cmax and cmin equal 0, then h = 0
if cmax equal r, then h = (60 * ((g – b) / diff) + 360) % 360
if cmax equal g, then h = (60 * ((b – r) / diff) + 120) % 360
if cmax equal b, then h = (60 * ((r – g) / diff) + 240) % 360
// Saturation Calculation
if cmax = 0, then s = 0
if cmax does not equal 0, then s = (diff/cmax)*100
// Value computation
v = cmax*100
```

Since the RGB values were obtained as a 16 bit value (as opposed to standardised 255 values) with large variations in measurements, our team had decided to use a pseudo HSV value calculation as detailed below:

```c
Compute cmax, cmin, difference (value range: 0-65535)
// Hue Calculation (value range: 0 - 3600)
if cmax and cmin equal 0, then h = 0
if cmax equal r, then h = (600 * ((g – b) / diff) + 3600) % 3600
if cmax equal g, then h = (600 * ((b – r) / diff) + 1200) % 3600
if cmax equal b, then h = (600 * ((r – g) / diff) + 2400) % 3600
// Saturation Calculation (value range: 0-65535)
if cmax = 0, then s = 0
if cmax does not equal 0, then s = diff
// Value computation (value range: 0-65535)
v = cmax
// Clear calculation (value range: 0-65535)
clear value is the same as the one measured by the photodiode sensor
```

Although the values calculated have a large range, from the testing and measurements, we realised that the range of Saturation, Value and Clear were less than 5000 for all colours. Hence, we decided to keep the range of the Hue to 3600.

#### Recognition

For the recognition process, there are two different aspects to consider: recognising that the autonomous buggy has reached a coloured card/wall and recognising the colour of the card in front of the buggy.

To recognise if the buggy has reached a wall, the clear readings read from the photodiode sensor. Before each straight movement towards a wall, the clear value of the ambient light is calibrated and the buggy would stop when the clear value strays too far from the ambient light value. This ensures that the buggy stops before hitting the coloured cards. The section of the code that governs this logic is as follows

```c
// dc_motor.c - void move2wall(DATA *data)
if (data->hsv.c < data->ambLight - 13 || data->hsv.c > data->ambLight + 30)
```

For the colour recognition process, there is a calibration process before going through each "mine", where the colour of each card of the maze is calibrated before beginning. This takes into account the ambient light of the "mine" to ensure the proper colour recognition process.

After each colour value has been stored in the `DATA` struct mentioned [above](#data-storage), the buggy would then begin navigating through the maze. Upon stopping before the coloured card, we read the current HSV and clear values and compute the difference with each of the calibrated values to determine the best guess of the wall colour. The absolute difference for each value is calculated and the colour with the smallest value is the prediction. The snippet of the code used to calculate the difference betweeen two HSVC values is detailed below:

```c
// color.c
unsigned int hsvDiff(struct HSV h1, struct HSV h2) {
  // find the absolute difference for each HSV component
  unsigned int h = h1.h > h2.h ? h1.h - h2.h : h2.h - h1.h;
  unsigned int s = h1.s > h2.s ? h1.s - h2.s : h2.s - h1.s;
  unsigned int v = h1.v > h2.v ? h1.v - h2.v : h2.v - h1.v;
  unsigned int c = h1.c > h2.c ? h1.c - h2.c : h2.c - h1.c;
  
  // obtain the sum based on the absolute difference of each channel
  return h + s + v + c;
}
```

The move that the buggy performs at each coloured card is detailed in the [challenge brief](./challenge_brief.md#mine-environment-specification). Although not defined in the challenge brief, the action when encountered with the *black* card would be to backtrack to the starting position and to start again from the beginning.

### Movement

The movement of the buggy can be found in the [dc_motor.c](dc_motor.c) file. The movement of the buggy is controlled by the DC_motors, which can be separated into the left motor `motorL` and the right motor `motorR`. Since the motors are only required to be controlled in the movement of the buggy, both structs `motorL` and `motorR` are initialised and kept local in the `dc_motor.c` file.

The movement of the buggy can be divided into 2 different variations, the straight movement (forward and backward), as well as the rotational movement (left or right turn). The direction of the dc_motors are then set by looking at the direction argument in both movement functions.

#### Straight Motion

When considering the straight motion, we have to consider two factors, the direction, speed of the motion (determined by the power of the dc motors). The generic function for the straight motion of the buggy is as follows:

```c
// direction: backward -> 0; forward -> 1
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
```

#### Rotational Motion

The rotational movement has two factors, direction and the angle of the turn. With that in mind, the generic functions for the rotational motion is as follows:

```c
// direction: left-> 0; right -> 1 
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
```

As the directions are of values 0 and 1, this makes backtracking a simpler process as we would be able to reverse the bits and the buggy would move in the opposite direction, essentially moving backwards towards the starting position.

### Backtracking

#### Time Tracking

The time taken for the straight motions are required to accurately backtrack to the beginning. One of the key variables when implementing the `timer0` is to set an appropriate prescaler (PS) for the backtracking. To set the prescaler, 2 key factors were taken into account, the time taken to overflow and the interval for each increment. Taking this into consideration, we have chosen a `1:2^14` PS

```c
// Key timer calculations
PS = 2^14
Overflow time = PS * Max Value * 4 / FOSC
              = 2^14 * (2^16-1) * 4 / (64*10^6)
              = 67.1 seconds

Interval time = PS * 4 / FOSC
              = 2^14 * 4 / (64*10^6)
              = 1.024 millisecond
```

The overflow time of roughly a minute gives ample time for the buggy to make a movement and capture the current 16 bit timer value before overflowing. Moreover, the interval time is small enough to offer more accurate backtracking of the buggy.

#### Sequence

The backtracking sequence would essentially move in the direction opposite to the direction moved in its forward motion as defined in the [Movement section](#movement). The backtracking logic and adding moves to the sequence structure is found in [sequence.c](sequence.c)

For the backtracking motion, we use the two structures defined below: one to store each movement and another structure to store the complete sequence:

```c
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
```

Since the directions of the motions are set to 0 and 1, they can be easily reversed using a `!` operator and by using the timer, we can determine the distance moved by the straight motion. The logic for the backtracking found in [sequence.c](sequence.c) is as follows:

```c
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
```

## Operating Procedure

### Calibration

Before running the buggy in the mine, the buggy would have to be calibrated to ensure that the buggy can accurately detect the colours in the maze. By pressing the `RF3 button` on the clicker board, this triggers the calibration loop found in [color.c](color.c) as detailed below:

```c
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
```

In the calibration loop, there are LED lights used to indicate the colour being calibrated where the corresponding colours can be derived from the table below. The colours HSV values of the colours would be stored in the `DATA` structure when the `RF2 button` is pressed.

| Color        | `i` | LED Flashes |
|--------------|-----|-------------|
| Red          | 0   | 1           |
| Green        | 1   | 2           |
| Blue         | 2   | 3           |
| Yellow       | 3   | 4           |
| Pink         | 4   | 5           |
| Orange       | 5   | 6           |
| Light Blue   | 6   | 7           |
| White        | 7   | 8           |
| Black (Wall) | 8   | 9           |

Note that the LED will stop flashing after the last colour, black, has been calibrated. This denotes the completion of the calibration process. If the user is unsatisfied, upon completion the user can recalibrate the values by pressing the `RF3 buttton` again.

### Starting

When all the different colours have been calibrated. The buggy can then be put into the "mine" and the `RF2 button` can be pressed to start the buggy in its course. 

The buggy would then calibrate its clear value to the ambient light, to track whether the buggy has reached a card, where the clear value would deviate from the current value. After each action, the buggy would calibrate to ambient light once more to handle the difference in ambient light at different angles.

### Exception Handling

In the case that the final *white* card cannot be found, the buggy should be able to return to the starting position. To accurately confirm that the final card has not been found, the buggy would attempt to read the colour 3 times. If the *black wall* is read 3 times, the buggy would turn on the backtrack flag and the buggy would return to its starting position.

## Further Improvements

Although the key objectives of the project were met within the time constraints, further improvements that could be considered if time permitted would be:

- Using interrupts for sensing the change in the clear value so that we can save memory instead of polling for the change in clear value for each segment of the `while` loop.
- Added functionality to keep the buggy on a straight path and ensure rotation is highly accurate to remove the need for adjusting the buggy manually on its course.

For future development purposes, the following hardware and software tools were used for development.

### Hardware Tools

Development was conducted using the following hardware tools:
- [Buggy](./datasheets/buggy.pdf)
- [Clicker 2](./datasheets/clicker-2-manual-pic18fk.pdf)
- [Pickit](./datasheets/PIC18(L)F67K40%20Data%20Sheet%2040001841D.pdf)

### Software Tools

Development was conducted using the following software development tools:
- [MPLAB X IDE](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide)
- [XC8 Compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers)

## Video Demonstration

The video demonstration shows the calibration process, maze testing and exception handling. 

[Youtube Link](https://youtu.be/_x_aiZtfjXM)

[Microsoft Stream Link](https://imperiallondon-my.sharepoint.com/:v:/g/personal/kwo18_ic_ac_uk/Ea1grgw9HvlPuhedoEoS3r8B2EGL1QcTFGvIGRXE_utjLA?e=sQkwJY)

[Google Drive Link](https://drive.google.com/file/d/12hQsx1XadRwc3yn_S3i-K2OHcbhLe7bR/view?usp=sharing)
