/****************************************************************************
 Module
   UART.c

 Description
   UART Initialization and ISR functions

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
05/13/2017			SC	
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
#include "Transmit_SM.h"
#include "Receive_SM.h"
#include "UART.h"

/*----------------------------- Module Defines ----------------------------*/
// UART7 Rx: PE0
// UART7 Tx: PE1
#define RECEIVE_TIMER_LENGTH 10 // based off of 9600 baud rate (each character takes ~1.04ms to send)

/*---------------------------- Module Variables ---------------------------*/
static uint8_t DataByte; 
static UARTReceiveState_t CurrentState;
static uint8_t LocalDataPacket[MAX_PACKET_LENGTH];
static uint8_t ReadyDataPacket[MAX_PACKET_LENGTH];

static uint8_t MSBLength = 0;
static uint8_t LSBLength = 0;
static uint8_t FrameLength = 0; // num bytes in data frame
static uint8_t BytesLeft = 0;
static uint8_t RunningSum = 0;
static uint8_t ArrayIndex_UART = 0;

static uint8_t API_Identifier = 0;

/*---------------------------- Module Function ---------------------------*/
static void ProcessByte(uint8_t DataByte);
static void CopyDataPacket(void);

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitUART

 Description
     Initializes UART7 pins, GPIO pins, and UART interrupts

 Author
     Sarah Cabreros
****************************************************************************/
void InitUART(void) {

	// 1. Enable clock to UART module using RCGCUART register
	HWREG(SYSCTL_RCGCUART) |= SYSCTL_RCGCUART_R7;
	
	// 2. Wait for the UART to be ready (PRUART)
	while ((HWREG(SYSCTL_PRUART) & SYSCTL_PRUART_R7 ) != SYSCTL_PRUART_R7 )
		;
	
	// 3. Enable the clock to GPIO Port E (RCGCGPIO)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	
	// 4. Wait for GPIO module to be ready (PRGPIO)
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4 ) != SYSCTL_PRGPIO_R4 )
		;
	
	// 5. Configure GPIO pins: PE0 = digital input, PE1 = digital output
	HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) |= (RX_PIN | TX_PIN);
	HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) |= TX_PIN;
	HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= ~RX_PIN;
	
	// 6. Select alternate function for UART pins (AFSEL)
	HWREG(GPIO_PORTE_BASE + GPIO_O_AFSEL) |= (RX_PIN | TX_PIN);
	
	// 7. Configure PMC0-1 fields in GPIOPCTL to assign the UART to PE0-1
	HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xFFFFFF00) + (1<<(0*4)) + (1<<(1*4));
	
	// 8. Disable UART (clear UARTEN bit in UARTCTL)
	HWREG(UART7_BASE + UART_O_CTL) &= ~UART_CTL_UARTEN;
	
	// 9. Write integer portion of BRD to UARTIBRD (Baud rate 9600, Integer = 260, Fraction = 27)
	HWREG(UART7_BASE + UART_O_IBRD) = 0x0104;

	// 10. Write fractional portion of BRD to UARTFBRD
	HWREG(UART7_BASE + UART_O_FBRD) = 0x1B;
	
	// 11. Write desired serial parameters to UARTLCRH
	HWREG(UART7_BASE + UART_O_LCRH) |= (BIT5HI | BIT6HI); // 8-bit word length, all other bits zero
	
	// 12. Configure UART operation using UARTCTL register 
	HWREG(UART7_BASE + UART_O_CTL) |= (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_EOT); // Enable Receive and Transmit
	
	// 13. Enable UART by setting UARTEN bit in UARTCTL 
	HWREG(UART7_BASE + UART_O_CTL) |= UART_CTL_UARTEN;

	// locally enable RX interrupts
	HWREG(UART7_BASE + UART_O_IM) |= UART_IM_RXIM;
	
	// set NVIC enable for UART7
	HWREG(NVIC_EN1) |= BIT31HI;
	
	// globally enable interrupts 
	__enable_irq();
	
	printf("UART init \r\n");
} 

