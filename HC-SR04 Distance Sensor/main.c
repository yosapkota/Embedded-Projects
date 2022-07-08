#include <stdio.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "UART.h"

// Built-in RGB LEDs
#define RED 	(*((volatile unsigned long *)0x40025008))
#define GREEN (*((volatile unsigned long *)0x40025020))
#define BLUE  (*((volatile unsigned long *)0x40025010))

/// On-board switches
#define SW  (*((volatile unsigned long*) 0x40025044))
#define SW1 (*((volatile unsigned long*) 0x40025040))
#define SW2 (*((volatile unsigned long*) 0x40025004))

// TRIG Pin
#define PE1  (*((volatile unsigned long*) 0x40024008))

void PortF_Init(void);
void PortE_Init(void);

/*Timer to measure the pulse width and create delays*/
void Timer0Capture_Init(void);
int Timer0A_pulseWidthCapture(void);
void timer1A_delayus(int);

  uint32_t timerCountReturn = 0;
  uint32_t distance = 0;

//void SystemInit(){}
int main(){
	
	
	PortF_Init();
	PortE_Init();
	PLL_init();
	
	Timer0Capture_Init();
	
	while(1){
		
		//set the TRIG PIN LOW initially
		PE1 = 0x00;
		//wait for 60 ms;
		timer1A_delayus(60000);
		
		//set the TRIG PIN HIGH for 10 us
		PE1 = 0x02;
		
	  timer1A_delayus(10);  
		PE1 = 0x00;
		
		/*set the TRIG PIN LOW to broadcast 8 cycle burst
		of ultrasound at 40KHz*/

	  timerCountReturn = Timer0A_pulseWidthCapture();
		distance = (timerCountReturn*0.034)/200;  
		
		
		/* test the return value */
		if(distance < 10)
			BLUE ^= 0x04;
		else
			RED ^= 0x02;
	}
	
}

/*PORTF Initialization*/
void PortF_Init(void){
	volatile unsigned long delay; 
  SYSCTL_RCGC2_R |= 0x20;      					// 1) PORT F
  delay = SYSCTL_RCGC2_R;      					// 2) no need to unlock 
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   		// 3) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           		// allow changes to PF4-0 
	GPIO_PORTF_AMSEL_R = 0x00; 						// 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00; 						// 4) enable regular GPIO 
  GPIO_PORTF_DIR_R &= ~0x11;    				// 5) input in switch PF0 and PF4
	GPIO_PORTF_DIR_R |= 0x0E;
  GPIO_PORTF_AFSEL_R = 0x00; 						// 6) regular function on PF0 to PF4 
  GPIO_PORTF_DEN_R = 0x1F;    					// 7) enable digital on PF0 to PF4
	GPIO_PORTF_PUR_R = 0x11;     					// 8) enable pullup resistors on PF4,PF0  
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

/* Initialize the Timer0A for edge capture mode*/

void Timer0Capture_Init(void){
	SYSCTL_RCGCTIMER_R |= 0x01 ;  // enable clock Timer Block 0
	SYSCTL_RCGC2_R |= 0x02;       // enable clock to Port B

	GPIO_PORTB_AMSEL_R &= ~0x7F;          //  disable analog function on PB6-PB0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;     //  enable regular GPIO 
  GPIO_PORTB_DIR_R &= ~0x40;             //  PB6 input for ECHO
  GPIO_PORTB_AFSEL_R |= 0x40;           //  PB6 alternate function
  GPIO_PORTB_DEN_R |= 0x40;             //  enable digital on PB6
	
	GPIO_PORTB_PCTL_R &= ~0x0F000000;      /* configure PB6 for T0CCP0 */
	GPIO_PORTB_PCTL_R |= 0x07000000;
	
	TIMER0_CTL_R &= ~1;    // disable Timer0A during setup
	TIMER0_CFG_R = 4; 		 // 16-bit timer mode
	TIMER0_TAMR_R = 0x17;  // up-count, edge-time, capture mode
	TIMER0_CTL_R |= 0x0C; // capture the rising edge and falling edge--> 11 in the 3rd and 4th bit of the register
  TIMER0_CTL_R |= 1;     // enable Timer0A
}

int Timer0A_pulseWidthCapture(void) {
 int risingEdge, fallingEdge;
	
 /*capture the rising edge */
	
	TIMER0_ICR_R = 4;										// clear TIMEROA capture flag
	while((TIMER0_RIS_R & 4) == 0);      // wait till rising edge is captured

	risingEdge = TIMER0_TAR_R;          // save the time stamp
	
	TIMER0_ICR_R = 4;										// clear TIMEROA capture flag
	while((TIMER0_RIS_R &4) == 0);      // wait till rising edge is captured
	
	fallingEdge = TIMER0_TAR_R;
	
	return (fallingEdge - risingEdge) & 0x00FFFFFF; //return the time difference
}

// Init PortE
void PortE_Init(void){
	int delay;
  SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R |= 0x02;         // PE1 output
  GPIO_PORTE_AFSEL_R &= ~0x02;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x02;      // no analog
  GPIO_PORTE_DEN_R |= 0x02;         // enable PE1
}

