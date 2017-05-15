/****************************************************************************
 Module
   DOG_SM.c

 Description
   This state machine was created by the communications committee for all the hovercrafts 
	 to implement to facilitate bug-free interoperability. It handles the decision making structure 
	 for incoming data packets and actuates the hovercraft's peripherals accordingly.

 History
 When           Who     What/Why
 -------------- ---     --------
 05/14/2017			MCH
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "Hardware.h"
#include "Constants.h"
#include "DOG_SM.h"

/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
uint8_t GetPairedFarmerLSB(void);
uint8_t GetPairedFarmerMSB(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static DOGState_t CurrentState;

static uint8_t DogTag = 0;

static uint8_t* DataPacket_Rx;

static uint8_t PairedFarmer_MSB;
static uint8_t PairedFarmer_LSB;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDOG_SM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Mihika Hemmady
****************************************************************************/
bool InitDOG_SM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
  // put us into the Initial PseudoState
  CurrentState = Waiting2Pair;

  //how to get dog tag number?
	
	InitAll(); //initialize all hardware (ports, pins, interrupts)
	
	
  return true;
}

/****************************************************************************
 Function
     PostDOG_SM

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Mihika Hemmady
****************************************************************************/
bool PostDOG_SM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunDOG_SM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Mihika Hemmady
****************************************************************************/
ES_Event RunDOG_SM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	DOGState_t NextState;

  switch ( CurrentState )
  {
    case Waiting2Pair : 
				if (ThisEvent.EventType == ES_PAIR_REQUEST_RECEIVED) {
          DataPacket_Rx = GetDataPacket();
          uint8_t DogTagReq = *(DataPacket_Rx + DOG_TAG_BYTE_INDEX);
          if (DogTagReq == DogTag) {
            PairedFarmer_MSB = *(DataPacket_Rx + SOURCE_ADDRESS_MSB_INDEX);
            PairedFarmer_LSB = *(DataPacket_Rx + SOURCE_ADDRESS_LSB_INDEX);
            
            //Transmit an 0x02 PAIR_ACK message to FARMER
            ES_Event NewEvent;
						NewEvent.EventType = ES_CONSTRUCT_DOG_ACK;
						//PostComm_Service(NewEvent); add back in when included
            
            //turn the Lift Fan On
            ActivateHover();
            NextState = Paired_Waiting4Key;
          }
        }						
    break;

    case Paired_Waiting4Key:      
				//if the encryption key 0x03 is received
					//store the encruption key
					//restart the 1s transmit timer	
					//NextState == Paired
				//if gametimer timeout or 1s transmit timer timeout
					//deactivate the 1s transmit timer timeout
					//turn TREAT off
					//Disable the 1s transmit timer
					//NextState == Waiting2Pair
      
    break;

		case Paired:   
				//if new command received 0x04 header
					//restart 1s transmit timeout timer
					//execute commands
					//transmit status 0x00
				//if encyrption counter incorrect 0x05
					//reset the encryption counter
		//if game timer timeout or 1s transmit timer timeout
					//deactivate 1s transmit timer timeout
					//TREAT fan off
					//disable 1s transmit timer
					//nextState == Waiting2Pair
    break;
		
  }                                  // end switch on Current State
  CurrentState = NextState;
  return ReturnEvent;
}

/*******Getters************/
uint8_t GetPairedFarmerLSB (void) {
	return PairedFarmer_LSB;
}

uint8_t GetPairedFarmerMSB (void) {
	return PairedFarmer_MSB;
}

