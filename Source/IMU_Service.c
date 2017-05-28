/****************************************************************************
 Module
   IMU_Service.c

 Revision
   1.0.1

 Description
   This is the service that handles communication with the IMU

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
#include "IMU_Service.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "ES_ShortTimer.h"

#include "Hardware.h"
#include "Constants.h"
#include "SPI.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define IMU_TIME 20 //ms

#define CTRL9_XL  0x18 
#define CTRL1_XL  0x10 
#define CTRL10_C  0x19 
#define CTRL2_G   0x11

#define OUTX_H_XL 0x29
#define OUTX_L_XL 0x28
#define OUTY_H_XL 0x2B
#define OUTY_L_XL 0x2A
#define OUTZ_H_XL 0x2D
#define OUTZ_L_XL 0x2C
#define OUTX_H_G 0x23
#define OUTX_L_G 0x22 
#define OUTY_H_G 0x25
#define OUTY_L_G 0x24
#define OUTZ_H_G 0x27
#define OUTZ_L_G 0x26

#define IMU_DATA_NUM_BYTES 12
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void Init_IMUhardware( void );


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static uint8_t IMUData[IMU_DATA_NUM_BYTES]; // array containing all bytes in IMU packet
static uint8_t ArrayIndex = 0;
static uint8_t *outgoingDataPacket; //pointer to data packet
static IMUState_t CurrentState;
static uint8_t Init_Bytes[8] = {(CTRL9_XL), 0x38, (CTRL1_XL), 0x60,
																(CTRL10_C), 0x38,(CTRL2_G), 0x60 };
static uint8_t DataRead_Bytes[IMU_DATA_NUM_BYTES] = { OUTX_H_XL, OUTX_L_XL, OUTY_H_XL,
															 OUTY_L_XL, OUTZ_H_XL, OUTZ_L_XL,    
															 OUTX_H_G, OUTX_L_G, OUTY_H_G, 
															 OUTY_L_G, OUTZ_H_G, OUTZ_L_G};
static uint8_t Init_Counter = 0;
static uint8_t Read_Counter = 0;
															 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTestHarnessService0

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitIMU_Service ( uint8_t Priority )
{  
  MyPriority = Priority;
	
	// Initialize SPI
	SPI_Init();
	// Initialize timer 
	ES_Timer_InitTimer(IMU_TIMER, IMU_TIME);
	
	CurrentState = Initializing_IMU;

	outgoingDataPacket = &IMUData[0];
 
  return true;
}

/****************************************************************************
 Function
     PostTestHarnessService0

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostIMU_Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTestHarnessService0

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event RunIMU_Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  IMUState_t NextState;
	
	switch( CurrentState ){
		case Initializing_IMU:
			if( (ThisEvent.EventType == ES_TIMEOUT) & (ThisEvent.EventParam == IMU_TIMER) ){
				printf("Writing first byte for SPI init\r\n");
				Write2IMU(Init_Bytes[Init_Counter]);
				Write2IMU(Init_Bytes[Init_Counter+1]);
				Enable_EOTInt();
				Init_Counter = Init_Counter+2;
				NextState = Initializing_IMU;
			}
			else if( ThisEvent.EventType == ES_EOT ){
				if( Init_Counter <= 6 ){
					printf("Writing other bytes for SPI init\r\n");
					Write2IMU(Init_Bytes[Init_Counter]);
					Write2IMU(Init_Bytes[Init_Counter+1]);
					Enable_EOTInt();
					Init_Counter = Init_Counter+2;
					NextState = Initializing_IMU;
				}
				else{
					NextState = Ready_IMU;
					ES_Timer_InitTimer(IMU_TIMER, IMU_TIME);
				}
			}
			break;
		case Ready_IMU:
			if( (ThisEvent.EventType == ES_TIMEOUT) & (ThisEvent.EventParam == IMU_TIMER) ){
				//printf("X acceleration data is %f\r\n",((double)(int16_t)((IMUData[0]<<8) | IMUData[1])/8000));
				//printf("Y acceleration data is %f\r\n",((double)(int16_t)((IMUData[2]<<8) | IMUData[3])/8000));
				//printf("Z acceleration data is %f\r\n",((double)(int16_t)((IMUData[4]<<8) | IMUData[5])/8000));
				Write2IMU( (DataRead_Bytes[Read_Counter] | BIT7HI) );
				Write2IMU( 0 );
				Enable_EOTInt();
				NextState = Ready_IMU;
			}
			else if( ThisEvent.EventType == ES_EOT ){
				if( Read_Counter < IMU_DATA_NUM_BYTES ){
					uint8_t Data = (HWREG(SSI1_BASE+SSI_O_DR) & SSI_DR_DATA_M); 
					IMUData[Read_Counter] = (HWREG(SSI1_BASE+SSI_O_DR) & SSI_DR_DATA_M); 
					Read_Counter++;
					Write2IMU( (DataRead_Bytes[Read_Counter] | BIT7HI) );
					Write2IMU( 0 );
					Enable_EOTInt();
					NextState = Ready_IMU;
				}
				else{
					uint8_t Data = (HWREG(SSI1_BASE+SSI_O_DR) & SSI_DR_DATA_M); 
					IMUData[Read_Counter] = (HWREG(SSI1_BASE+SSI_O_DR) & SSI_DR_DATA_M); 
					Read_Counter = 0;
					ES_Timer_InitTimer( IMU_TIMER, IMU_TIME );
					NextState = Ready_IMU;	
				}
			}
			break;
	}
	CurrentState = NextState;

  
  return ReturnEvent;
}



uint8_t* GetIMU_Data(void) {
	return outgoingDataPacket;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void Init_IMUhardware( void ){
	Write2IMU( (CTRL9_XL<<1) );
	Write2IMU( 0x38 );
	Write2IMU( (CTRL1_XL<<1) );
	Write2IMU( 0x60 );
	Write2IMU( (CTRL10_C<<1) );
	Write2IMU( 0x38 );
	Write2IMU( (CTRL2_G<<1) );
	Write2IMU( 0x60 );
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

