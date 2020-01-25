#include "sam.h"
#define HALLAUPPER 24
#define HALLALOWER 20
#define HALLBUPPER 25
#define HALLBLOWER 21
#define HALLCUPPER 26
#define HALLCLOWER 22

void SixStepCommutation(uint32_t OnMax, uint32_t OffMax, uint32_t Direction, uint32_t HallA, uint32_t HallB, uint32_t HallC)
{
	
	

	
	//pick which outputs to turn on
	//Direction == 0  --> Clockwise
	//Direction == 1  --> AntiClockwise
	if (Direction <= 2)
	{
		
		if ((HallA == 0) && (HallB == 0) && (HallC > 0))//001
		{
			REG_PIOD_SODR = (1<<HALLCUPPER) | (1<<HALLBLOWER);
			REG_PIOD_CODR = (1<<HALLALOWER) | (1<<HALLBUPPER) | (1<<HALLCLOWER) | (1<<HALLAUPPER);
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
