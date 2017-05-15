/****************************************************************************
 Module
   Hardware.c

 Revision
   1.0.0

 Description
   This module contains all information for hardware - intialization and control of I/O pins,
	 ports, and interrupts on the DOG TIVA

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec     began conversion from TemplateFSM.c
 01/25/17	08:10	mch			write pindefs
****************************************************************************/
 
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/


//My Includes
#include "Hardware.h"
#include "Constants.h"




/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void InitGPIOPins(void);
void InitPorts(void);
void InitInterrupts(void);
void InitPWM(void);

/*---------------------------- Module Variables ---------------------------*/

/***************************PUBLIC FUNCTIONS********************************/
uint8_t ReadInput (int Port, uint8_t Pin) {
	if ((HWREG(Port + (GPIO_O_DATA + ALL_BITS)) & Pin) == Pin) {
		return HI;
	} else {
		return LO;
	}
}

void SetOutput (int Port, uint8_t Pin, uint8_t Value) {
	if (Value == HI) {
		HWREG(Port + (GPIO_O_DATA + ALL_BITS)) |= Pin;		
	} else {
		HWREG(Port + (GPIO_O_DATA + ALL_BITS)) &= ~Pin;
	}
}	


void InitPin (int Port, uint8_t Pin, uint8_t Direction) {
	//Enable digital I/O
	HWREG(Port + GPIO_O_DEN) |= Pin;
	//Direction
	if (Direction == OUTPUT) {
		HWREG(Port + GPIO_O_DIR) |= Pin; //output
	} else {
		HWREG(Port + GPIO_O_DIR) &= ~Pin; //input
	}		
}

void InitAll (void) {
	InitPorts();
	InitGPIOPins(); 
	InitInterrupts();
	InitPWM();
	//etc....
}

/*****************PRIVATE FUNCTIONS******************************************/


void InitPorts(void) {
	//ports
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_RCGCGPIO_R0);
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_RCGCGPIO_R1);
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R2) != SYSCTL_RCGCGPIO_R2);
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R3) != SYSCTL_RCGCGPIO_R3);
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R4) != SYSCTL_RCGCGPIO_R4);
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R5) != SYSCTL_RCGCGPIO_R5);
}

void InitGPIOPins(void) {

	//Lift fan PA2
	InitPin(LIFT_FAN_PORT, LIFT_FAN_PIN, LO);
	//initialize pins
  
}


void InitPWM(void) { 
	//pwm for 2 motors and servo
}




/*************INTERRUPTS*******************/

void InitInterrupts(void) {
	//interrupts in here?
}


//add in interrupts needed

