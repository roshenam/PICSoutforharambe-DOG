/****************************************************************************
 
  Header file of Standard Project Constants

 ****************************************************************************/

#ifndef Constants_H
#define Constants_H

//Ticks
#define TicksPerUS 							  40
#define TicksPerMS 							  40000
#define TicksPerUS_f							40.f

#define MS_PER_S									1000
#define TicksPerS 								40000000


//Bits
#define BITS_IN_NIBBLE 					  4
#define ALL_BITS 								  (0xff<<2)
#define MAX_16_BIT 							  0xFFFF


//GPIO
#define INPUT										  0
#define OUTPUT									  1
#define HI											  1
#define LO											  0

//Data Types
#define DOG_FARMER_REPORT         0x00
#define FARMER_DOG_REQ_2_PAIR     0x01
#define DOG_FARMER_IDENTIFICATION 0x02
#define FARMER_DOG_ENCR_KEY       0x03
#define FARMER_DOG_CTRL           0x04
#define FARMER_DOG_RESET_ENCR     0x05

//Data Packet Array
#define FRAME_ID                  0x01
#define API_IDENTIFIER_Tx         0x01
#define API_IDENTIFIER_Rx         0x81
#define API_IDENTIFIER_Tx_Result  0x89
#define MAX_FRAME_LENGTH          40
#define START_DELIMITER 					0x7E

#define PAIR_ACK_FRAME_LENGTH     6

//Tx Packet
#define START_BYTE_INDEX					0
#define LENGTH_MSB_BYTE_INDEX			1
#define LENGTH_LSB_BYTE_INDEX			2
#define API_IDENT_BYTE_INDEX_TX   3
#define FRAME_ID_BYTE_INDEX       4
#define DEST_ADDRESS_MSB_INDEX    5
#define DEST_ADDRESS_LSB_INDEX    6
#define OPTIONS_BYTE_INDEX_TX     7
#define PACKET_TYPE_BYTE_INDEX_TX 8 

//Rx Packet
#define API_IDENT_BYTE_INDEX_RX   0
#define SOURCE_ADDRESS_MSB_INDEX  1
#define SOURCE_ADDRESS_LSB_INDEX  2
#define RSSI_BYTE_INDEX           3 
#define OPTIONS_BYTE_INDEX_RX			4
#define PACKET_TYPE_BYTE_INDEX_RX	5
#define DOG_TAG_BYTE_INDEX        6   


// Servo
#define SERVO_MAX_PULSE						2000 // uS
#define SERVO_MIN_PULSE						1000 // uS


//Game Time
#define GAME_TIME									138

//Interrupts
#define PRIORITY_0 								0
#define PRIORITY_1 								1
#define PRIORITY_2 								2 
#define PRIORITY_3								3

#define CLOCK_FULL_LOAD 					0xFFFFFFFF


#endif 
