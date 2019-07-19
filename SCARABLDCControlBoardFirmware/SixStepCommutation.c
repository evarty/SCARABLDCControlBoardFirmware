#include "sam.h"
#define HALLAUPPER 20
#define HALLALOWER 24
#define HALLBUPPER 21
#define HALLBLOWER 25
#define HALLCUPPER 22
#define HALLCLOWER 26

void SixStepCommutation(uint32_t OnMax, uint32_t OffMax, uint32_t Direction, uint32_t HallA, uint32_t HallB, uint32_t HallC)
{
	
	/*
	//wait some time
	//The waits are to make a (very) rough PWM to keep motor speed down
	for (uint32_t i = 0; i<OffMax;i++)
	{
		asm("nop");
	}
	*/
	//pick which outputs to turn on
	//Direction == 0  --> Clockwise
	//Direction == 1  --> AntiClockwise
	if (Direction == 0)
	{
		if ((HallA == 0) && (HallB == 0) && (HallC > 0))//001
		{
			REG_PIOD_SODR = (1<<HALLAUPPER) | (1<<HALLBLOWER);
			//|+|-|float|
		}else if ((HallA > 0) && (HallB == 0) && (HallC > 0))//101
		{
			REG_PIOD_SODR = (1<<HALLAUPPER) | (1<<HALLCLOWER);
			//|+|float|-|
		}else if ((HallA > 0) && (HallB == 0) && (HallC == 0))//100
		{
			REG_PIOD_SODR = (1<<HALLBUPPER) | (1<<HALLCLOWER);
			//|float|+|-|
		}else if ((HallA > 0) && (HallB > 0) && (HallC == 0))//110
		{
			REG_PIOD_SODR = (1<<HALLALOWER) | (1<<HALLBUPPER);
			//|-|+|float|
		}else if ((HallA == 0) && (HallB > 0) && (HallC == 0))//010
		{
			REG_PIOD_SODR = (1<<HALLALOWER) | (HALLCUPPER);
			//|-|float|-|
		}else if ((HallA > 0) && (HallB == 0) && (HallC == 0))//011
		{
			REG_PIOD_SODR = (1<<HALLBLOWER) | (1<<HALLCUPPER);
			//|float|-|+|
		}
	}else if (Direction > 0)
	{
	}
	/*
	//Wait some more
	for (uint32_t i = 0; i<OnMax;i++)
	{
		asm("nop");
	}
	*/
	//Set the outputs to 0
	REG_PIOD_CODR = 0xFFFFFFFF;
	
}
