/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char tmpC = 0x00;
unsigned char incButton;
unsigned char decButton;
/*
Here is a function used to transmit an 8-bit char to a shift register. This example assumes RCLK and SRCLK do NOT share the same clock line.

For the following function. Use the following connections:
PORTC[3] connected to SRCLR
PORTC[2] connected to RCLK
PORTC[1] connected to SRCLK
PORTC[0] connected to SER
*/
void transmit_data(unsigned char data){
  int i;
  for(i = 0; i < 8; ++i){
    //Sets SRCLR to 1 allowing data to be set
    //Also clears SRCLK in preparation of  sending data
    PORTC = 0x08;
    //set SER = next bit of data to be sent.
    PORTC |= ((data>>i)&0x01);
    //set SRCLK = 1. Rising edge shifts next bit of data into the shift register
    PORTC |= 0x02;
  }
  //set RCLK = 1. Rising edge copies data from "shift" register to "storage" register
  PORTC |= 0x04;
  //clears all lines in preparation of a new transmission
  PORTC = 0x00;
}
enum shiftSM_States{INIT, WAIT, INC, DEC}state;

void tick(){
  switch(state){ //State transitions
    case INIT: state = WAIT; break;
    case WAIT:
      if(incButton && tmpC < 0xFF){
        state = INC;
      }else if(decButton && tmpC > 0x00){
        state = DEC;
      }else{
        state = WAIT;
      }
      break;
    case INC: state = WAIT; break;
    case DEC: state = WAIT; break;
    default: state = INIT;
  }
  switch(state){
    case INIT: tmpC = 0x00; break;
    case WAIT: break;
    case INC:
      tmpC = tmpC + 0x01;
      transmit_data(tmpC);
      break;
    case DEC:
      tmpC = tmpC - 0x01;
      transmit_data(tmpC);
      break;
  }

}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    /* Insert your solution below */
    tmpC = 0x00;
    incButton = ~PINA & 0x01; //button on PA0;
    decButton = ~PINA & 0x02; //button on PA1;

    while (1) {
      tick();
    }
    return 1;
}
