#include "sam.h"

//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)

void UpdateOutputPWMDutyCycles(uint32_t PWMValues[])
{
	
	//I am assuming that this function will be called only at certain intervals
	//It disables the PWM, sets the needed polarity and duty cycle, then re-enables them
	//This is non-ideal, but the polarity needs to change a lot and this chip doesn't have enough independent
	//PWM channels to control each one by itself, so I'm using the complementary pairs.
	//This should probably be called by an interrupt when the counter hits 0.
	
	
	//Because I need to adjust the polarity a lot, this actually disables the PWM, sets the polarity and duty cycles, then re-enables them
	//Disable synced PWM channels
	REG_PWM_ENA = 0x0U;
	//Set duty cycles
	REG_PWM_CDTY0 = PWMValues[0];
	REG_PWM_CDTY1 = PWMValues[1];
	REG_PWM_CDTY2 = PWMValues[2];
	//Set polarity
	REG_PWM_CMR0 = (REG_PWM_CMR0 & ~(1 << 9)) | (PWMValues[3] << 9);
	REG_PWM_CMR1 = (REG_PWM_CMR1 & ~(1 << 9)) | (PWMValues[4] << 9);
	REG_PWM_CMR2 = (REG_PWM_CMR2 & ~(1 << 9)) | (PWMValues[5] << 9);
	//Enable synced channels
	REG_PWM_ENA |= PWM_ENA_CHID0;
	//REG_PWM_SCUC |= PWM_SCUC_UPDULOCK;
	
}

void PWMSetup(){
	
	
	//Set up PWM
	//Need to do PIO stuff for PWM enabling and activate the peripheral clock
	//Give the relevant pins to the PWM peripheral. (PIO stuff)
	REG_PIOD_ABCDSR1 |= (0<<20) | (0<<21) | (0<<22) | (0<<24) | (0<<25) | (0<<26);
	REG_PIOD_PDR = 0xFFFFFFFF;
	//Active the PWM clocks (PMC stuff)
	REG_PMC_PCER1 |= PMC_PCER1_PID36;
	REG_PMC_PCER0 |= PMC_PCER0_PID21 | PMC_PCER0_PID22 | PMC_PCER0_PID23;
	
	//Following steps in the datasheet 39.6.5.1
	//Disable write protection of PWM registers
	REG_PWM_WPCR = PWM_WPCR_WPKEY(0x50574D) | PWM_WPCR_WPCMD(0x0U);
	
	
	//Select the clock 
	//Also set the alignment, polarity, deadtime, update type, event selection
	//Set for no deadtime, the FET driver will take care of that.
	//Register starts at zero, so CPOL bit doesn't need to change. CES also should be 0.
	REG_PWM_CMR0 |= PWM_CMR_CPRE_MCK | PWM_CMR_CALG;
	
	//Set period of PWM
	//At 24 V, 4800 give me a resolution of 5 mV
	//I am going to assert this is enough
	REG_PWM_CPRD0 = 4800;
	
	
	//Init the duty cycle at 0
	REG_PWM_CDTY0 = 1200;
	REG_PWM_CDTY1 = 1200;
	REG_PWM_CDTY2 = 1200;
	
	//From 39.6.2.9 for synchronous channels
	REG_PWM_SCM = (PWM_SCM_UPDM_MODE0);
	REG_PWM_SCM |= PWM_SCM_SYNC0 | PWM_SCM_SYNC1 | PWM_SCM_SYNC2;
	//Don't need to change PWM_SCUP at all
	
	
	REG_PWM_ENA |= PWM_ENA_CHID0;
	
	
}
