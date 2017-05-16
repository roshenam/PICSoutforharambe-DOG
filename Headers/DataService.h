/****************************************************************************
 
  Header file for Data Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef DataService_H
#define DataService_H

#include "ES_Configure.h"
#include "ES_Types.h"

// Public Function Prototypes

bool InitDataService ( uint8_t Priority );
bool PostDataService( ES_Event ThisEvent );
ES_Event RunDataService( ES_Event ThisEvent );

#endif /* DataService_H */

