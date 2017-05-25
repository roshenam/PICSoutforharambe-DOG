
#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>
#include <stdbool.h>

void SPI_Init( void );
void SPI_ISR( void );
void Enable_EOTInt( void );
void Disable_EOTInt( void );
void Write2IMU( uint8_t Byte );

#endif // __SPI_H__

