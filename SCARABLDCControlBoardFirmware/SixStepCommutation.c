#include "sam.h"
#define HALLAUPPER 24
#define HALLALOWER 20
#define HALLBUPPER 25
#define HALLBLOWER 21
#define HALLCUPPER 26
#define HALLCLOWER 22

//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)

void SixStepCommutation(uint32_t OnMax, uint32_t OffMax, uint32_t Direction, uint32_t HallA, uint32_t HallB, uint32_t HallC)
{
	
	

	//For each Hall Sensor configuration, need to activate the correct pins via PWM
	//So, Need to give the pins that will be high to the PWM controller
	//All other pins will be given/left with the PIO for GPIO control
	//The high pins will be synchronized to Counter0
	
	
	
	//pick which outputs to turn on
	//Direction == 0  --> Clockwise
	//Direction == 1  --> AntiClockwise
	if (Direction <= 2)
	{
		
		if ((HallA == 0) && (HallB == 0) && (HallC > 0))//001
		{
			//This register controls which pins are controlled by the PWM.
			//In this case, we want Cupper, Clower, and Blower, to be controlled by the PWM.
			//The rest are GPIO
			REG_PIOD_ABCDSR1 |= PIO_ABCDSR_P21 | PIO_ABCDSR_P22 | PIO_ABCDSR_P26;
			
			//Now, set the duty cycle for both C and B (W and V) to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			
			//REG_PIOD_SODR = (1<<HALLCUPPER) | (1<<HALLBLOWER);
			REG_PIOD_CODR = (1<<HALLALOWER) | (1<<HALLBUPPER) |(1<<HALLAUPPER);// | (1<<HALLCLOWER);
			//|+|-|float|
		}else if ((HallA > 0) && (HallB == 0) && (HallC > 0))//101
		{
			REG_PIOD_SODR = (1<<HALLAUPPER) | (1<<HALLBLOWER);
			REG_PIOD_CODR = (1<<HALLALOWER) | (1<<HALLCUPPER) | (1<<HALLBUPPER) | (1<<HALLCLOWER);
			//|+|float|-|
		}else if ((HallA > 0) && (HallB == 0) && (HallC == 0))//100
		{
			REG_PIOD_SODR = (1<<HALLAUPPER) | (1<<HALLCLOWER);
			REG_PIOD_CODR = (1<<HALLBLOWER) | (1<<HALLCUPPER) | (1<<HALLBUPPER) | (1<<HALLALOWER);
			//|float|+|-|
		}else if ((HallA > 0) && (HallB > 0) && (HallC == 0))//110
		{
			REG_PIOD_SODR = (1<<HALLCLOWER) | (1<<HALLBUPPER);
			REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBLOWER) | (1<<HALLCUPPER) | (1<<HALLALOWER);
			//|-|+|float|
		}else if ((HallA == 0) && (HallB > 0) && (HallC == 0))//010
		{
			REG_PIOD_SODR = (1<<HALLALOWER) | (HALLBUPPER);
			REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLCLOWER) | (1<<HALLCUPPER) | (1<<HALLBLOWER);
			//|-|float|-|
		}else if ((HallA == 0) && (HallB > 0) && (HallC > 0))//011
		{
			REG_PIOD_SODR = (1<<HALLALOWER) | (1<<HALLCUPPER);
			REG_PIOD_CODR = (1<<HALLBUPPER) | (1<<HALLCLOWER) | (1<<HALLAUPPER) | (1<<HALLBLOWER);
			//|float|-|+|
		}else
		{
			
			REG_PIOD_SODR = (1<<HALLBLOWER) | (1<<HALLCLOWER) | (1<<HALLALOWER);
			REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);
		}
	}else if (Direction > 2)
	{
			REG_PIOD_SODR = (1<<HALLBLOWER) | (1<<HALLCLOWER) | (1<<HALLALOWER);
			REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);
	}else
	{
			REG_PIOD_SODR = (1<<HALLBLOWER) | (1<<HALLCLOWER) | (1<<HALLALOWER);
			REG_PIOD_CODR = (1<<HALLAUPPER) | (1<<HALLBUPPER) | (1<<HALLCUPPER);
	}
	
	//Wait some more
	for (uint32_t i = 0; i<OnMax;i++)
	{
	}
	
	//Set the outputs to 0.
	//This really sets all FET outputs to 0.
	REG_PIOD_CODR = 0xFFFFFFFF;
	
	//wait some time
	//The waits are to make a (very) rough PWM to keep motor speed down
	for (volatile uint32_t i = 0; i<OffMax;i++)
	{
	}
	
}
