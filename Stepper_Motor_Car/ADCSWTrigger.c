// ADCSWTrigger.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 SS3 to be triggered by
// software and trigger a conversion, wait for it to finish,
// and return the result.
// Daniel Valvano
// October 20, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "../tm4c123gh6pm.h"
#include "ADCSWTrigger.h"

// This initialization function sets up the ADC according to the
// following parameters.  Any parameters not explicitly listed
// below are not modified:
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: software trigger
// SS3 1st sample source: Ain1 (PE2)
// SS3 interrupts: flag set on completion but no interrupt requested

//SS2 priority 0--first
//SS2FIFO stores upto 4 samples--need 2 reading from Pin PE1 (Ain2) and PE2 (Ain1)

void ADC0_InitSWTriggerSeq3_Ch1(void){ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;   // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;         //    allow time for clock to stabilize
  GPIO_PORTE_DIR_R &= ~0x06;      // 2) make PE2,PE1 input
  GPIO_PORTE_AFSEL_R |= 0x06;     // 3) enable alternate function on PE2,PE1
  GPIO_PORTE_DEN_R &= ~0x06;      // 4) disable digital I/O on PE2,PE1
  GPIO_PORTE_AMSEL_R |= 0x06;     // 5) enable analog function on PE2,PE1
  SYSCTL_RCGC0_R |= 0x00010000;   // 6) activate ADC0 
  delay = SYSCTL_RCGC2_R;         
  SYSCTL_RCGC0_R &= ~0x00000300;  // 7) configure for 125K 
  ADC0_SSPRI_R = 0x3012;          // 8) Sequencer 2 is highest priority
  ADC0_ACTSS_R &= ~0x0004;        // 9) disable sample sequencer 2
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
	ADC0_SSMUX2_R &= ~0x00FF;       // clear SS2 field
  ADC0_SSMUX2_R += 0x21;          // 11) channel Ain1,Ain2 (PE2,PE1)
  ADC0_SSCTL2_R = 0x0060;         // 12) no TS0 D0, yes IE0 END0 --> no all, yes IE1,END1
  ADC0_ACTSS_R |= 0x0004;         // 13) enable sample sequencer 2
}


//------------ADC0_Reading------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion


unsigned long ADC0_Reading(void){
	unsigned long value = 0;
	unsigned long xPos;
	unsigned long yPos;
	ADC0_PSSI_R = 0x0004;
	while((ADC0_RIS_R&0x04) == 0){};
	xPos = ADC0_SSFIFO2_R & 0xFFF;
	yPos = ADC0_SSFIFO2_R & 0xFFF;
	value = (xPos << 12);
	value |= yPos;
	ADC0_ISC_R = 0x0004;
	return value;
			
}


