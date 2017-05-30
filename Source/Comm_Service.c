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
#include "UART.h"

/*----------------------------- Module Defines ----------------------------*/
#define COMM_TEST_PRINTS

/*---------------------------- Module Functions ---------------------------*/
uint8_t CalculateChecksum (uint8_t FrameLength);
void ConstructIMUData (void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

static uint8_t* DataPacket_Rx;
static uint8_t DataPacket_Tx[MAX_PACKET_LENGTH];
static uint8_t DataFrameLength_Tx;
static uint8_t API_Ident;
static ES_Event DeferralQueue[3+1];

static uint8_t* IMU_Data;

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
  ES_InitDeferralQueueWith(DeferralQueue,3+1	);
  MyPriority = Priority;
	API_Ident = 0;
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
	/*
  if (ThisEvent.EventType == ES_DATAPACKET_RECEIVED) {
		printf("received a datapacket received in comm service \n\r");
	} else {
		printf("received a datapacket construct in comm service \r\n");
	}	*/	

  switch ( ThisEvent.EventType )
  {
    case ES_DATAPACKET_RECEIVED  : 
			
			  DataPacket_Rx	= GetDataPacket();
				/*for (int i = 0; i < 5; i++) {
					printf("%i \n\r", *(DataPacket_Rx + i));
				}*/
				API_Ident = GetAPIIdentifier(); //API_IDENT_BYTE_INDEX_RX);
		    printf("datapacket received in comm service: %i \n\r", API_Ident);
				if (API_Ident == API_IDENTIFIER_Rx) {
					  ES_Event NewEvent;
					  uint8_t PacketType = 0;
						DOGState_t CurrentState = GetDOGState();
						if (CurrentState == Paired) {
							PacketType = GetHeader();
						} else {
							PacketType = *(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX);
						}
						printf("RECEIVED A DATAPACKET (Comm_Service): %i \n\r", PacketType);
						switch (PacketType) {
							case FARMER_DOG_REQ_2_PAIR :
								printf("received REQ2PAIR\r\n");
								NewEvent.EventType = ES_PAIR_REQUEST_RECEIVED;
								break;
							case FARMER_DOG_ENCR_KEY :
								printf("received ENCR KEY \r\n");
								NewEvent.EventType = ES_ENCRYPTION_KEY_RECEIVED;
								break;
							case FARMER_DOG_CTRL:
								printf("received CMD\r\n");
								NewEvent.EventType = ES_NEW_CMD_RECEIVED;
								break;
							default:
								printf("Alternative PacketType = %i \n\r", PacketType);
								printf("Ask to Reset Encryption Key \n\r");
								TransmitResetEncryption();
							  ResetEncryptionKeyIndex();
							  ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
								break;
						}
						//printf("about to post to DOG SM \n\r");
						NewEvent.EventParam = ThisEvent.EventParam; //the frame length
						PostDOG_SM(NewEvent);
					} else if (API_Ident == API_IDENTIFIER_Tx_Result) { 
						#ifdef COMM_TEST_PRINTS
						printf("RECEIVED A TRANSMISSION RESULT DATAPACKET (Comm_Service) \n\r");
						#endif
						uint8_t TxStatusResult = *(DataPacket_Rx + TX_STATUS_BYTE_INDEX);
						if (TxStatusResult == SUCCESS) {
							//do nothing
						} else {
							//RESEND THE TX DATA PACKET
							ES_Event NewEvent;
							NewEvent.EventType = ES_START_XMIT;
							NewEvent.EventParam = DataFrameLength_Tx;
							PostTransmit_SM(NewEvent);
						}
					} else if (API_Ident == API_IDENTIFIER_Reset) {
						printf("Hardware Reset Status Message \n\r");
					}
    break;

    case ES_CONSTRUCT_DATAPACKET :
			    #ifdef COMM_TEST_PRINTS
					printf("--------------CONSTRUCTING-----------\n\r");
					#endif
		      //Header Construction
		      #ifdef COMM_TEST_PRINTS
					printf("Constructing Datapacket (Comm_Service) \n\r");
					#endif
		      DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER;
					DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00; 
					DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
					DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
					DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = GetPairedFarmerMSB(); //from DOG_SM
					DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = GetPairedFarmerLSB(); //from Dog_SM
					DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = OPTIONS;
					//Data Construction
					switch (ThisEvent.EventParam) {
						case DOG_ACK:
							//printf("Dog Ack Construction \n\r");
							//store the length of the data frame
						  DataFrameLength_Tx = ACK_N_ENCRYPT_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_ACK;
							//no extra data
							break;
						case DOG_FARMER_RESET_ENCR:
							//printf("Reset Encryption Construction \n\r");
						  //store the length of the data frame
						  DataFrameLength_Tx = ACK_N_ENCRYPT_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_FARMER_RESET_ENCR;
							//no extra data
							break;
						case DOG_FARMER_REPORT:
							//printf("Dog Report Construction \n\r");
						  //store the length of the data frame
						  DataFrameLength_Tx = STATUS_FRAME_LEN;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = DOG_FARMER_REPORT;
							//add in data from IMU SERVICE
						  IMU_Data = GetIMU_Data();
							ConstructIMUData();
							break;
					}
					
					
					//Add Unique Frame Length Data and Checksum
		      //add the unique frame length
					DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = DataFrameLength_Tx;
					//add checksum
					DataPacket_Tx[HEADER_LENGTH+DataFrameLength_Tx] = CalculateChecksum(DataFrameLength_Tx);
					
					//STart Xmit of data
					ES_Event NewEvent;
					NewEvent.EventType = ES_START_XMIT;
					NewEvent.EventParam = DataFrameLength_Tx;						
					 //param is length of entire data packet
					//Post NewEvent to transmit service
					PostTransmit_SM(NewEvent);
    break;

						default:
							break;
		
  }                                  // end switch on Current State

  return ReturnEvent;
}

/*******HELPER FUNCTIONS**********/
uint8_t CalculateChecksum (uint8_t FrameLength) {
	uint8_t RunningSum = 0;
	uint8_t Checksum = 0;
	uint8_t numBytes = FrameLength + HEADER_LENGTH;
	for (int i = HEADER_LENGTH; i < numBytes; i++) {
		RunningSum += DataPacket_Tx[i];
	}
	Checksum = 0xFF - RunningSum;
	
	//printf("Running sum: %i\r\n", RunningSum);
	//printf("Check sum: %i\r\n", Checksum);
	return Checksum;
}

void ConstructIMUData (void) {
	uint8_t DataPacketIndex = PACKET_TYPE_BYTE_INDEX_TX + 1;
	for (int i = 0; i < IMU_DATA_NUM_BYTES; i++) {
		DataPacket_Tx[DataPacketIndex + i] = *(IMU_Data + i);
	}
}


/******GETTER FUNCTIONS************/
uint8_t* GetDataPacket_Tx (void) {
  return &DataPacket_Tx[0];
}


