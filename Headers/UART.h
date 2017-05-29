#ifndef UART_H
#define UART_H

// Public Function Prototypes

typedef enum { Wait4Start, Wait4MSBLength, Wait4LSBLength, ReceivingData } UARTReceiveState_t ;

void InitUART(void);
void UART_ISR(void);
uint8_t GetAPIIdentifier(void);

uint8_t* GetDataPacket (void);
void SetUARTState(void);

#endif 
