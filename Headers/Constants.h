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

#define ON												1
#define OFF												0

//Data Packet Types
#define DOG_FARMER_REPORT         0x00
#define FARMER_DOG_REQ_2_PAIR     0x01
#define DOG_ACK										0x02
#define FARMER_DOG_ENCR_KEY       0x03
#define FARMER_DOG_CTRL           0x04
#define DOG_FARMER_RESET_ENCR     0x05

//API Structure Stuff
#define FRAME_ID                  0x01
#define API_IDENTIFIER_Tx         0x01
#define API_IDENTIFIER_Rx         0x81
#define API_IDENTIFIER_Tx_Result  0x89
#define API_IDENTIFIER_Reset			0x8A
#define START_DELIMITER 					0x7E
#define OPTIONS										0x00

//frame lengths
#define ACK_N_ENCRYPT_FRAME_LEN		6
#define STATUS_FRAME_LEN					18 //13 bytes + 5 header type bytes in frame data
#define HEADER_LENGTH							3
#define MAX_PACKET_LENGTH         45
#define FARMER_CMD_LENGTH					4 //header and data bytes 


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

#define IMU_DATA_NUM_BYTES				12

//Rx Packet
#define API_IDENT_BYTE_INDEX_RX   0
#define FRAME_ID_BYTE_INDEX_RX		1
#define TX_STATUS_BYTE_INDEX			2

#define SOURCE_ADDRESS_MSB_INDEX  1
#define SOURCE_ADDRESS_LSB_INDEX  2
#define RSSI_BYTE_INDEX           3 
#define OPTIONS_BYTE_INDEX_RX			4

#define PACKET_TYPE_BYTE_INDEX_RX	5
#define DOG_TAG_BYTE_INDEX        6  

#define ENCRYPTION_KEY_LENGTH			32

#define DIGITAL_MASK							0x03
 

//Result from Transmit Packet
#define SUCCESS										0

// Actuators

#define MOTOR_LEFT_PWM						0
#define MOTOR_RIGHT_PWM						1
#define SERVO_PWM									2

#define PWM_FORWARD_POL						0
#define PWM_REVERSE_POL						1

#define SERVO_MAX_PULSE						2 // mS
#define SERVO_MIN_PULSE						1 // mS

#define MAX_TURNING_DIFF_DIVISOR	1 //100/MAX --> DIVISOR
#define DEADBAND									20

//Timers
#define ONE_SEC										976
#define GAME_TIME									218*ONE_SEC
#define INTER_MESSAGE_TIME				300	// FARMER transmits a packet every 300 ms 
#define LOST_COMM_TIME						3*ONE_SEC // DOG+FARMER unpair if no message received after 1 second
#define WAG_TIME									ONE_SEC

//Interrupts
#define PRIORITY_0 								0
#define PRIORITY_1 								1
#define PRIORITY_2 								2 
#define PRIORITY_3								3

#define CLOCK_FULL_LOAD 					0xFFFFFFFF


#endif 
