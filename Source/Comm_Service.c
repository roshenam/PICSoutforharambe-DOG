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
bool InitComm_Service( uint8_t Priority )
{

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
					  printf("RECEIVED A DATAPACKET (Comm_Service) \n\r");
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
						}
						NewEvent.EventParam = ThisEvent.EventParam; //the frame length
						PostDOG_SM(NewEvent);
					} else if (API_Ident == API_IDENTIFIER_Tx_Result) { 
						printf("RECEIVED A TRANSMISSION RESULT DATAPACKET (Comm_Service) \n\r");
						uint8_t TxStatusResult = *(DataPacket_Rx + TX_STATUS_BYTE_INDEX);
						if (TxStatusResult == SUCCESS) {
							//do nothing
						} else {
							//RESEND THE TX DATA PACKET -- ADD CODE IN HERE
						}
					} else if (API_Ident == API_IDENTIFIER_Reset) {
						printf("Hardware Reset Status Message \n\r");
					}
    break;

    case ES_CONSTRUCT_DATAPACKET :
					printf("--------------CONSTRUCTING-----------\n\r");
					ES_Event NewEvent;
					//Header Construction
					printf("Constructing Datapacket (Comm_Service) \n\r");
					DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER;
					DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00; 
					DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
					DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
					DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = 0x21; //GetPairedFarmer_MSB() DONT HARDCODE;
					//uint8_t PairedFarmer_LSB = GetPairedFarmerLSB();
					DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = 0x8B; //GetPairedFarmer_LSB() DONT HARDCODE;
					DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = OPTIONS;
					switch (ThisEvent.EventParam) {
						case DOG_ACK:
							printf("Dog Ack Construction \n\r");
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = ACK_N_ENCRYPT_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_ACK;
							// add check sum
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+1] = 0x4F;
						  //set the frame length as event param
							NewEvent.EventParam = ACK_N_ENCRYPT_FRAME_LEN;						
							break;
						case DOG_FARMER_RESET_ENCR:
							printf("Reset Encryption Construction \n\r");
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = ACK_N_ENCRYPT_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_FARMER_RESET_ENCR;
							// add check sum
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+1] = 0x52;
						  //set the frame length as event param
							NewEvent.EventParam = ACK_N_ENCRYPT_FRAME_LEN;	
							break;
						case DOG_FARMER_REPORT:
							printf("Dog Report Construction \n\r");
								//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = STATUS_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_FARMER_REPORT;
							//add in data from IMU SERVICE
							// add check sum
							//DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+13] = ???; NEED TO CALCULATE THE CHECKSUM CORRECTLY!!!
						  //set the frame length as event param
							NewEvent.EventParam = STATUS_FRAME_LEN;	
							break;
					}
		
					NewEvent.EventType = ES_START_XMIT;
					 //param is length of entire data packet
					//Post NewEvent to transmit service
					PostTransmit_SM(NewEvent);
    break;

		
  }                                  // end switch on Current State

  return ReturnEvent;
}

/******GETTER FUNCTIONS************/
uint8_t* GetDataPacket_Tx (void) {
  return &DataPacket_Tx[0];
}


