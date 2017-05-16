/****************************************************************************
 Module
   DataService.c

 Revision
   1.0.1

 Description
   This is the service that generates data

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


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define EIGHTH_SEC (ONE_SEC/2)

#define BIT_TIME 1950

#define DATA GPIO_PIN_2

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
static bool LED_State = true;
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
bool InitDataService ( uint8_t Priority )
{
  ES_Event ThisEvent;
  
  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
	// initialize deferral queue for testing Deferal function
  ES_InitDeferralQueueWith( DeferralQueue, ARRAY_SIZE(DeferralQueue) );
	
		
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4 ) != SYSCTL_PRGPIO_R4);
	puts("\rDone initializing \r");
	
	HWREG( GPIO_PORTE_BASE + GPIO_O_DEN ) |= ( DATA );
	HWREG( GPIO_PORTE_BASE + GPIO_O_DIR ) |= ( DATA );
	HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
	
	
	ES_ShortTimerInit(MyPriority, MyPriority);
	
	printf("Initialized in DATA");
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
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
bool PostDataService( ES_Event ThisEvent )
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
ES_Event RunDataService( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
  switch (ThisEvent.EventType){
    case ES_INIT :
      printf("\rES_INIT received in Service %d\r\n", MyPriority);
      break;
		case ES_NEW_KEY :
			if (ThisEvent.EventParam == 'o'){
				LED_State = true;
				DataCounter = 0;
				ES_ShortTimerStart(TIMER_A, BIT_TIME);
			}
			break;
			//else if (ThisEvent.EventParam == 'd'){
		case ES_HOVER_ON :
			printf("hover on\r\n");
				LED_State = true;
				DataCounter = 0;
				ES_ShortTimerStart(TIMER_A, BIT_TIME);
			
		break;
    case ES_SHORT_TIMEOUT :  
			if (ThisEvent.EventParam == TIMER_A) {
				switch (DataCounter){
					case 0: //Start bit
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~( DATA ) ;
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						DataCounter++;	
					break;
					case 1: //Data bit 1 
						//if( LED_State ){
							HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
						//else{
						//	HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~( DATA );}
						
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						DataCounter++;
					break;
					case 2: //Data bit 2
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) &= ~( DATA ) ;
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						DataCounter++;
						
					//DataCounter++;
					break;
					case 3:	//Data bit 3
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
					//	ES_ShortTimerStart(TIMER_A, BIT_TIME);
					DataCounter=0;
					printf("data sent\r\n");
					break;
					/*case 4:  //Destination bit
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						DataCounter++;
					break;
					case 5: //Inversion bit
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
						ES_ShortTimerStart(TIMER_A, BIT_TIME);
						DataCounter++;
					break;
					case 6: //Set line low again
						HWREG( GPIO_PORTE_BASE + ( GPIO_O_DATA + ALL_BITS) ) |= ( DATA ) ;
						DataCounter = 0;
					break;*/
					default :
						break;
				}
			}
      break;
    default :
      break;
  }
  return ReturnEvent;
}
