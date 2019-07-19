#include "sam.h"
//The atmel headers think this is a RO register. The datasheet says it's RW
#define REG_AFEC0_CSELR (*(RwReg*)0x400B0064U)
#define REG_AFEC1_CSELR (*(RwReg*)0x400B4064U)

uint32_t ADC0Read(uint32_t Channel)
{
	//Set the ADC channel
	REG_AFEC0_CSELR = (0xF & Channel) << AFE_CSELR_CSEL_Pos;
	//Start ADC conversion
	REG_AFEC0_CR |= AFE_CR_START;
	//wait for conversion to complete
	while(!(REG_AFEC0_ISR & (1 << Channel)));
	
	return REG_AFEC0_CDR & 0x0000FFFF;
}


uint32_t ADC1Read(uint32_t Channel)
{
	//Set the ADC channel
	REG_AFEC1_CSELR |= (uint8_t)Channel << AFE_CSELR_CSEL_Pos;
	//Start ADC conversion
	REG_AFEC1_CR |= AFE_CR_START;
	//wait for conversion to complete
	while(!(REG_AFEC1_ISR & (1 << Channel)));
	
	return REG_AFEC1_CDR & 0x0000FFFF;
}