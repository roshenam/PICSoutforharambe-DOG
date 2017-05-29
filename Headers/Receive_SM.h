#ifndef RECEIVE_SM_H
#define RECEIVE_SM_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

typedef enum {InitReceive, RunReceive  } ReceiveState_t ;

bool InitReceive_SM ( uint8_t Priority );
bool PostReceive_SM( ES_Event ThisEvent );
ES_Event RunReceive_SM( ES_Event ThisEvent );


#endif 
