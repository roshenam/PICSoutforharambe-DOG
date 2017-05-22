/****************************************************************************
 Module
   DogTail_Service.c

 Revision
   1.0.1

 Description
   This is the service that controls the PIC to the lift fan

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "DataService.h"
#include "termio.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#include "ES_ShortTimer.h"

#include "Hardware.h"
#include "Constants.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void DecidePosition(void); 


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static bool Position = true;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDataService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
			R. MacPherson 1/13/2017 
****************************************************************************/
bool InitDogTail_Service ( uint8_t Priority )
{
  
  MyPriority = Priority;
  SendServoHome();
	Position = true;
	
  return true;
}

/****************************************************************************
 Function
     PostADService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     R. MacPherson, 1/13/2017
****************************************************************************/
bool PostDogTail_Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunADService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Reads changes in potentiometer and notifies motor service of speed change
 Notes
   
 Author
   R. MacPherson, 1/13/2017
****************************************************************************/
ES_Event RunDogTail_Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
  switch (ThisEvent.EventType){
		case ES_STOP_WAGGING :
			printf("turn wagging off\r\n");
			break;
		
		case ES_START_WAGGING :
			printf("turn wag on \n\r");
			DecidePosition();
		  ES_Timer_InitTimer(WAG_TIMER, WAG_TIME);
			break;
		
    case ES_TIMEOUT :  
			if (ThisEvent.EventParam == WAG_TIMER) {
				DecidePosition(); 
				ES_Timer_InitTimer(WAG_TIMER, WAG_TIME);
			}
      break;
  }
  return ReturnEvent;
}

/***helpers***/
void DecidePosition (void) {
	if (Position == true) {
				SendServoAway();
				Position = false;
			} else {
				SendServoHome();
				Position = true;
			}
}
