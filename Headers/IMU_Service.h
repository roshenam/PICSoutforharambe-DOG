/****************************************************************************
 
  Header file for IMU Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef IMU_Service_H
#define IMU_Service_H

#include "ES_Configure.h"
#include "ES_Types.h"

// Public Function Prototypes

bool InitIMU_Service ( uint8_t Priority );
bool PostIMU_Service( ES_Event ThisEvent );
ES_Event RunIMU_Service( ES_Event ThisEvent );


#endif /* IMU_Service_H */