/****************************************************************************
 Function
     UART_ISR

 Description
     Responds to RXIM or TXIM interrupts

 Author
     Sarah Cabreros
****************************************************************************/

 void UART_ISR(void) {
	//printf("isr \r\n");
	// read UARTMIS
    // if RXMIS set:
 	if ((HWREG(UART7_BASE+UART_O_MIS) & UART_MIS_RXMIS) == UART_MIS_RXMIS) {
		//printf("r \r\n");
		// save new data byte
		DataByte = HWREG(UART7_BASE + UART_O_DR); 

		// clear interrupt flag (set RXIC in UARTICR)
		HWREG(UART7_BASE + UART_O_ICR) |= UART_ICR_RXIC;

		/*
		// post ByteReceived event to Receive_SM (event param is byte in UARTDR) 
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_BYTE_RECEIVED;
		ThisEvent.EventParam = DataByte;
		PostReceive_SM(ThisEvent);*/
		
		ProcessByte(DataByte);
		
 	}

	// else if TXMIS set (FIFO open): // where do we enable TXIM interrupts??? 
	else if ((HWREG(UART7_BASE+UART_O_MIS) & UART_MIS_TXMIS) == UART_MIS_TXMIS) {
		//printf("t \r\n");
	// should get this interrupt for all bytes AFTER the start byte (0x7E) 
		//printf("TXMIS INTERRUPT \n\r");
		// clear interrupt flag 
		HWREG(UART7_BASE + UART_O_ICR) |= UART_ICR_TXIC;

		// post ByteSent event 
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_BYTE_SENT;
		PostTransmit_SM(ThisEvent);
		
		// if this was last byte in message block
		if (IsLastByte()) { // isLastByte from Transmit_SM
			// disable TXIM 
			HWREG(UART7_BASE + UART_O_IM) &= ~UART_IM_TXIM;
		}
		
	}
}

static void ProcessByte(uint8_t DataByte) {
	
	switch (CurrentState) {
	case Wait4Start: 
				
				// check if byte received is 0x7E 
				if ( DataByte == START_DELIMITER ) {
					printf("-R- \r\n");
					//printf("start byte received\r\n");
					// start timer 
					ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
					
					// set current state to Wait4MSB
					CurrentState = Wait4MSBLength;
				}
      
    break;

		case Wait4MSBLength:      
			
				// store MSB in data packet 
				MSBLength = DataByte; 
				// start receive timer
				ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				// set current state to Wait4LSB
				CurrentState = Wait4LSBLength;
			
		
    break;
		
		case Wait4LSBLength: 

				// store LSB in data packet 
				LSBLength = DataByte; 
				
				// update FrameLength and BytesLeft
				FrameLength = MSBLength + LSBLength;
				BytesLeft = FrameLength;
				
				// start receive timer
				ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				
				// set ArrayIndex to 0 
				ArrayIndex_UART = 0;
				
				// initialize runningsum to 0
				RunningSum = 0;
				
				// set current state to ReceivingData
				CurrentState = ReceivingData;
				
      
    break;
		
		case ReceivingData: 

				// if BytesLeft = 0, then we just received the checksum 
				if (BytesLeft == 0) {

					if (DataByte == (0xFF - RunningSum)) {
						//printf("Checksum is good: ReceiveSM");
						
						/*
						uint8_t Header = LocalPacket[INDEX];
						if (Header == ENCRY) {
							CopyDataPacketToEncryptionArray();
						} else {
							CopyDataPacket();
						}
						
						
						*/
						// copy local data packet into ready data packet
						//CopyDataPacket();
						
						// if good checksum, post PacketReceived event to FARMER_SM
						ES_Event ThisEvent;         
						ThisEvent.EventType = ES_DATAPACKET_RECEIVED;
						ThisEvent.EventParam = FrameLength; 
						PostComm_Service(ThisEvent);
						//printf("event posted \n\r");
					} else {
						// if bad checksum, don't do anything
					}
					
					// go back to Wait4Start
					CurrentState = Wait4Start;
				} else {

					// else if BytesLeft > 0, then we're still receiving data bytes
					// store current data byte 
					//uint8_t CurrentByte = DataByte;
					
					// add byte to DataPacket
					LocalDataPacket[ArrayIndex_UART] = DataByte;
					
					if (ArrayIndex_UART == 0) {
						API_Identifier = DataByte;
						printf("API_Ident: %i \n\r", API_Identifier);
					}
								//printf("%i: %i\r\n", ArrayIndex, DataByte);
					//printf("LDP: %i \n\r", LocalDataPacket[0]);
					//}
					// increment ArrayIndex
					ArrayIndex_UART++;
					
					// update check sum
					RunningSum += DataByte;
					
					// decrement Bytes left 
					BytesLeft--;
					
					//printf("bytes left: %i\r\n", BytesLeft);
					
					// start receive timer 
					ES_Timer_InitTimer(RECEIVE_TIMER, RECEIVE_TIMER_LENGTH);
				
			}
      
    break;
		}
}

uint8_t* GetDataPacket (void) {
	return &LocalDataPacket[0];
}

uint8_t GetAPIIdentifier(void) {
	return API_Identifier;
}

void SetUARTState(void) {
	CurrentState = Wait4Start;
}

static void CopyDataPacket(void) {
	for (int i = 0; i < MAX_PACKET_LENGTH; i++) {
		ReadyDataPacket[i] = LocalDataPacket[i];
	}
			printf("LocalDataPacket: %i      ReadyDataPacket: %i \n\r", LocalDataPacket[0], ReadyDataPacket[0]);

}
