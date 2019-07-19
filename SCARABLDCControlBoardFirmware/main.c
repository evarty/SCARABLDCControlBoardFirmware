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

//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)


int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	

	
	//Set up Crystal and main clock
	//This will occur only on startup, so speed is not a priority.
	//Following Process from datasheet section 29.15 Programming Sequence
	//Write protection starts as 0, so no need to deal with that 
	//Enable crystal oscillator
	REG_CKGR_MOR |= CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTST_Msk | CKGR_MOR_MOSCXTEN;
	//Wait for MOSCXTST field in PMC_SR to be set
	while(!(REG_PMC_SR & PMC_SR_MOSCXTS));
	//Switch the main clock to the external crystal
	REG_CKGR_MOR |= CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCSEL;
	//Wait for MOSCELS bit to be set the PMC_SR register
	while(!(REG_PMC_SR & PMC_SR_MOSCSELS));
	//Check the clock frequency
	//Wait for check to be available
	while(!(REG_CKGR_MCFR & CKGR_MCFR_MAINFRDY))
	/*
	Check if the crystal is doing anything, 
	in particular, if it is close to the correct frequency,
	If not, revert to internal oscillator.
	The default slow clock is 4 MHz. The crystal in the design
	is 12 MHz. Reading from MAINF gives the number of crystal cycles
	in 16 slow clock cycles. The crystal is three times
	as fast as the slow clock. So, 16*3 = 48.
	*/
	if((REG_CKGR_MCFR & CKGR_MCFR_MAINF_Pos) < 45)
	{
		REG_CKGR_MOR &= ~(CKGR_MOR_MOSCSEL);
	}
	//At this point, the crystal is working and the main clock source
	//but I want a faster clock, so, break out the PLL
	
	//Setup PLL
	//Write the count register for the PLL setup time
	REG_CKGR_PLLAR |= CKGR_PLLAR_PLLACOUNT(0x000F);
	//Set PLL multiplier. multiplication value is this value + 1
	REG_CKGR_PLLAR |= CKGR_PLLAR_MULA(0x0009) | CKGR_PLLAR_DIVA(0x0001);
	//Wait for the LOCKA bit to be set
	while(!(REG_PMC_SR & PMC_SR_LOCKA));
	//Select the PLL as master clock, following datasheet
	REG_PMC_MCKR |= PMC_MCKR_PRES_CLK_1;
	while(!(REG_PMC_SR & PMC_SR_MCKRDY));
	REG_PMC_MCKR |= PMC_MCKR_CSS_PLLA_CLK;
	while(!(REG_PMC_SR & PMC_SR_MCKRDY));
	//At this point the master clock is the PLL
	//which is (9 + 1)*12 MHz = 120 MHz.
	
	
	//Setup PIO
	//The encoder inputs are given to peripherals, so they are fine.
	//The hall effect inputs need to be configured.
	//Turn on PIO clock for all ?channels? I guess
	REG_PMC_PCER0 |= PMC_PCER0_PID9 | PMC_PCER0_PID10 | PMC_PCER0_PID11 | PMC_PCER0_PID12 | PMC_PCER0_PID13;
	//Set relevant pins to be inputs
	//THESE ARE PLACEHOLDER VALUES.
	//enable PIO control
	REG_PIOA_PER |= PIO_PER_P0 | PIO_PER_P1 | PIO_PER_P2;
	//Disable output
	REG_PIOA_ODR |= PIO_ODR_P0 | PIO_ODR_P1 | PIO_ODR_P2;
	//Disable pullup resistors
	REG_PIOA_PUDR |= PIO_PUDR_P0 | PIO_PUDR_P1 | PIO_PUDR_P2;
	//Disable pulldown resistors
	REG_PIOA_PPDDR |= PIO_PPDDR_P0 | PIO_PPDDR_P1 | PIO_PPDDR_P2;
	

	
	
	//Setup the quadrature decoder
	//TIOA0 and TIOB0 are the quadrature signals.
	//TIOB1 is the index signal.
	//This follows AT42706 process in section 5.2.4
	//Configure IO pins to their peripheral functions
	REG_PIOA_PDR |= PIO_PER_P0 | PIO_PER_P1 | PIO_PER_P16;
	REG_PIOA_ABCDSR1 |=  PIO_ABCDSR_P0 | PIO_ABCDSR_P1 | PIO_ABCDSR_P16;
	//Enable channel 0 and 1 peripheral clocks
	//CURRENT CODE ENABLES MODULE 0
	REG_PMC_PCER0 |= PMC_PCER0_PID21 | PMC_PCER0_PID22;
	//Step 3
	REG_TC0_CMR0 &= ~(TC_CMR_WAVE);
	REG_TC0_CMR1 &= ~(TC_CMR_WAVE);
	
	REG_TC0_CMR0 |= TC_CMR_TCCLKS_XC0;
	REG_TC0_CMR1 |= TC_CMR_TCCLKS_XC0;
	
	REG_TC0_CMR0 |= TC_CMR_ETRGEDG_RISING;
	REG_TC0_CMR1 |= TC_CMR_ETRGEDG_RISING;
	
	REG_TC0_CMR0 |= TC_CMR_ABETRG;
	REG_TC0_CMR1 |= TC_CMR_ABETRG;
	
	//Step 4
	REG_TC0_BMR |= TC_BMR_QDEN | TC_BMR_POSEN | TC_BMR_EDGPHA | TC_BMR_MAXFILT(1);
	//Step 5. Start TC0 and TC1
	REG_TC0_CCR0 |= TC_CCR_CLKEN;
	REG_TC0_CCR1 |= TC_CCR_CLKEN;
	//The QDEC using channels 0 and 1 of TC0 is enabled and configured.
	
	
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
	uint32_t HallA = 0;
	uint32_t HallB = 0;
	uint32_t HallC = 0;
	
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
	
	//This is the loop that will run when the motor just boots up.
	//It has not yet caught the index, so the vector control won't work yet
	//This loop just runs the motor very slowly using 6 step commutation using the Hall sensors
	//Set output pins to be output
	REG_PIOD_PER |= PIO_PER_P20 | PIO_PER_P21 | PIO_PER_P22 | PIO_PER_P24 | PIO_PER_P25 | PIO_PER_P26;
	REG_PIOD_OER |= PIO_OER_P20 | PIO_OER_P21 | PIO_OER_P22 | PIO_OER_P24 | PIO_OER_P25 | PIO_OER_P26;
	//Set the Hall  Pins to be inputs. See diagram. 
	//HallA is connected to PD30.
	//HallB is connected to PA07.
	//HallC is connected to PA08.
	REG_PIOD_PER |= PIO_PER_P30;
	REG_PIOA_PER |= PIO_PER_P7 | PIO_PER_P8;
	REG_PIOD_ODR |= PIO_ODR_P30;
	REG_PIOA_ODR |= PIO_ODR_P7 | PIO_ODR_P8;
	while (!HasBeenIndex)
	{
		//Measure the Hall sensor outputs
		HallA = (REG_PIOD_PDSR & PIO_PDSR_P30);
		HallB = (REG_PIOA_PDSR & PIO_PDSR_P7);
		HallC = (REG_PIOA_PDSR & PIO_PDSR_P8);
		//Energize the windings in accordance with simple six step commutation
		SixStepCommutation(000,000,0,HallA,HallB,HallC);
		//Check if the index has been passed.
		HasBeenIndex = REG_TC0_CV1;
	}
	
	
	//Putting PWM setup code below the six step section so that the pins are not given to the peripheral
	
	//Set up PWM
	//Need to do PIO stuff for PWM enabling and activate the peripheral clock
	//Give the relevant pins to the PWM peripheral. (PIO stuff)
	REG_PIOD_ABCDSR1 |= PIO_ABCDSR_P20 | PIO_ABCDSR_P21 | PIO_ABCDSR_P22 | PIO_ABCDSR_P24 | PIO_ABCDSR_P25 | PIO_ABCDSR_P26;
	//Active the PWM clocks (PMC stuff)
	REG_PMC_PCER1 |= PMC_PCER1_PID36;
	//Following steps in the datasheet 39.6.5.1
	//Disable write protection of PWM registers
	REG_PWM_WPCR = PWM_WPCR_WPKEY(0x50574D) | PWM_WPCR_WPCMD(0x0);
	//Configure clock generator
	//Select CLKA clock. Set PREA clock to be the peripheral clock
	//Don't actually need this
	//REG_PWM_CLK |= PWM_CLK_DIVA(1) | PWM_CLK_PREA(0);
	//Select the clock for each channel. Will use CLKA dealt with just above.
	//Also set the alignment, polarity, deadtime, update type, event selection
	//Set for no deadtime, the FET driver will take care of that.
	//Register starts at zero, so CPOL bit doesn't need to change. CES also should be 0.
	//Probably don't need to set up 1 and 2, since I'm going to sync them, but oh well
	REG_PWM_CMR0 |= PWM_CMR_CPRE_MCK | PWM_CMR_CALG;
	REG_PWM_CMR1 |= PWM_CMR_CPRE_MCK | PWM_CMR_CALG;
	REG_PWM_CMR2 |= PWM_CMR_CPRE_MCK | PWM_CMR_CALG;
	//Set period of PWM
	//At 24 V, 4800 give me a resolution of 5 mV
	//I am going to assert this is enough
	REG_PWM_CPRD0 = PWMMAXVALUE;
	REG_PWM_CPRD1 = PWMMAXVALUE;
	REG_PWM_CPRD2 = PWMMAXVALUE;
	//Init the duty cycle at 0
	REG_PWM_CDTY0 = 0;
	REG_PWM_CDTY1 = 0;
	REG_PWM_CDTY2 = 0;
	//Sync channels 1 and 2 to channel 0
	//Also set update method. Default is manual update, so that's fine
	REG_PWM_SCM |= PWM_SCM_SYNC0 | PWM_SCM_SYNC1 | PWM_SCM_SYNC2;
	//Enable the interrupt channels. Might need them
	REG_PWM_ENA |= PWM_ENA_CHID0 | PWM_ENA_CHID1 | PWM_ENA_CHID2;
	
	
	
	
	
	
	
	

    /* Replace with your application code */
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
}