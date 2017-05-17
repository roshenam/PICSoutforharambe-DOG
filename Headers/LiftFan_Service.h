/****************************************************************************
 
  Header file for LiftFan_Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef LiftFan_Service_H
#define LiftFan_Service_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

// Public Function Prototypes

bool InitLiftFan_Service ( uint8_t Priority );
bool PostLiftFan_Service( ES_Event ThisEvent );
ES_Event RunLiftFan_Service( ES_Event ThisEvent );

#endif /* LiftFan_Service_H */

