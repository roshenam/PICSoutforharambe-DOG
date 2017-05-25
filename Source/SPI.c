/****************************************************************************
 Module
		SSI.c

 Revision
   1.0.1

 Description
   This module acts as the initializer and interactor with all serial communication functionality

 Notes

****************************************************************************/
// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>

#include "termio.h"
#include "ES_Port.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "IMU_Service.h"

#include "BITDEFS.H"

/*---------------------------- Module Defines ---------------------------*/
#define SSI_CLOCK GPIO_PIN_0
#define SSI_SS GPIO_PIN_1
#define SSI_RX GPIO_PIN_2
#define SSI_TX GPIO_PIN_3
#define BITS_PER_NIBBLE 4
#define SSI_NVIC BIT2HI
#define CPSDVSR 50
#define SCRDVSR 150

/*---------------------------- Module Variables ---------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*------------------------------ Module Code ------------------------------*/


/****************************************************************************
 Function
     SSI_Init

 Parameters
     none

 Returns
     none

 Description
     initializes Tiva serial communication
 Notes

 Author
     R. MacPherson, 2/18/2017
****************************************************************************/
void SPI_Init( void ){
	
	printf("\n\rInitialized in SSI init at the beginning\r\n");
	//Enable the clock to the GPIO port D
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	//Enable the clock to SSI module 1
		HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R1;
  //Wait for the GPIO port to be ready
		while( (HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3 ) != SYSCTL_PRGPIO_R3){};
  //Program the GPIO to use the alternate functions on the SSI pins
		HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= ( SSI_CLOCK | SSI_SS | SSI_RX | SSI_TX );
  //Set mux position in GPIOPCTL to select the SSI use of the pins (SSI functions are at bit position 2 for each pin)
	// 2 is the mux value to select SSI1, 8 to shift it over to the
  // right nibble for bit 0 for PD0 (4 bits/nibble * 0 bits), then do the same for PD1, PD2, PD3
		HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) =
		(HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0xffff0000) + (2<<0*BITS_PER_NIBBLE) + (2<<1*BITS_PER_NIBBLE) + (2<<2*BITS_PER_NIBBLE) + (2<<3*BITS_PER_NIBBLE);
  //Program the port lines for digital I/O
		HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= ( SSI_CLOCK | SSI_SS | SSI_RX | SSI_TX );
  //Program the required data directions on the port lines
		HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) |= ( SSI_CLOCK | SSI_SS | SSI_TX );
		HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= ~( SSI_RX );
  //Program the pull-up on the clock line
		HWREG(GPIO_PORTD_BASE+GPIO_O_PUR) |= SSI_CLOCK;
  //Wait for the SSI1 to be ready
		while( (HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R1 ) != SYSCTL_PRSSI_R1){};
  //Make sure that the SSI is disabled before programming mode bits
		HWREG(SSI1_BASE+SSI_O_CR1) &= ~SSI_CR1_SSE;
  //Select master mode (MS) & TXRIS indicating End of Transmit (EOT)
		HWREG(SSI1_BASE+SSI_O_CR1) &= ~SSI_CR1_MS;
		HWREG(SSI1_BASE+SSI_O_CR1) |= SSI_CR1_EOT;
  //Configure the SSI clock source to the system clock
		HWREG(SSI1_BASE+SSI_O_CC) = SSI_CC_CS_SYSPLL; 
  //Configure the clock pre-scaler
		HWREG(SSI1_BASE+SSI_O_CPSR) = CPSDVSR;
  //Configure clock rate (SCR), phase & polarity (SPH, SPO), mode (FRF), data size (DSS)
		HWREG(SSI1_BASE+SSI_O_CR0) = (HWREG(SSI1_BASE+SSI_O_CR0) & ~SSI_CR0_SCR_M) + (SCRDVSR<<8);
		HWREG(SSI1_BASE+SSI_O_CR0) |= SSI_CR0_SPH;
		HWREG(SSI1_BASE+SSI_O_CR0) |= SSI_CR0_SPO;
		HWREG(SSI1_BASE+SSI_O_CR0) = (HWREG(SSI1_BASE+SSI_O_CR0) & ~SSI_CR0_FRF_M) + SSI_CR0_FRF_MOTO;
		HWREG(SSI1_BASE+SSI_O_CR0) = (HWREG(SSI1_BASE+SSI_O_CR0) & ~SSI_CR0_DSS_M) + SSI_CR0_DSS_8;
  //Locally enable interrupts (TXIM in SSIIM)
		HWREG(SSI1_BASE+SSI_O_IM) |= SSI_IM_TXIM;	
  //Make sure that the SSI is enabled for operation
		HWREG(SSI1_BASE+SSI_O_CR1) |= SSI_CR1_SSE;
  //Enable the NVIC interrupt for the SSI when starting to transmit, SSI1 is interrupt 34 
		HWREG(NVIC_EN1) |= SSI_NVIC; 
		printf("\n\rInitialized in SSI init at the end\r\n");
	
}

/****************************************************************************
 Function
     SSI_ISR

 Parameters
     none

 Returns
     none

 Description
     deals with clearing the ISR interrupt and pulling the data when it's ready
 Notes

 Author
     R. MacPherson, 2/18/2017
****************************************************************************/
void SPI_ISR( void ){
	// clear the source of the interrupt
	//HWREG(SSI0_BASE+SSI_O_ICR) = SSI_ICR_RTIC;
	// disable interrupts
	//HWREG(NVIC_EN0) &= ~SSI_NVIC; 
	HWREG(SSI1_BASE+SSI_O_IM) &= ~SSI_IM_TXIM;
	
	// read the status of the SSI Data Register
	ES_Event ThisEvent;
  ThisEvent.EventType = ES_EOT;
  ThisEvent.EventParam = 0;
	PostIMU_Service( ThisEvent );
	
	//printf("EOT\r\n");
}

/****************************************************************************
 Function
     Enable_EOTInt

 Parameters
     none

 Returns
     none

 Description
     allows other modules to enable the EOT interrupts
 Notes

 Author
     R. MacPherson, 2/6/2017
****************************************************************************/
void Enable_EOTInt( void ){
	// enable interrupts
	//HWREG(NVIC_EN0) |= SSI_NVIC; 
	HWREG(SSI1_BASE+SSI_O_IM) |= SSI_IM_TXIM;
}

/****************************************************************************
 Function
     Disable_EOTInt

 Parameters
     none

 Returns
     none

 Description
     allows other modules to disable the EOT interrupts
 Notes

 Author
     R. MacPherson, 2/23/2017
****************************************************************************/
void Disable_EOTInt( void ){
	// disable interrupts
	//HWREG(NVIC_EN0) |= SSI_NVIC; 
	HWREG(SSI1_BASE+SSI_O_IM) &= ~SSI_IM_TXIM;
}


/****************************************************************************
 Function
     Write2LOC

 Parameters
		Data - uint8_t one byte of data to write to the LOC

 Returns
     none

 Description
     allows other modules to write data to the LOC
 Notes

 Author
     R. MacPherson, 2/23/2017
****************************************************************************/
void Write2IMU( uint8_t Byte ){
	// write the byte to the data register to send it to the LOC
	HWREG(SSI1_BASE+SSI_O_DR) = (HWREG(SSI1_BASE+SSI_O_DR) & ~SSI_DR_DATA_M) + Byte;
	
}





