#include "sam.h"

//ABSOLUTELY NEED TO ADJUST THE PCB TO BE THE OPPOSITE OF THIS
//THESE CURRENT DEFINITIONS ARE OPPOSITE OF THE INTERNAL PWM HIGH AND LOW OUTPUT
//OBVIOUSLY IT CAN BE MADE TO WORK, BUT IT'S JUST ONE MORE THING TO REMEMBER
//EASIER TO JUST NOT DO THAT
#define U_UPPER_OUTPUT 20//PWM channel 0
#define U_LOWER_OUTPUT 24
#define V_UPPER_OUTPUT 21//PWM channel 1
#define V_LOWER_OUTPUT 25
#define W_UPPER_OUTPUT 22//PWM channel 2
#define W_LOWER_OUTPUT 26

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
	
	//For PWM complementary outputs:
	//When non-inverted (CPOL=0), LOW OUTPUT has the defined duty cycle, and HIGH OUTPUT has 1-duty cycle
	//When inverted (CPOL=1), LOW OUTPUT has 1-duty cycle, and HIGH OUTPUT has the defined duty cycle
	//What this means for this function is that the pins controlling the (-) output should be non-inverted
	//and the pins controlling the (+) output should be inverted
	
	REG_PIOD_OER = 0xFFFFFFFF;
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
			REG_PIOD_ABCDSR1 |= (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT) | (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_PDR |= (1<<W_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<V_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);//U Output non-inverted
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);//V Output non-inverted
			REG_PWM_CMR2 |= PWM_CMR_CPOL;//W Output inverted
			//Now, set the duty cycle for both W and V to be pretty low, probs 15%.
			//Not using the update registers since the PWM is disabled above.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			//REG_PIOD_SODR = (1<<HALLCUPPER) | (1<<HALLBLOWER);
			//REG_PIOD_OER |= (1<<U_LOWER_OUTPUT) | (1<<V_UPPER_OUTPUT) |(1<<U_UPPER_OUTPUT);
			REG_PIOD_CODR |= (1<<U_LOWER_OUTPUT) | (1<<V_UPPER_OUTPUT) |(1<<U_UPPER_OUTPUT);
			//|float|-|+|
		}else if ((HallA > 0) && (HallB == 0) && (HallC > 0))//101
		{
			//Need Aupper, Alower, and Blower to be controlled by PWM
			REG_PIOD_ABCDSR1 |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<V_LOWER_OUTPUT);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			REG_PIOD_PDR |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<V_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			//REG_PIOD_SODR = (1<<HALLAUPPER) | (1<<HALLBLOWER);
			REG_PIOD_CODR =  (1<<W_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			//|+|-|float|
		}else if ((HallA > 0) && (HallB == 0) && (HallC == 0))//100
		{
			//Need Aupper, Alower, and Clower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			REG_PIOD_PDR |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;		
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			//REG_PIOD_SODR = (1<<A_UPPER_OUTPUT) | (1<<C_LOWER_OUTPUT);
			REG_PIOD_CODR = (1<<V_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT);
			//|+|float|-|
		}else if ((HallA > 0) && (HallB > 0) && (HallC == 0))//110
		{
			//Need Bupper, Blower, and Clower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			REG_PIOD_PER |= (1<<U_UPPER_OUTPUT) | (1<<U_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			REG_PIOD_PDR |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;		
			
			//REG_PIOD_SODR = (1<<C_LOWER_OUTPUT) | (1<<B_UPPER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			//|float|+|-|
		}else if ((HallA == 0) && (HallB > 0) && (HallC == 0))//010
		{
			//Need Bupper, Blower, and Alower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_PER |= (1<<U_UPPER_OUTPUT) | (1<<W_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT);
			REG_PIOD_PDR |= (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);

			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;		
			
			//REG_PIOD_SODR = (1<<A_LOWER_OUTPUT) | (B_UPPER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<W_UPPER_OUTPUT);
			//|-|+|float|
		}else if ((HallA == 0) && (HallB > 0) && (HallC > 0))//011
		{
			//Need Cupper, Clower, and Alower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<W_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_PER |= (1<<U_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT);
			REG_PIOD_PDR |= (1<<W_UPPER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 720;
			REG_PWM_CDTY1 = 720;
			REG_PWM_CDTY2 = 720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;		
			
			//REG_PIOD_SODR = (1<<A_LOWER_OUTPUT) | (1<<C_UPPER_OUTPUT);
			REG_PIOD_CODR = (1<<V_UPPER_OUTPUT) | (1<<U_UPPER_OUTPUT) | (1<<V_LOWER_OUTPUT);
			//|-|float|+|
		}else
		{
			
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT) | (1<<W_UPPER_OUTPUT);
		}
	}else if (Direction > 2)
	{
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT) | (1<<W_UPPER_OUTPUT);
	}else
	{
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT) | (1<<W_LOWER_OUTPUT) | (1<<U_LOWER_OUTPUT);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT) | (1<<V_UPPER_OUTPUT) | (1<<W_UPPER_OUTPUT);
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
