#include <xc.h>
#include "serial.h"

/************************************************
 *  Function to initialise USART
 ***********************************************/
void initUSART4(void) {
	// code to set up USART4 for Reception and Transmission
    RC0PPS = 0x12;            // Map EUSART4 TX to RC0
    RX4PPS = 0x11;            // RX is RC1
    
    TRISCbits.TRISC1 = 1;     // set TRIS value for RC1 pin

    BAUD4CONbits.BRG16 = 0;   // set baud rate scaling
    TX4STAbits.BRGH = 0;   	  // high baud rate select bit
    SP4BRGL = 51; 			  // set baud rate to 51 = 19200 bps
    SP4BRGH = 0;		      // not used

    RC4STAbits.CREN = 1; 	  // enable continuous reception
    TX4STAbits.TXEN = 1; 	  // enable transmitter
    RC4STAbits.SPEN = 1; 	  // enable serial port
}

/************************************************
 *  Function to wait for a byte to arrive on serial port and read it once it does 
 ***********************************************/
char getCharSerial4(void) {
    while (!PIR4bits.RC4IF); // wait for the data to arrive
    return RC4REG;           // return byte in RCREG
}

/************************************************
 *  Function to check the TX reg is free and send a byte
 ***********************************************/
void sendCharSerial4(char charToSend) {
    while (!PIR4bits.TX4IF); // wait for flag to be set
    TX4REG = charToSend;     // transfer char to transmitter
}

/************************************************
 *  Function to send a string over the serial interface
 ***********************************************/
void sendStringSerial4(char *string){
	// Hint: look at how you did this for the LCD lab 
    while(*string != 0){               // while the data pointed to isn't a 0x00 do below (strings in C must end with a NULL byte) 
		sendCharSerial4(*string++);    // send out the current byte pointed to and increment the pointer
    }
}

/************************************************
 *  Circular buffer functions for RX
 *  Retrieve a byte from the buffer
 ***********************************************/
char getCharFromRxBuf(void){
    if (RxBufReadCnt>=RX_BUF_SIZE) {RxBufReadCnt=0;} 
    return EUSART4RXbuf[RxBufReadCnt++];
}

/************************************************
 *  Function to add a byte to the Rx buffer
 ***********************************************/
void putCharToRxBuf(char byte){
    if (RxBufWriteCnt>=RX_BUF_SIZE) {RxBufWriteCnt=0;}
    EUSART4RXbuf[RxBufWriteCnt++]=byte;
}

/************************************************
 *  Function to check if there is data in the RX buffer
 *  1: there is data in the buffer
 *  0: nothing in the buffer
 ***********************************************/
char isDataInRxBuf (void){
    return (RxBufWriteCnt!=RxBufReadCnt);
}

/************************************************
 *  Circular buffer functions for TX
 *  Retrieve a byte from the buffer
 ***********************************************/
char getCharFromTxBuf(void){
    if (TxBufReadCnt>=TX_BUF_SIZE) {TxBufReadCnt=0;} 
    return EUSART4TXbuf[TxBufReadCnt++];
}

/************************************************
 *  Function to add a byte to the Tx buffer
 ***********************************************/
void putCharToTxBuf(char byte){
    if (TxBufWriteCnt>=TX_BUF_SIZE) {TxBufWriteCnt=0;}
    EUSART4TXbuf[TxBufWriteCnt++]=byte;
}

/************************************************
 *  Function to check if there is data in the RX buffer
 *  1: there is data in the buffer
 *  0: nothing in the buffer
 ***********************************************/
char isDataInTxBuf (void){
    return (TxBufWriteCnt!=TxBufReadCnt);
}

/************************************************
 *  Function to add a string to the Tx buffer
 ***********************************************/
void TxBufferedString(char *string){
	// Hint: look at how you did this for the LCD lab
    while(*string != 0){               // while the data pointed to isn't a 0x00 do below (strings in C must end with a NULL byte) 
		putCharToTxBuf(*string++);     // send out the current byte pointed to and increment the pointer
    }
}

/************************************************
 *  Initialise interrupt driven transmission of the Tx buf
 ***********************************************/
void sendTxBuf(void){
    if (isDataInTxBuf()) {PIE4bits.TX4IE=1;} // enable the TX interrupt to send data
}
