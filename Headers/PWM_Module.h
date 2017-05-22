/****************************************************************************
 
  Header file of PWM_Module 

 ****************************************************************************/

#ifndef PWM_Module_H
#define PWM_Module_H

#include "ES_Types.h"

// Public Function Prototypes

void InitPWM(void); //init PWM on TIVA
void SetDuty(uint8_t Duty, uint8_t Polarity, uint8_t Actuator); //set the duty of the PWM based on control law
void SendServoHome(void);
void SendServoAway(void);

#endif /* PWM_Module_H */

