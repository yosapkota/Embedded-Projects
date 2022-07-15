/* Interfacing DHT11 Sensor */

// ***** 1. Pre-processor Directives Section *****
#include <stdio.h>   // standard C library 
#include <math.h>
#include <stdint.h>
#include "uart.h"    // functions to implment input/output
#include "TExaS.h"   
#include "PLL.h"     
#include "tm4c123gh6pm.h"


// Built-in RGB LEDs
#define RED 	(*((volatile unsigned long *)0x40025008))
#define GREEN (*((volatile unsigned long *)0x40025020))
#define BLUE  (*((volatile unsigned long *)0x40025010))


// define input/output pin
#define PE1                     (*((volatile unsigned long *)0x40024008))

// threshold for LOW value for pulseWidth < 26 us
#define LOW_PULSE 26


void PortF_Init(void);
void startPulse(void);

//delay using timer1A to determine LOW/HIGH bit
void timer1A_delayus(int time);

// ***** 3. Subroutines Section *****
int main (void) {
	
	int humid = 0, temp = 0;
	uint8_t data[5] = {0,0,0,0,0};
	uint8_t checkSum = 0;
	int d,i,j, pulseWidth;

	PortF_Init();
	PLL_init();
	UART_Init();

  while(1) {
		RED ^= 0x02;
		startPulse(); 
		
/* wait for two pulses before reading pulse stream */
      while (PE1 == 0x02);
      while (PE1 == 0);
      while (PE1 == 0x02);	
		
		/* start reading data */
		for(d = 0; d < 5; d++){        // 5 bytes of data
		for (i = 0; i < 8; i++){   // reading each bits
			  do { timer1A_delayus(1); }
        while (PE1 == 0);
			  pulseWidth = 0;
				
				do{
					pulseWidth++;
					timer1A_delayus(1);
				} while (PE1 == 0x02);
				
				data[d] = data[d] | (( pulseWidth > LOW_PULSE) << (7-i));
		}
	}
	
	for (j= 0; j < 4; j++){
		checkSum += data[j];
	}
	
	humid = data[0];
	temp = data[2];
	
//	printf("Temp      : %d \n", temp);
//  printf("Humidity  : %d \n", humid);
	
	if(checkSum == data[4]){
		printf("Temp      : %d \n", temp);
		printf("Humidity  : %d \n", humid);
  }
	else
		printf("Error \n");
	 timer1A_delayus(2000000);
}
	
	}

/*PORTF Initialization*/
void PortF_Init(void){ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}


/* microsecond delay using one-shot mode */

void timer1A_delayus(int time){
	SYSCTL_RCGCTIMER_R |= 0x02 ;  // enable clock Timer Block 
	TIMER1_CTL_R = 0; //disable timer before initializatiom
	TIMER1_CFG_R = 0x00;  //32-bit timer mode
	TIMER1_TAMR_R = 0x01; //one shot mode and down counter
	TIMER1_TAILR_R = 80*time - 1; //timer A load value register
	TIMER1_ICR_R = 0x01; // clear TimerA timeput flag
	TIMER1_CTL_R = 0x01; //enable timer1A
	while( (TIMER1_RIS_R & 0x01) == 0);   // wait for TimerA timeout flag to set
	
}


/* start comms with DHT11 using PE1 as DATA Single-Bus 
   PORT E initialization
   PE1 congfiured as Output and Input pin*/
void startPulse(void){
  volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R |= 0x02;         // PE1 output
  GPIO_PORTE_AFSEL_R &= ~0x02;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x02;      // no analog
  GPIO_PORTE_DEN_R |= 0x02;         // enable PE1
	
	PE1 = 0x00;  // PE1 pulled low for 20 ms
	timer1A_delayus(20000); 
	PE1 = 0x02;  // PE1 pulled high after 20 ms
	
	GPIO_PORTE_DIR_R &= ~0x02;        // PE1 input
}



