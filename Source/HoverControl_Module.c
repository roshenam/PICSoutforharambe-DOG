/****************************************************************************
 Module
   HoverControl_Module.c

 Description
   Controls for lift fan and propellers

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
05/13/2017			MH	
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "Constants.h"
#include "Hardware.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Variables ---------------------------*/

static bool IsLiftFanOn = false;
static uint8_t AverageDuty;
static uint8_t Polarity;
static uint8_t RightDuty;
static uint8_t LeftDuty;

/*---------------------------- Module Functions ---------------------------*/
void CalculateAverageDuty(uint8_t DirectionBit);
void CalculateRequestedDuties(uint8_t DifferentialCalculated, bool IsTurningLeft);

/*------------------------------ Module Code ------------------------------*/


void ActivateHover(void) {
  ES_Event ThisEvent;
	ThisEvent.EventType = ES_HOVER_ON;
	PostLiftFan_Service(ThisEvent);
	IsLiftFanOn = true;
}

void DeactivateHover(void) {
  ES_Event ThisEvent;
	ThisEvent.EventType = ES_HOVER_OFF;
	PostLiftFan_Service(ThisEvent);
	IsLiftFanOn = false;
	
	//Also Deactivate Thrust Fans
	SetDuty(OFF, PWM_FORWARD_POL, MOTOR_LEFT_PWM);
	SetDuty(OFF, PWM_FORWARD_POL, MOTOR_RIGHT_PWM);
}

void ActivateDirectionSpeed(uint8_t DirectionSpeed, uint8_t Turning) {
	//find average duty and forward/reverse polarity
	CalculateAverageDuty(DirectionSpeed);
	printf("AverageDuty: %i      Polarity (0 is forward): %i \n\r", AverageDuty, Polarity);
	
	uint8_t Differential = 0;
	/*
	//find duty differential between the two motors for turning action
	uint8_t Differential;
	
	if (Turning == 127) {
		Differential = 0;
		LeftDuty = AverageDuty;
		RightDuty = AverageDuty;
	} else if (Turning > 127) {
		//max leftward control
		Differential = ((Turning-127)*100) / (255-127);
		Differential = (Differential/MAX_TURNING_DIFF_DIVISOR)/2;
		CalculateRequestedDuties(Differential, true);
	} else { //turn right
		Differential = AverageDuty = ((DirectionSpeed)*100) / (127);
		Differential = (Differential/MAX_TURNING_DIFF_DIVISOR)/2;
		CalculateRequestedDuties(Differential, false);
	}
	*/
	
	printf("Differential (max 30): %i     RightDuty: %i     LeftDuty: %i\n\r", Differential, RightDuty, LeftDuty);
	
	//Set the duty and direction
	SetDuty(LeftDuty, Polarity, MOTOR_LEFT_PWM);
	SetDuty(RightDuty, Polarity, MOTOR_RIGHT_PWM);
} 

void ActivatePeripheral(uint8_t Peripheral) {
}

void ActivateBrake(uint8_t Brake) {
	if ((Brake == OFF) && (IsLiftFanOn == false)) {
		ActivateHover();
	} else if ((Brake == ON) && (IsLiftFanOn == true)) {
		DeactivateHover();
	}
}

/********PRIVATE FUNCTIONS**************/

void CalculateAverageDuty(uint8_t DirectionBit) {
	if (DirectionBit == 127) {
		AverageDuty = OFF;
		Polarity = PWM_FORWARD_POL;
	} else if (DirectionBit > 127) {
		AverageDuty = ((DirectionBit-127)*100) / (255-127);
		Polarity = PWM_FORWARD_POL;
	} else {
		AverageDuty = ((DirectionBit)*100) / (127);
		AverageDuty = 100 - AverageDuty;
		Polarity = PWM_REVERSE_POL;
	}
}

void CalculateRequestedDuties(uint8_t DifferentialCalculated, bool IsTurningLeft) {
	if (IsTurningLeft == true) {
		if (AverageDuty < DifferentialCalculated) {
			LeftDuty = 0;
		} else {
			LeftDuty = AverageDuty - DifferentialCalculated;
		}
		
		//calcaulate right differential
		if (AverageDuty > DifferentialCalculated) {
			RightDuty = 100;
		} else {
			RightDuty = AverageDuty + DifferentialCalculated;
		}
	} else {
		if (AverageDuty < DifferentialCalculated) {
			RightDuty = 0;
		} else {
			RightDuty = AverageDuty - DifferentialCalculated;
		}
		
		//calcaulate right differential
		if (AverageDuty > DifferentialCalculated) {
			LeftDuty = 100;
		} else {
			LeftDuty = AverageDuty + DifferentialCalculated;
		}
	}	
}





