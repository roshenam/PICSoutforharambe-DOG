/****************************************************************************
 
  Header file of Hardware created for ME218C Final Project

 ****************************************************************************/

#ifndef HoverControl_Module_H
#define HoverControl_Module_H


//My Modules
#include "Hardware.h"
#include "Constants.h"

/********************Module Defines*******************************************/


// Public Function Prototypes
void ActivateHover(void);
void DeactivateHover(void);
void ActivateDirectionSpeed(uint8_t DirectionSpeed, uint8_t Turning);
void ActivatePeripheral(uint8_t Peripheral);
void ActivateBrake(uint8_t Brake);

#endif /* Hovercontrol_Module_H */

