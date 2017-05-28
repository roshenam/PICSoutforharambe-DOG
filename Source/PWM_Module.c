/****************************************************************************
 Module
   PWM_Module.c

 Revision
   1.0.1

 Description
   Initialize and use 3 lines of PWM

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 mch      lab 6
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
// the common headers for C99 types 
#include "Hardware.h"

/*----------------------------- Module Defines ----------------------------*/

#define PWMTicksPerMS 40000/32
//Set the Period of PWM
#define PWMTicksPerUS PWMTicksPerMS/1000
// set 1000Hz based on motor
#define PeriodInMS_Motors 1
#define PeriodMS_Motors PeriodInMS_Motors * PWMTicksPerMS

// set 50 Hz frequency based off of time constant of the servo
#define PeriodInMS_Servo 20
#define PeriodMS_Servo PeriodInMS_Servo * PWMTicksPerMS

#define GEN0_A_NORM (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO) //module 0, generator 0, channel A, cmpA
#define GEN0_B_NORM (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO) //module 0, generator 0, channel B, cmpB
#define GEN1_A_NORM (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO) //module 0, generator 1, channel A, cmpA
#define GEN1_B_NORM (PWM_1_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO) //module 0, generator 1, channel B, cmpB
#define GEN2_A_NORM (PWM_2_GENA_ACTCMPAU_ONE | PWM_2_GENA_ACTCMPAD_ZERO) //module 0, generator 2, channel A, cmpA

#define GEN0_A_ZERO PWM_0_GENA_ACTZERO_ZERO
#define GEN0_B_ZERO PWM_0_GENB_ACTZERO_ZERO
#define GEN1_A_ZERO PWM_1_GENA_ACTZERO_ZERO
#define GEN1_B_ZERO PWM_1_GENB_ACTZERO_ZERO
#define GEN2_A_ZERO PWM_2_GENA_ACTZERO_ZERO

#define GEN0_A_HUNDRED PWM_0_GENA_ACTZERO_ONE
#define GEN0_B_HUNDRED PWM_0_GENB_ACTZERO_ONE
#define GEN1_A_HUNDRED PWM_1_GENA_ACTZERO_ONE
#define GEN1_B_HUNDRED PWM_1_GENB_ACTZERO_ONE
#define GEN2_A_HUNDRED PWM_2_GENA_ACTZERO_ONE

//calculate RPM
#define TicksPerSec 40000000 //40MHz
#define EncoderSegmentsPerRev 512
#define SecPerMin 60
#define GearBox 6

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file

// with the introduction of Gen2, we need a module level Priority var as well
//Set the Duty


/***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
    Initialize the PWM on the TIVA
 ***************************************************************************/

void InitPWM(void) {
	
// start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

// enable the clock to Port B  and Port E done elsewhere
	  
// Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  
// make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);

// disable the PWM while initializing
	//Module 0, Generator 0 (channels A and B or PWM0 and PWM1)
  HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;
	//Module 0, Generator 1 (channel A and B, PWM2 and PWM3)
	HWREG( PWM0_BASE+PWM_O_1_CTL ) = 0;
	//Module 0, Generator 2 (channel A, PWM4)
	HWREG( PWM0_BASE+PWM_O_2_CTL ) = 0;

// program generators to go to 0% duty cycle. Not normal 1 on rising compare A/B, 0 on falling compare A/B  
  HWREG( PWM0_BASE+PWM_O_0_GENA) = (GEN0_A_ZERO );
  HWREG( PWM0_BASE+PWM_O_0_GENB) = (GEN0_B_ZERO );
  HWREG( PWM0_BASE+PWM_O_1_GENA) = (GEN1_A_ZERO );
  HWREG( PWM0_BASE+PWM_O_1_GENB) = (GEN1_B_ZERO	);
	
	//servo normal
	HWREG( PWM0_BASE+PWM_O_2_GENA) = GEN2_A_NORM;
	//set the initial position of servo home
	SendServoHome();
		
	// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period. We will also program
// the match compare registers to 1/2 the desired high time  
  HWREG( PWM0_BASE+PWM_O_0_LOAD) = ((PeriodMS_Motors))>>1;
	HWREG( PWM0_BASE+PWM_O_1_LOAD) = ((PeriodMS_Motors))>>1;
	HWREG( PWM0_BASE+PWM_O_2_LOAD) = ((PeriodMS_Servo))>>1;
	
	//polarity normal
	HWREG( PWM0_BASE+PWM_O_INVERT ) &= ~(PWM_INVERT_PWM0INV | PWM_INVERT_PWM1INV | PWM_INVERT_PWM2INV | PWM_INVERT_PWM3INV);
	
	// enable the PWM outputs
  HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM2EN | PWM_ENABLE_PWM3EN | PWM_ENABLE_PWM4EN);

// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for 3 pins
  HWREG(MOTOR_PORT+GPIO_O_AFSEL) |= (MOTOR_LEFT_0_PIN | MOTOR_LEFT_1_PIN | MOTOR_RIGHT_0_PIN | MOTOR_RIGHT_1_PIN);
	HWREG(SERVO_PORT+GPIO_O_AFSEL) |= (SERVO_PIN);


// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
  HWREG(MOTOR_PORT+GPIO_O_PCTL) = 
    (HWREG(MOTOR_PORT+GPIO_O_PCTL) & 0x0000ffff) +
      (4<<(6*BITS_IN_NIBBLE)) + (4<<(7*BITS_IN_NIBBLE)) + (4<<(4*BITS_IN_NIBBLE)) + (4<<(5*BITS_IN_NIBBLE));
			
	HWREG(SERVO_PORT+GPIO_O_PCTL) = 
    (HWREG(SERVO_PORT+GPIO_O_PCTL) & 0xfff0ffff) +
      (4<<(4*BITS_IN_NIBBLE));

// Enable pins 6 on Port B for digital I/O
	InitPin(MOTOR_PORT, MOTOR_LEFT_0_PIN, OUTPUT);
	InitPin(MOTOR_PORT, MOTOR_LEFT_1_PIN, OUTPUT);
	InitPin(MOTOR_PORT, MOTOR_RIGHT_0_PIN, OUTPUT);
	InitPin(MOTOR_PORT, MOTOR_RIGHT_1_PIN, OUTPUT);
	InitPin(SERVO_PORT, SERVO_PIN, OUTPUT);
  
// set the up/down count mode, enable the PWM generator and make
// both generator updates locally synchronized to zero count
  HWREG(PWM0_BASE+ PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE | 
                                    PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);

  HWREG(PWM0_BASE+ PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE | 
                                    PWM_1_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
																		
	HWREG(PWM0_BASE+ PWM_O_2_CTL) = (PWM_2_CTL_MODE | PWM_2_CTL_ENABLE | 
                                    PWM_2_CTL_GENAUPD_LS);

}


/****************************************************************************
 Function
    Set the duty of the PWM based off of the control law above
 ***************************************************************************/
void SetDuty(uint8_t Duty, uint8_t Polarity, uint8_t Actuator) {
	
	//set the right duty cycle
	switch (Actuator) {
		case MOTOR_LEFT_PWM:			
			if (Duty == 100) {
				HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_HUNDRED;
				//HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_ZERO;
				//printf("DUTY = 100 IN PWMMODULE \n\r");
			} else if (Duty == 0) {
				HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_ZERO;
				//HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_ZERO;
			} else {
				HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_NORM;
				//HWREG( PWM0_BASE+PWM_O_0_GENB) = GEN0_B_ZERO;
				uint32_t HiTime = (PeriodMS_Motors) * Duty/100; //calculate the desired high time
			  HWREG(PWM0_BASE+PWM_O_0_CMPB) = ((PeriodMS_Motors)/2) - (HiTime/2); //set pwm
			}
			
			break;
		case MOTOR_RIGHT_PWM:
			if (Duty == 100) {
				HWREG( PWM0_BASE+PWM_O_1_GENB) = GEN0_B_HUNDRED;
				//printf("DUTY = 100 IN PWMMODULE \n\r");
			} else if (Duty == 0) {
				HWREG( PWM0_BASE+PWM_O_1_GENB) = GEN0_B_ZERO;
			} else {
				HWREG( PWM0_BASE+PWM_O_1_GENB) = GEN0_B_NORM;
				uint32_t HiTime = (PeriodMS_Motors) * Duty/100; //calculate the desired high time
			  HWREG(PWM0_BASE+PWM_O_1_CMPB) = ((PeriodMS_Motors)/2) - (HiTime/2); //set pwm
			}
			
			break;	
	}
	
	/*
	//set the right polarity
	if (Polarity == PWM_FORWARD_POL) {
		HWREG( PWM0_BASE+PWM_O_INVERT ) &= ~(PWM_INVERT_PWM0INV | PWM_INVERT_PWM1INV | PWM_INVERT_PWM2INV | PWM_INVERT_PWM3INV);
	} else if (Polarity == PWM_REVERSE_POL) {
		HWREG( PWM0_BASE+PWM_O_INVERT ) |= (PWM_INVERT_PWM0INV | PWM_INVERT_PWM1INV | PWM_INVERT_PWM2INV | PWM_INVERT_PWM3INV);
	} */
}

void SendServoHome(void) {
	//HWREG( PWM0_BASE+PWM_O_2_GENA) = GEN2_A_NORM;
	HWREG(PWM0_BASE+PWM_O_2_CMPA) = ((PeriodMS_Servo)/2) - ((SERVO_MIN_PULSE*PWMTicksPerMS)/2);
}

void SendServoAway(void) {
	//HWREG( PWM0_BASE+PWM_O_2_GENA) = GEN2_A_NORM;
  HWREG(PWM0_BASE+PWM_O_2_CMPA) = ((PeriodMS_Servo)/2) - ((SERVO_MAX_PULSE*PWMTicksPerMS)/2);
}
