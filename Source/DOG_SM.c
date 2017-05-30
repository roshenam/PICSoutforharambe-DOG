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
void StoreEncryptionKey(void);
void TransmitStatus(void);
void TransmitAck(void);
//void TransmitResetEncryption(void);
void StopWagging(void);
void StartWagging(void);
void InitDogTag(void);
void DecodeCommandMessage(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static DOGState_t CurrentState;

static uint8_t DogTag = 0;
static uint32_t ADResults[1];
static uint32_t CurrentDogTagVal;
static double MaxVoltage = 3.3;

static uint8_t* DataPacket_Rx;
static uint8_t  DecryptedFarmerCommands[FARMER_CMD_LENGTH]; //header and data bytes

static uint8_t PairedFarmer_MSB;
static uint8_t PairedFarmer_LSB;

static uint8_t EncryptionKey[ENCRYPTION_KEY_LENGTH];
static uint8_t EncryptionKey_Index = 0;

static uint8_t DirectionSpeed;
static uint8_t Turning;
static uint8_t Brake;
static uint8_t Peripheral;


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
  //ES_Event ThisEvent;

  MyPriority =  Priority;
	
  // put us into the Initial PseudoState
  CurrentState = Waiting2Pair;

//GET DOG TAG NUMBER
	//InitDogTag();
	//DogTag = GetDogTag();
	DogTag = 39;
	printf("DOGTAG#: %i \n\r", DogTag);	
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

	/*
	printf("Event was posted \n\r");
  if (CurrentState == Waiting2Pair) {
		printf("Waiting 2 Pair \n\r");
	} else if (CurrentState == Paired_Waiting4Key) {
		printf("Paired_Waiting4Key \n\r");
	} else if (CurrentState == Paired) {
		printf("Paired \n\r");
	} else {
		printf("THERES A FOURTH STATE! \n\r");
	} */
	
	switch ( CurrentState )
  {
    case Waiting2Pair : 
			printf("waiting 2 pair state \n\r");
			
				//initialize encryption key index
				EncryptionKey_Index = 0;
		
		   //stop wagging
				StopWagging();
		
				if (ThisEvent.EventType == ES_PAIR_REQUEST_RECEIVED) {
          DataPacket_Rx = GetDataPacket();
          uint8_t DogTagReq = *(DataPacket_Rx + DOG_TAG_BYTE_INDEX);
          if (DogTagReq == DogTag) {
            PairedFarmer_MSB = *(DataPacket_Rx + SOURCE_ADDRESS_MSB_INDEX);
            PairedFarmer_LSB = *(DataPacket_Rx + SOURCE_ADDRESS_LSB_INDEX);
            
            //Transmit an 0x02 PAIR_ACK message to FARMER
            TransmitAck();
            
            //turn the Lift Fan On
            ActivateHover();
												
						//start the lost-communications timer for 1s
						ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
						
            NextState = Paired_Waiting4Key;
          } 
        } else {
						NextState = Waiting2Pair;
				} 
    break;

    case Paired_Waiting4Key: 
				printf("Paired Waiting4Key \n\r");
				if (ThisEvent.EventType == ES_ENCRYPTION_KEY_RECEIVED) {
					DataPacket_Rx = GetDataPacket();
					StoreEncryptionKey();
					//start the lost-communications timer for 1s
					ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
					
					//transmit status
					TransmitStatus();
					
					//Start Tail Wag Service!!
					StartWagging();
					
					NextState = Paired;
				} else if ((ThisEvent.EventType == ES_UNPAIR) || (ThisEvent.EventType == ES_TIMEOUT 
																						&& ThisEvent.EventParam == LOST_COMM_TIMER)) {
						printf("Lost Comm Timer Timeout \n\r");
						DeactivateHover();
						NextState = Waiting2Pair;
						StopWagging();
				} else {
					NextState = Paired_Waiting4Key;
				}					
    break;

		case Paired:
				printf("Paired \n\r");
				if ( ThisEvent.EventType == ES_NEW_CMD_RECEIVED) {
					DataPacket_Rx = GetDataPacket();
					//start the lost-communications timer for 1s
					ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
          //decode
					DecodeCommandMessage();
				
					//executed commands as required
						DirectionSpeed = DecryptedFarmerCommands[1];
						//printf("DECRYPTED Direction/Speed: %i \n\r", DirectionSpeed);
						Turning = DecryptedFarmerCommands[2];
						//printf("DECRYPTED Turning: %i \n\r", Turning);
						//printf("DECRYPTED Digital: %i \n\r", DecryptedFarmerCommands[3]);
						switch ((DecryptedFarmerCommands[3] & DIGITAL_MASK)) {
							case 0x00:
								Peripheral = OFF;
								Brake = OFF;
								break;
							case 0x01:
								Peripheral = ON;
								Brake = OFF;
								break;
							case 0x02:
								Peripheral = OFF;
								Brake = ON;
								break;
							case 0x03:
								Peripheral = ON;
								Brake = ON;
								break;
							default:
								Peripheral = OFF;
								Brake = OFF;
							break;
						}
						//printf("DECRYPTED Peripheral: %i \n\r", Peripheral);
						//printf("DECRYPTED Brake: %i \n\r", Brake);
						ActivateDirectionSpeed(DirectionSpeed, Turning);
						ActivatePeripheral(Peripheral);
						ActivateBrake(Brake);
						
						//tranmsit status message
						TransmitStatus();
/*
					} else {
						printf("Reset the encryption key \n\r");
						//reset encryption
						TransmitResetEncryption();
						//reset the encryption key
						EncryptionKey_Index = 0;
						
						ES_Timer_InitTimer(LOST_COMM_TIMER, LOST_COMM_TIME);
					} */
					
					NextState = Paired;
				} else if ((ThisEvent.EventType == ES_UNPAIR) || (ThisEvent.EventType == ES_TIMEOUT 
																						&& ThisEvent.EventParam == LOST_COMM_TIMER)) {
						printf("Lost Comm Timer Timeout \n\r");
						DeactivateHover();
						NextState = Waiting2Pair;
						StopWagging();
				} else {
					NextState = Paired;
				}					

    break;
		
  }                                  // end switch on Current State
  CurrentState = NextState;
  return ReturnEvent;
}

