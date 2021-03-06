#ifndef DOG_SM_H
#define DOG_SM_H

#include "ES_Configure.h"
#include "ES_Types.h"
#include "ES_Events.h"

typedef enum {Waiting2Pair, Paired_Waiting4Key, Paired  } DOGState_t ;


bool InitDOG_SM ( uint8_t Priority );
bool PostDOG_SM( ES_Event ThisEvent );
ES_Event RunDOG_SM( ES_Event ThisEvent );
uint8_t GetHeader(void);
void ResetEncryptionKeyIndex(void);
void TransmitResetEncryption(void);

/***getters**/
uint8_t GetPairedFarmerLSB (void);
uint8_t GetPairedFarmerMSB (void);
uint8_t GetDogTag(void);
DOGState_t GetDOGState(void);

#endif 
