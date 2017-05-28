/****************************************************************************
 
  Header file for DogTail_Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef DogTail_Service_H
#define DogTail_Service_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

// Public Function Prototypes

bool InitDogTail_Service ( uint8_t Priority );
bool PostDogTail_Service( ES_Event ThisEvent );
ES_Event RunDogTail_Service( ES_Event ThisEvent );

#endif /* DogTail_Service_H */

