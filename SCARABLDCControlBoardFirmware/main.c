/*
 * SCARAMotorControlBoardFirmware.c
 *
 * Created: 6/6/2019 6:29:34 AM
 * Author : User
 */ 


#include "sam.h"
#include <math.h>
#include "Utils.h"
#include "Transforms.h"
#include "QDEC.h"
#include "SVPWM.h"
#include "PWMControl.h"
#include "ADC.h"
#include "SixStepCommutation.h"

#define ENCODERRESOLUTION 2000
#define PWMMAXVALUE 4800

#define HALL_U_PIN 30
#define HALL_U_PORT 3//D
#define HALL_V_PIN 7
#define HALL_V_PORT 0//A
#define HALL_W_PIN 8
#define HALL_W_PORT 0//A


#define U_UPPER_OUTPUT_PIN 20//PWM channel 0
#define U_UPPER_OUTPUT_PORT 3//D

#define U_LOWER_OUTPUT_PIN 24
#define U_LOWER_OUTPUT_PORT 3//D

#define V_UPPER_OUTPUT_PIN 21//PWM channel 1
#define V_UPPER_OUTPUT_PORT 3//D

#define V_LOWER_OUTPUT_PIN 25
#define V_LOWER_OUTPUT_PORT 3//D

#define W_UPPER_OUTPUT_PIN 22//PWM channel 2
#define W_UPPER_OUTPUT_PORT 3//D

#define W_LOWER_OUTPUT_PIN 26
#define W_LOWER_OUTPUT_PORT 3//D

#define DIR_INPUT 0
#define DIR_OUTPUT 1

#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3

#define STATE_OFF 0
#define STATE_ON 1

#define SETDPIN(a) (REG_PIOD_SODR |= (1 << a))
#define CLEARDPIN(a) (REG_PIOD_CODR |= (1<<a))



//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)


