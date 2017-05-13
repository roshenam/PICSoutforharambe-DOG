/****************************************************************************
 
  Header file for #defines that my be needed by multiple modules

 ****************************************************************************/
 
 //General Bit Command definitions
#define GET_MSB_IN_LSB(x) ((x & 0x80)>>7)
#define ALL_BITS (0xff<<2)
#define BITS_PER_NIBBLE 4


// SSI defines
#define SSI_CLOCK GPIO_PIN_2
#define SSI_SS GPIO_PIN_3
#define SSI_RX GPIO_PIN_4
#define SSI_TX GPIO_PIN_5
#define SSI_CLOCK_HI BIT2HI
#define SSI_CLOCK_LO BIT2LO
#define SSI_SS_HI BIT3HI
#define SSI_SS_LO BIT3LO
#define SSI_RX_HI BIT4HI
#define SSI_RX_LO BIT4LO
#define SSI_TX_HI BIT5HI
#define SSI_TX_LO BIT5LO
#define SSI_NVIC BIT7HI
#define CPSDVSR 50
#define SCRDVSR 150


// Timing Defines
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define uS_PER_SECOND 1000000




