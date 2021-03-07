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

//shared variables
unsigned char tmpC = 0x00;
unsigned char incButton; //incrase button
unsigned char decButton; //decrease button
unsigned char init_output = 0x00;
unsigned char go = 0x00; //0x00 = initial //0x01 = seq0 //0x02 = seq1 //0x03 = seq2
//SEQUENCE ARRAYS
Queue seq0;
Queue seq1;
Queue seq2;
//end shared variables
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

//Pressing both buttons together toggles the system on or off;
enum toggleSM_States{t_WAIT, t_TOGGLE};

int toggleSMTick(int state){
  switch(state){ //state transitions
    case t_WAIT:
      if(incButton && decButton){
        state = t_TOGGLE;
      }else{
        state = t_WAIT;
      }
      break;
    case t_TOGGLE: state = t_WAIT; break;
  }
  switch(state){ //state actions
    case t_WAIT: break;
    case t_TOGGLE:
    sReg = 0;
    go = (go > 0x00) ? 0x00 : 0x01; //if go is greater than 0 then it is on so turn off else turn on
    tmpC = 0x01;
    PORTC = tmpC;
    break; //toggle on or off (1 on / 0 off)
  }
  return state;
}

enum changeSeqSM_States{cs_WAIT, cs_INC, cs_DEC};

//change led sequence with two buttons
int changeSeqSMTick(int state){
  switch(state){ //state transitions
    case cs_WAIT:
      if(incButton && (go > 0x00)){
        state = cs_INC;
      }else if(decButton && (go > 0x00)){
        state = cs_DEC;
      }else{
        state = cs_WAIT;
      }
      break;
    case cs_INC: state = cs_WAIT; break;
    case cs_DEC: state = cs_WAIT; break;
    default: state = cs_WAIT; break;
  }
  switch(state){//state actions
    case cs_WAIT: break;
    case cs_INC:
      if(go == 0x03){ //if it is at the last sequence go back to first sequence
        go = 0x01;
      }else{
        go = go + 0x01; //change to next sequence
      }
      break;
    case cs_DEC:
      if(go > 0x01){ //if it is greater than the first sequence we can decrement
        go = go - 0x01;
      }else{
        go = 0x03; //if less at first sequence and we try to decrement go to the last sequence (0x03)
      }
      break;
  }
  return state;
}

enum seq0SM_States{seq0_WAIT, seq0_ON};

//sequence 0 SM
int seq0SMTick(int state){
  switch(state){//state transitions
    case seq0_WAIT: state = (go == 0x01) ? seq0_ON : seq0_WAIT; break;
    case seq0_ON: state = (go != 0x01) ? seq0_WAIT : seq0_ON; break; //while go is on 0x01 stay on ON
    default: state = seq0_WAIT; break;
  }
  switch(state){//state actions
    case seq0_WAIT: break;
    case seq0_ON:
      transmit_data(seq0->front);
      QueueEnqueue(seq0, seq0->front); //push front to back
      QueueDequeue(seq0); //pop to move to next value in queue
      break;
  }
  return state;
}

enum seq1SM_States{seq1_WAIT, seq1_ON};

//sequence 1 SM
int seq1SMTick(int state){
  switch(state){ //State machine transitions
    case seq1_WAIT: state = (go == 0x02) ? seq1_ON : seq1_WAIT; break;
    case seq1_ON : state = (go != 0x02) ? seq1_WAIT : seq1_ON; break;
    default: state = seq1_WAIT; break;
  }
  switch(state){//state actions
    case seq1_WAIT: break;
    case seq1_ON:
      transmit_data(seq1->front);
      QueueEnqueue(seq1, seq1->front);
      QueueDequeue(seq1);
      break;
  }
  return state;
}

enum seq2SM_States{seq2_WAIT, seq2_ON};
//sequence 2 SM
int seq2SMTick(int state){
  //local variables
  unsigned char i;
  unsigned char seqSize = 5;
  unsigned char seq[seqSize] = {0x18, 0x24, 0x42, 0x81};
  switch(state){//State machine transitions
    case seq2_WAIT: state = (go == 0x03) ? seq3_ON : seq3_WAIT; break;
    case seq2_ON: state = (go != 0x03) ? seq3_WAIT : seq3_ON; break;
    default: state = seq2_WAIT; break;
  }
  switch(state){//state actions
    case seq2_WAIT: i = 0; break;
    case seq2_ON:
      transmit_data(seq2->front);
      QueueEnqueue(seq2, seq2->front);
      QueueDequeue(seq2);
      break;
  }
  return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;

    //initialize variables
    tmpC = 0x00;
    go = 0x00;
    incButton = ~PINA & 0x01; //button on PA0;
    decButton = ~PINA & 0x02; //button on PA1;
    //initialize queue sequences
    seq0 = QueueInit(4);
    QueueEnqueue(seq0, 0x81);
    QueueEnqueue(seq0, 0xC3);
    QueueEnqueue(seq0, 0xE7);
    QueueEnqueue(seq0, 0xFF);
    seq1 = QueueInit(2);
    QueueEnqueue(seq1, 0xAA);
    QueueEnqueue(seq1, 0x55);
    seq2 = QueueInit(4);
    QueueEnqueue(seq2, 0x18);
    QueueEnqueue(seq2, 0x24);
    QueueEnqueue(seq2, 0x42);
    QueueEnqueue(seq2, 0x81);

    //declare array of tasks
    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    const char start = -1;
    //Task1 (toggleSMTick)
    task1.state = start; //Task initial state
    task1.period = 100; //task period
    task1.elapsedTime = task1.period;
    task1.TickFct = &toggleSMTick;
    //Task2 (changeSeqSMTick)
    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &changeSeqSMTick;
    //Task3 (seq0SMTick)
    task3.state = start;
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &seq0SMTick;
    //Task4 (seq1SMTick)
    task4.state = start;
    task4.period = 100;
    task4.elapsedTime = task4.period;
    task4.TickFct = &seq1SMTick;
    //Task5 (seq2SMTick)
    task5.state = start;
    task5.period = 100;
    task5.elapsedTime = task5.period;
    task5.TickFct = &seq2SMTick;

    unsigned long GCD = tasks[0]->period;
    for(int i = 1; i < numTasks; i++){
      GCD = findGCD(GCD, tasks[i]->period);
    }

    //Set timer and turn on
    TimerSet(GCD);
    TimerOn();

    unsigned short i;
    while (1) {
      for(i = 0; i < numTasks; i++){
        if(tasks[i]->elapsedTime == tasks[i]->period){//task is ready to ticks
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //set next state
          tasks[i]->elapsedTime = 0; //reset the elapsed time for next cik
        }
        tasks[i]->elapsedTime += GCD;
      }
      while(!TimerFlag);
      TimerFlag = 0;
    }
    return 0; //Error: Program should not exit!
}
