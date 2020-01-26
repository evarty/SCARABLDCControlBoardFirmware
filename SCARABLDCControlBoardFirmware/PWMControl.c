#include "sam.h"

void UpdateOutputPWMDutyCycles(uint32_t PWMValues[])
{
	
	//I am assuming that this function will be called only at certain intervals
	//It disables the PWM, sets the needed polarity and duty cycle, then re-enables them
	//This is non-ideal, but the polarity needs to change a lot and this chip doesn't have enough independent
	//PWM channels to control each one by itself, so I'm using the complementary pairs.
	//This should probably be called by an interrupt when the counter hits 0.
	
	
	//Because I need to adjust the polarity a lot, this actually disables the PWM, sets the polarity and duty cycles, then re-enables them
	//Disable synced PWM channels
	REG_PWM_ENA = 0x0;
	//Set duty cycles
	REG_PWM_CDTY0 = PWMValues[0];
	REG_PWM_CDTY1 = PWMValues[1];
	REG_PWM_CDTY2 = PWMValues[2];
	//Set polarity
	REG_PWM_CMR0 = (REG_PWM_CMR0 & ~(1 << 9)) | (PWMValues[3] << 9);
	REG_PWM_CMR1 = (REG_PWM_CMR1 & ~(1 << 9)) | (PWMValues[3] << 9);
	REG_PWM_CMR2 = (REG_PWM_CMR2 & ~(1 << 9)) | (PWMValues[3] << 9);
	//Enable synced channels
	REG_PWM_ENA |= PWM_ENA_CHID0 | PWM_ENA_CHID1 | PWM_ENA_CHID2;
	//REG_PWM_SCUC |= PWM_SCUC_UPDULOCK;
	
}