/****************************************************************************
 Module
   LiftFan_Service.c

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
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define EIGHTH_SEC (ONE_SEC/2)

#define BIT_TIME 1950

#define PERIOD_IN_MS 5
#define PWM_TICKS_PER_MS 40000/32
#define MICROS_PER_MS 1000
#define PERIOD_IN_MICROS 1000
#define BITS_PER_NIBBLE 4

#define ALL_BITS (0xff<<2)


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for ovehead
static ES_Event DeferralQueue[3+1];

static uint32_t DataCounter;
static bool LiftFan_State = false; //true = on
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
bool InitLiftFan_Service ( uint8_t Priority )
{
  
  MyPriority = Priority;
  
  ES_InitDeferralQueueWith( DeferralQueue, ARRAY_SIZE(DeferralQueue) );
	
	ES_ShortTimerInit(MyPriority, MyPriority);
	
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
bool PostLiftFan_Service( ES_Event ThisEvent )
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
ES_Event RunLiftFan_Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
  switch (ThisEvent.EventType){
		case ES_HOVER_ON :
			printf("turn hover on\r\n");
			LiftFan_State = false;
			DataCounter = 0;
			ES_ShortTimerStart(TIMER_A, BIT_TIME);
			break;
		
		case ES_HOVER_OFF :
			printf("turn hover off \n\r");
			LiftFan_State = true;
		  DataCounter = 0;
		  ES_ShortTimerStart(TIMER_A, BIT_TIME);
			break;
		
    case ES_SHORT_TIMEOUT :  
			if (ThisEvent.EventParam == TIMER_A) {
				switch (DataCounter) {
					case 0:
						//start bit, set low
						SetOutput(PIC_PORT, PIC_PIN, LO);
					  ES_ShortTimerStart(TIMER_A, BIT_TIME);
					break;
					
					case 1:
						//data bit
						//if hover is off, set hi
						if (LiftFan_State == false) {
							//turn on the fan
							SetOutput(PIC_PORT, PIC_PIN, HI);
							LiftFan_State = true;
						} else if (LiftFan_State == true) {
							//turn off the fan
							SetOutput(PIC_PORT, PIC_PIN, LO);
							LiftFan_State = false;
						}
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						
					break;
						
					case 2:
						//stop bit
						SetOutput(PIC_PORT, PIC_PIN, LO);
					break;
				}
				DataCounter++;
			}
      break;
  }
  return ReturnEvent;
}