/******Helper function******/
void StoreEncryptionKey(void) {
	uint16_t DataIndex = PACKET_TYPE_BYTE_INDEX_RX + 1;
	for (int i = 0; i < ENCRYPTION_KEY_LENGTH; i++) {
		EncryptionKey[i] = *(DataPacket_Rx + DataIndex + i);
	}
}

uint8_t GetHeader(void){
	uint8_t localindex = EncryptionKey_Index;
	uint8_t local_decrypted = 0; 
	local_decrypted = EncryptionKey[localindex] ^ (*(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX));
	return local_decrypted;
}
	

void DecodeCommandMessage(void) {
	for (int i = 0; i < FARMER_CMD_LENGTH; i++) {
		DecryptedFarmerCommands[i] = EncryptionKey[EncryptionKey_Index] ^ (*(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX + i));
		if (EncryptionKey_Index < 31) {
				EncryptionKey_Index++;
		} else {
			EncryptionKey_Index = 0;
		}
	}
} 

void InitDogTag(){
		ADC_MultiInit(1); // initializes PE0 as analog input 
		ADC_MultiRead(ADResults);
		CurrentDogTagVal = ADResults[0];
		//printf("analog: %i\r\n",CurrentDogTagVal);
		if(CurrentDogTagVal < 2100){
			DogTag = 1;
		}else if (CurrentDogTagVal > 2100 & CurrentDogTagVal < 2800){
			DogTag = 2;
		}else if (CurrentDogTagVal > 2800){
			DogTag = 3;
		}
}

void TransmitStatus(void) {
	 //Transmit an 0x00 DOG STATUS message to FARMER
		ES_Event NewEvent;
		NewEvent.EventType = ES_CONSTRUCT_DATAPACKET;
		NewEvent.EventParam = DOG_FARMER_REPORT;
		PostComm_Service(NewEvent); //add back in when included
}

void TransmitAck(void) {
	  ES_Event NewEvent;
		NewEvent.EventType = ES_CONSTRUCT_DATAPACKET;
		NewEvent.EventParam = DOG_ACK;
		PostComm_Service(NewEvent); //add back in when included
}

void TransmitResetEncryption(void) {
	  ES_Event NewEvent;
		NewEvent.EventType = ES_CONSTRUCT_DATAPACKET;
		NewEvent.EventParam = DOG_FARMER_RESET_ENCR;
		PostComm_Service(NewEvent); //add back in when included
}

void ResetEncryptionKeyIndex(void) {
	EncryptionKey_Index	= 0;
}

void StopWagging(void) {
	  ES_Event NewEvent;
		NewEvent.EventType = ES_STOP_WAGGING;
		PostDogTail_Service(NewEvent); //add back in when included
}

void StartWagging(void) {
	  ES_Event NewEvent;
		NewEvent.EventType = ES_START_WAGGING;
		PostDogTail_Service(NewEvent); //add back in when included
}

/*******Getters************/
uint8_t GetPairedFarmerLSB (void) {
	return PairedFarmer_LSB;
}

uint8_t GetPairedFarmerMSB (void) {
	return PairedFarmer_MSB;
}

uint8_t GetDogTag(void){
	return DogTag;
}

DOGState_t GetDOGState(void) {
	return CurrentState;
}
