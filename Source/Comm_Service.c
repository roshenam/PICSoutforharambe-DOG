/****************************************************************************
 Module
   Comm_Service.c

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

/*----------------------------- Module Defines ----------------------------*/



/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

static uint8_t* DataPacket_Rx;
static uint8_t DataPacket_Tx[40];


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitComm_Service

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

  return true;
}

/****************************************************************************
 Function
    PostComm_Service

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
bool PostComm_Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunComm_Service

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
ES_Event RunComm_Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	

  switch ( ThisEvent.EventType )
  {
    case ES_DATAPACKET_RECEIVED  : 
			  DataPacket_Rx	= GetDataPacket();
				uint8_t API_Ident = *(DataPacket_Rx + API_IDENT_BYTE_INDEX_RX);
				if (API_Ident == API_IDENTIFIER_Rx) {
						ES_Event NewEvent;
						uint8_t PacketType = *(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX);
						switch (PacketType) {
							case FARMER_DOG_REQ_2_PAIR :
								NewEvent.EventType = ES_PAIR_REQUEST_RECEIVED;
								break;
							case FARMER_DOG_ENCR_KEY :
								NewEvent.EventType = ES_ENCRYPTION_KEY_RECEIVED;
								break;
							case FARMER_DOG_CTRL :
								NewEvent.EventType = ES_NEW_CMD_RECEIVED;
								break;
							case FARMER_DOG_RESET_ENCR :
                NewEvent.EventType = ES_ENCRYPTION_COUNTER_INCORRECT;
								break;
						}
						NewEvent.EventParam = ThisEvent.EventParam; //the frame length
						PostDOG_SM(NewEvent);
					} else if (API_Ident == API_IDENTIFIER_Tx_Result) {
						//check if the message is a success = 0, no ACK = 1, CCA failure = 2, Purged = 3
						//if not 0, resend the message
					}
    break;

    case ES_CONSTRUCT_DOG_ACK :
					DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER;
					DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00;
					DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = PAIR_ACK_FRAME_LENGTH;
				  DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
					DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
					DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = PairedFarmer_MSB;
					DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = PairedFarmer_LSB;
					DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = 0x00;
					DataPacket_Tx[PACKET_TYPE_BYTE_INDEX] = DOG_FARMER_IDENTIFICATION;
					ES_Event NewEvent;
					NewEvent.EventType = ES_SEND_DOG_ACK;
					NewEvent.EventParam = &DataPacket_Tx[0]; //param is pointer to data packet
					//Post NewEvent to transmit service
    break;

		
  }                                  // end switch on Current State

  return ReturnEvent;
}

/******GETTER FUNCTIONS************/
uint8_t* GetDataPacket_Tx (void) {
  return &DataPacket_Tx[0];
}


