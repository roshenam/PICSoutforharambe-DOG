/****************************************************************************
 
  Header file of PWM_Module 

 ****************************************************************************/

#ifndef PWM_Module_H
#define PWM_Module_H

#include "ES_Types.h"

// Public Function Prototypes

void PWM_InitHardware( void );
void PWM_SetDutyCycle( float newDuty, uint8_t generatorNum, uint8_t channel );
void PWM_SetDutyPeriod( float newPeriod_uS, uint8_t generatorNum, uint8_t channel );
void PWM_SetFrequency( uint16_t newFrequency, uint8_t generatorNum );
void PWM_SetPolarity( uint8_t polarity, uint8_t generatorNum );
uint8_t PWM_GetPolarity( uint8_t generatorNum );

#endif /* PWM_Module_H */

