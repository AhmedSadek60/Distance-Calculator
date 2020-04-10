#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "inc/tm4c123gh6pm.h"



//#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
//#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
//#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value

//#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))


// port B macros
//#define GPIO_PORTB_DATA_BITS_R  ((volatile unsigned long *)0x40005000)
//#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
//#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
//#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
//#define GPIO_PORTB_PUR_R        (*((volatile unsigned long *)0x40005510))
//#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
//#define GPIO_PORTB_LOCK_R       (*((volatile unsigned long *)0x40005520))
//#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
//#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))

// port F macros
//#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
//#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
//#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
//#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
//#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
//#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
//#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
//#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
//#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))



// initialise port D
void portBInit(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000002;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTB_AMSEL_R = 0x00;    // disable analog function
	GPIO_PORTB_PCTL_R = 0x00000000;
	GPIO_PORTB_DIR_R = 0x02; // 0010 -> PB0: input/echo, PB1: output/trigger
	GPIO_PORTB_AFSEL_R = 0x00; // no alternate function
	GPIO_PORTB_DEN_R = 0x03; // 0011 -> enable digital pins for PB0, PB1

}

void portFInit(void){
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

// Initialize SysTick with busy wait running at bus clock.
void sysTickInit(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE + NVIC_ST_CTRL_CLK_SRC; // enable SysTick with core clock
}

// Time delay using busy wait.
// 16 MHz clock
void sysTickWait(unsigned long delay){
	NVIC_ST_RELOAD_R = delay - 1;				// number of counts to wait
	NVIC_ST_CURRENT_R = 0;
	while( (NVIC_ST_CTRL_R & 0x00010000) == 0 ) {} // wait for count flag
}

// Time delay using busy wait.
// This assumes 16 MHz system clock (PLL not ativated).
void sysTickWait10us(unsigned long delay){
  unsigned long i;
  for(i=0; i < delay; i++){
    sysTickWait(80);
  }
}

unsigned long measureDistance(void){

	unsigned long dist = 0;
	unsigned long count = 0;

	GPIO_PORTB_DATA_R &= ~ 0x02; // resetting trigger/PB1 to 0
	sysTickWait10us(1); // wait 20 us
	GPIO_PORTB_DATA_R |= 0x02; // setting trigger/PB1 to 1
	sysTickWait10us(1); // wait 10 us
	GPIO_PORTB_DATA_R &= ~ 0x02; // resetting trigger/PB1 to 0
//	sysTickWait10us(2); // wait 20 us

	while( (GPIO_PORTB_DATA_R & 0x01) == 0 ) {} // busy waiting if echo/PB0 = 0
	while( (GPIO_PORTB_DATA_R & 0x01) == 1 ){
		count += 10;
		sysTickWait10us(1);
	}

	dist = count * (float) 340.0 / 20000;  // distance =  time / 2 * speed = time in us * 10e-6 * 340 * 10e2 = time in us * 340/20000

	return dist;
}

// main program
int main(void) {

	portBInit();
	portFInit();
	sysTickInit();

	unsigned long distance = 0;

	while(1){

		distance = measureDistance();

//		printf("%ld", distance);

		if (distance < 10 )
			GPIO_PORTF_DATA_R = 0x04;   // BLUE LED on

		else
			GPIO_PORTF_DATA_R = 0x02;   // RED LED on

		sysTickWait10us(10000);   // delay 0.1 second


	}
	//return 0;
}