int main(void)
{
    /* Initialize the SAM system */
    //SystemInit();
	
	ClockSetup();
	
	PIOSetup();
	
	PWMSetup();
	
	QDECSetup();

/*	
	
	//Disable PMC write protection
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43) | (0<<0);
	
	
	//Set up the ADC
	//Give pins to ADC rather than peripheral
	//Disable write protection
	REG_AFEC0_WPMR = AFE_WPMR_WPEN | AFE_WPMR_WPKEY(0x414443);
	//Set the ADC so it will tell what channel a reading is from.
	REG_AFEC0_EMR |= AFE_EMR_TAG;
	REG_AFEC1_EMR |= AFE_EMR_TAG;
	//Looks to be a bit weird. Need to consult my schematic. Woops. Forgot to connect those. Yay for V2!
	//NEED TO FIGURE OUT WHICH PINS TO USE AND HOW MANY PHASES TO MEASURE. V1 ONLY MEASURES 2 PHASES
	REG_AFEC0_CHER |= AFE_CHER_CH0;//For Channel 0
	REG_AFEC1_CHER |= AFE_CHER_CH1;//For Channel 1
	//Enable ADC clock (PMC stuff)
	REG_PMC_PCER0 |= PMC_PCER0_PID30 | PMC_PCER0_PID31;
	//Setup various settings. AFEC_MR starts as 0, so don't need to change anything that should be 0.
	REG_AFEC0_MR |= AFE_MR_TRANSFER(2) | AFE_MR_STARTUP_SUT512;
	
	
	
	//Set up the FPU
	
	
	
	
	
	
	
	//Set up SPI
	//Enable peripheral clock
	REG_PMC_PCER0 |= PMC_PCER0_PID19;
	//Need to connect the SS line from the master (screw terminal in this case) to NPCS0 pin
	//Set to slave mode
	REG_SPI_MR &= ~(SPI_MR_MSTR);
	//Just use the default polarity and phase settings since I'm coding both master and slave.
	//Enable SPI
	REG_SPI_CR |= SPI_CR_SPIEN;
	//Set for 16 bit words
	REG_SPI_CSR |= SPI_CSR_BITS_16_BIT;
	//Should be good to go
	
	
	
	uint32_t RotorPosition = 0;
	uint32_t MotorPosition = 0;
	//uint32_t MotorDirection = 0;
	float CommandedTorque = 0;
	uint32_t CommandedPosition = 0;
	uint32_t CurrentA = 0;
	uint32_t CurrentB = 0;
	float ClarkCurrentMeasure[2];
	float ParkCurrentMeasure[2];
	float KpTorque = 2.4;
	float MotorTorqueConstant = 1.8;
	float ClarkVoltageCommand[2];
	float ParkCurrentCommand[2] = {0,0};
	uint32_t PositionError = 0;
	float ParkVoltageReference[2] = {0,0};
	float KParkFlux = 0.7;
	float KParkTorque = 1.3;
	float ParkCurrentError[2] = {0,0};
	uint32_t SVPWMOutputValues[4];
	uint32_t HasBeenIndex = 0;
	uint32_t HallU = 0;
	uint32_t HallV = 0;
	uint32_t HallW = 0;
	
	
	//Construct arrays of sine and cosine values. Memory heavy (like a quarter of the RAM), but should increase runtime speed.
	static float SinArrayVar[ENCODERRESOLUTION];
	for(uint32_t i = 0; i < ENCODERRESOLUTION; i++){
		SinArrayVar[i] = sin((float)i/2000.*6.283185);
	}
	static float CosArrayVar[ENCODERRESOLUTION];
	for(uint32_t i = 0; i < ENCODERRESOLUTION; i++){
		CosArrayVar[i] = cos((float)i/2000.*6.283185);
	}
	//SineArray(SinArrayVar ,ENCODERRESOLUTION);
	//CosineArray(CosArrayVar ,ENCODERRESOLUTION);
	
	
	//The motor driver needs to have its floating caps charged.
	//So, the code will hold the bottom FETs on for a couple seconds
	//to ensure this happens. 
	//After this, the control will all be via PWM, so they shouldn't discharge
	//Turn on lower FETs and turn off upper FETs
	REG_PIOD_SODR = (1<<HALLALOWER) | (1<<HALLBLOWER) | (1<<HALLCLOWER);
	REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);
	//Busy wait loop. Yes its terrible. Don't care.
	//Bootup time is not a concern for me.
	for (volatile uint32_t i = 0; i < 4500;i++)
	{
	}
	//Turn off all FETs
	REG_PIOD_CODR = (1<<HALLALOWER) | (1<<HALLBLOWER) | (1<<HALLCLOWER);
	REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);	
	
	PWMSetup();
*/
	
	
	
		//This is the loop that will run when the motor just boots up.
		//It has not yet caught the index, so the vector control won't work yet
		//This loop just runs the motor very slowly using 6 step commutation using the Hall sensors

		uint32_t HallU = 0;
		uint32_t HallV = 0;
		uint32_t HallW = 0;
		
		//uint32_t IndexCount = 0;
		volatile int32_t SingleRotationPosition = 0;
		volatile int32_t EncoderPosition = 0;
		volatile int32_t Direction = 0;
		

		
	
		//Set the Hall input pins to be inputs
		EnablePIOControl(HALL_U_PORT, HALL_U_PIN);
		EnablePIOControl(HALL_V_PORT, HALL_V_PIN);
		EnablePIOControl(HALL_W_PORT, HALL_W_PIN);
		SetPinIODirection(HALL_U_PORT, HALL_U_PIN, DIR_INPUT);
		SetPinIODirection(HALL_V_PORT, HALL_V_PIN, DIR_INPUT);
		SetPinIODirection(HALL_W_PORT, HALL_W_PIN, DIR_INPUT);

		//Set the Output pins to be outputs. This works for six step commutation. will have to be adjusted for space vector control later on.
		//First give them to the PIO
		EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
		EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
		EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
		EnablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
		EnablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
		EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
		
		//Then set them as outputs.
		SetPinIODirection(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN, DIR_OUTPUT);
		SetPinIODirection(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN, DIR_OUTPUT);
		SetPinIODirection(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN, DIR_OUTPUT);
		SetPinIODirection(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN, DIR_OUTPUT);
		SetPinIODirection(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN, DIR_OUTPUT);
		SetPinIODirection(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN, DIR_OUTPUT);
		
		while (1)//!HasBeenIndex)
		{
			//Measure the Hall sensor outputs
			HallU = ReadIOPinValue(HALL_U_PORT, HALL_U_PIN);
			HallV = ReadIOPinValue(HALL_V_PORT, HALL_V_PIN);
			HallW = ReadIOPinValue(HALL_W_PORT, HALL_W_PIN);
			
			//Read position from encoder
			SingleRotationPosition = QDECGetPostionSingle();
			EncoderPosition = QDECGetPositionTotal();
			Direction = QDECGetDirection();
			

			

			//Energize the windings in accordance with simple six step commutation
			//SixStepCommutation(2550,1,0,HallU,HallV,HallW);
			
			/*
			//PWM Test
			//Set PWM control of all output pins
			//Since the outputs are all on PORTD, and the ABCDSR registers default to 0x0 and 0x0, they default to the PWM
			//So, don't need to do anything
			DisablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			DisablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			DisablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			DisablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			DisablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			DisablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			//Set the PWM duty cycle to something
			//remember max is 4800
			//REG_PWM_CDTY0 = 1000;
			//REG_PWM_CDTY1 = 2000;
			//REG_PWM_CDTY2 = 3000;
			uint32_t hold[] = {200,200,200};//,1,1,0};
			uint32_t hold2[] = {2000,2000,2000};//,1,1,0};
			UpdateOutputPWMDutyCycles(hold);//AndPolarities(hold);
			for (volatile uint32_t i = 0; i < 2550; i++)
			{
			}
			UpdateOutputPWMDutyCycles(hold2);//AndPolarities(hold2);
			for (volatile uint32_t i = 0; i < 2550; i++)
			{
			}
			*/
			
			
			
			/*
			if(HallU >= 0){
			SetPinIOOutputValue(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN, STATE_ON);
			SetPinIOOutputValue(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN, STATE_ON);
			SetPinIOOutputValue(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN, STATE_ON);
			SetPinIOOutputValue(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN, STATE_ON);
			SetPinIOOutputValue(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN, STATE_ON);
			SetPinIOOutputValue(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN, STATE_ON);
			
			for (volatile uint32_t i = 0; i < 1; i++)
			{
			}

			
			SetPinIOOutputValue(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN, STATE_OFF);
			SetPinIOOutputValue(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN, STATE_OFF);
			SetPinIOOutputValue(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN, STATE_OFF);
			SetPinIOOutputValue(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN, STATE_OFF);
			SetPinIOOutputValue(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN, STATE_OFF);
			SetPinIOOutputValue(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN, STATE_OFF);
			
			for (volatile uint32_t i = 0; i < 1; i++)
			{
			}

			}
			
			*/
			
			/*
			REG_PIOD_ABCDSR1 |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			REG_PIOD_PDR |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 00;//720;
			REG_PWM_CDTY1 = 00;//720;
			REG_PWM_CDTY2 = 00;//720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			//REG_PIOD_SODR = (1<<C_LOWER_OUTPUT) | (1<<B_UPPER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<W_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			//|float|+|-|
			*/
			
			
			
			
			
			
			
			
			//}
			//REG_PIOD_SODR = (1<<HALLBLOWER) | (1<<HALLCLOWER) | (1<<HALLALOWER);
			//REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);
			//Check if the index has been passed.
			//HasBeenIndex = REG_TC0_CV1;
		}
	
	
	
	
	
	
	
	

    /* Replace with your application code */
/*
    while (1) 
    {
				
		
		//Must first measure the phase currents of A and B.
		CurrentA = ADC0Read(0);
		CurrentB = ADC1Read(1);
		CurrentA = (float)CurrentA/(4096.)*3.3;
		CurrentB = (float)CurrentB/(4096.)*3.3;
		
		
		//Initial transform of measured current to rotating frame current (where the reference is constant)
		RotorPosition = QDECGetPostionSingle();
		ClarkTransform(ClarkCurrentMeasure, CurrentA, CurrentB);
		ParkTransform(ParkCurrentMeasure, ClarkCurrentMeasure, RotorPosition, SinArrayVar, CosArrayVar);
		
		//Temporary P controller for Torque. This is position control, it assumes a desired position will be provided.
		MotorPosition = QDECGetPositionTotal();
		PositionError = CommandedPosition - MotorPosition;
		CommandedTorque = (float)PositionError*KpTorque;
		
		//Define the reference Park Currents
		//ParkCurrent[0] is the flux current. This is 0 for a BLDC.
		//ParkCurrent[1] is the torque current. This depends on the desired torque.
		ParkCurrentCommand[0] = 0;
		ParkCurrentCommand[1] = MotorTorqueConstant*CommandedTorque;
		
		ParkCurrentError[0] = ParkCurrentCommand[0] - ParkCurrentMeasure[0];
		ParkCurrentError[1] = ParkCurrentCommand[1] - ParkCurrentMeasure[1];
		
		//Now, regulate the Park currents via some controller to get the Park Voltage references.
		//Temporarily just a P controller
		ParkVoltageReference[0] = ParkCurrentError[0]*KParkFlux;
		ParkVoltageReference[1] = ParkCurrentError[1]*KParkTorque;
		
		//Inverse Park transform to get the commanded Clark Voltages
		//Update rotor position again first.
		RotorPosition = QDECGetPostionSingle();
		InversePark(ClarkVoltageCommand, ParkVoltageReference, RotorPosition, SinArrayVar, CosArrayVar);
		
		//The ClarkVoltageCommand values will be passed to a space vector PWM scheme for output.
		//This gives back the PWM compare values and invert status for each switch. 
		//[0]-[5] are the compare values. [6]-[11] are the invert statuses.
		SVPWMBase(SVPWMOutputValues, ClarkVoltageCommand[0], ClarkVoltageCommand[1], PWMMAXVALUE);
		
    }
*/
}
