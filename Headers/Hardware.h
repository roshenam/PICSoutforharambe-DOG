/****************************************************************************
 
  Header file of Hardware created for ME218C Final Project

 ****************************************************************************/

#ifndef Hardware_H
#define Hardware_H

#include "ES_Types.h"
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"


// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "termio.h"

#include "BITDEFS.H"

//My Modules
#include "UART.h"
#include "Receive_SM.h"
#include "DOG_SM.h"
#include "IMU_Service.h"
#include "HoverControl_Module.h"
#include "Constants.h"
#include "Comm_Service.h"
#include "Transmit_SM.h"
#include "LiftFan_Service.h"
#include "PWM_Module.h"

#include "DogTail_Service.h"
#include "ADMulti.h"


/********************Module Defines*******************************************/
//Port A


//Port B
#define PIC_PORT					GPIO_PORTB_BASE
#define PIC_PIN	  				GPIO_PIN_0

#define MOTOR_PORT 		    GPIO_PORTB_BASE
#define MOTOR_LEFT_EF			GPIO_PIN_2
#define MOTOR_RIGHT_EF		GPIO_PIN_3
#define MOTOR_LEFT_0_PIN	GPIO_PIN_6
#define MOTOR_LEFT_1_PIN  GPIO_PIN_7
#define MOTOR_RIGHT_0_PIN GPIO_PIN_4
#define MOTOR_RIGHT_1_PIN GPIO_PIN_5


//Port C


//Port D


//Port E

#define RX_PIN	BIT4HI 	// UART4 Rx: PC4
#define TX_PIN	BIT5HI	// UART4 Tx: PC5

#define SERVO_PORT				GPIO_PORTE_BASE
#define SERVO_PIN					GPIO_PIN_4

//Port F


//Port G


// Public Function Prototypes
uint8_t ReadInput (int Port, uint8_t Pin);
void SetOutput (int Port, uint8_t Pin, uint8_t Value);
void InitPin (int Port, uint8_t Pin, uint8_t Direction);
void InitAll (void);

#endif /* Hardware_H */

