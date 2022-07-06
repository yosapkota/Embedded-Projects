//#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "ADCSWTrigger.h"
#include "PLL.h"
#include <stdint.h>

//all pins of Port B
#define OUTPUT    (*((volatile unsigned long*) 0x400053FC))
	

// LEDs RED, BLUE and GREEN on PF1, PF2 and PF3 respectively
#define RED_LED         				(*((volatile unsigned long *)0x40025008))
#define BLUE_LED         				(*((volatile unsigned long *)0x40025010))
#define GREEN_LED         			(*((volatile unsigned long *)0x40025020))
#define SANE                    (*((volatile unsigned long *)0x4002400C))
	

//2. Declaration Section
volatile unsigned long value;
volatile unsigned long xPos;
volatile unsigned long yPos;
volatile unsigned long ADCvalue;
//volatile float result;
const unsigned int numGridLines = 4;
const unsigned long posGridLine[numGridLines] = { 100, 1900, 2195, 3995 };
#define STEPPER_1         			(*((volatile unsigned long *)0x4000503C))
#define STEPPER_2         			(*((volatile unsigned long *)0x400053C0))
const unsigned int STEP[32] = {0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09, 0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09, 0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09, 0x08, 0x0c, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09};

const int velGrid[numGridLines+1][numGridLines+1][2] = {
      {{ 0, 2},{ 1, 2},{ 2, 2},{ 2, 1},{ 2, 0}},
      {{-1, 2},{ 0, 1},{ 1, 1},{ 1, 0},{ 2,-1}},
      {{-2, 2},{-1, 1},{ 0, 0},{ 1,-1},{ 2,-2}},
      {{-2, 1},{-1, 0},{-1,-1},{ 0,-1},{ 1,-2}},
      {{-2, 0},{-2,-1},{-2,-2},{-1,-2},{ 0,-2}}
    };

//*******
// initialization function
///****
void stepMotors(int, int);
unsigned long ADC0_xPos(void);
void PortF_Init(void); 
void PortE_Init(void);
void PortB_Init(void);




/*SysTick Timer*/
void SysTick_Init(void);
void SysTick_wait(unsigned long);
void SysTick_Wait1ms(unsigned long);

//uint32_t result[2];

int main(void)
{
  //int i, j;
  //unsigned int grid[lenGridLine][lenGridLine][2];
  volatile int i, j, k, xVal, yVal;
  volatile int lVelocity, rVelocity;
	PLL_Init();   
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1
	PortF_Init();  												// PortF initialization
	PortE_Init();  												// PortE initialization
	PortB_Init();
	SysTick_Init();
	/*
	//BLUE_LED  = 0x04;
  for(i=0; i<lenGridLine; i++) {
    for(j=0; j<lenGridLine; j++) {
      grid[i][j][0] = gridLine[i];
      grid[i][j][1] = gridLine[j];
    }
  }
  */
	while(1)
	{	
		//reading value from x-axis  and y-axis motion of the joystick
		value = ADC0_Reading();
		xPos = value >> 12;
		yPos = value & ~(0xFFFFF000);
    xVal = 4;
    yVal = 4;
      
    for(i=numGridLines-1; i>=0; i--) {
      for(j=numGridLines-1; j>=0; j--) {
        //grid[i][j][0] = gridLine[i];
        //grid[i][j][1] = gridLine[j];
        if(xPos < posGridLine[i]){
          xVal = i;
        }
        if(yPos < posGridLine[j]){
          yVal = j;
        }
      }
    }
    lVelocity = velGrid[xVal][yVal][0];
    rVelocity = velGrid[xVal][yVal][1];
    
    if(!SANE) {
      BLUE_LED  = 0x04;
      
      lVelocity = (int)(xPos & 0x03);
      rVelocity = (int)(yPos & 0x03);
      for(k=0; k<50; k+=2) {
        stepMotors(lVelocity-1,rVelocity-1);
      }
      
    } else {
      BLUE_LED  = 0x00;
      stepMotors(lVelocity,rVelocity);
    }

  }	
		
    //ADCvalue = ADC0_InSeq3();	
		
		/*
		Forward Motion --> Full step  and above (higher speed)
									 --> Half step  to    (slower speed)
		Stop           -->  to 
		Backward Motion--> Half step  to     (slower speed)
									 --> Full step  and below  (higher speed)
		
		*/
			

}

void stepMotors(int step1Speed, int step2Speed) {
  int i;
  for (i=0; i<8; i++) {
    STEPPER_1 = STEP[15+(i*step1Speed)];
    STEPPER_2 = STEP[15+(-i*step2Speed)]<<4;
    SysTick_Wait1ms(2);
  }
}


void PortF_Init(void)
{
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0

}

void PortE_Init(void) {
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;   // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;         //    allow time for clock to stabilize
  GPIO_PORTE_DIR_R &= ~0x01;      // 2) make PE2,PE1 input
  GPIO_PORTE_AMSEL_R &= ~0x01; // 3) disable analog function on PE0
  GPIO_PORTE_PCTL_R &= ~0x0000000F; // 4) enable regular GPIO 
  GPIO_PORTE_DIR_R &= ~0x01;    // 5) inputs on PE0
  GPIO_PORTE_AFSEL_R &= ~0x01; // 6) regular function on PE0
  GPIO_PORTE_PUR_R = 0x01;          // enable pullup resistors on PE0
  GPIO_PORTE_DEN_R |= 0x01;    // 7) enable digital on PE0
}


/*Stepper Motor*/

//Port B Initialization
void PortB_Init(void) {
  volatile unsigned long delay; 
  SYSCTL_RCGC2_R |= 0x02;      // 1) B 
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock 
  GPIO_PORTB_AMSEL_R &= ~0xFF; // 3) disable analog function on PB7-0 
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO 
  GPIO_PORTB_DIR_R |= 0xFF;    // 5) outputs on PB7-0 
  GPIO_PORTB_AFSEL_R &= ~0xFF; // 6) regular function on PB7-0 
  GPIO_PORTB_DEN_R |= 0xFF;    // 7) enable digital on PB7-0 
  
}


// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}
// Time delay using busy wait.
// The delay parameter is in units of the core clock. 
void SysTick_Wait(unsigned long delay){
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}
// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait1ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(80000);  // wait 1ms (assumes 80 MHz clock)
  }
}

