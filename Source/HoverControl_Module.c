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

/*------------------------------ Module Code ------------------------------*/


void ActivateHover(void) {
  ES_Event ThisEvent;
	ThisEvent.EventType = ES_HOVER_ON;
	PostLiftFan_Service(ThisEvent);
}

void DeactivateHover(void) {
  ES_Event ThisEvent;
	ThisEvent.EventType = ES_HOVER_OFF;
	PostLiftFan_Service(ThisEvent);
	
	//Also Deactivate Thrust Fans
}


void ActivateTurning(uint8_t Turning) {
}


void ActivateDirectionSpeed(uint8_t DirectionSpeed) {
} 

void ActivatePeripheral(uint8_t Peripheral) {
}

void ActivateBrake(uint8_t Brake) {
}



