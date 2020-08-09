#include "sam.h"
#include <math.h>
#include "Utils.h"
#include "Transforms.h"
#include "QDEC.h"
#include "SVPWM.h"
#include "PWMControl.h"
#include "ADC.h"
#include "SixStepCommutation.h"

//ABSOLUTELY NEED TO ADJUST THE PCB TO BE THE OPPOSITE OF THIS
//THESE CURRENT DEFINITIONS ARE OPPOSITE OF THE INTERNAL PWM HIGH AND LOW OUTPUT
//OBVIOUSLY IT CAN BE MADE TO WORK, BUT IT'S JUST ONE MORE THING TO REMEMBER
//EASIER TO JUST NOT DO THAT
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

//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)

void SixStepCommutation(uint32_t OnMax, uint32_t OffMax, uint32_t Direction, uint32_t HallU, uint32_t HallV, uint32_t HallW)
{
	uint32_t temp = 4700;
	uint32_t hold[] = {temp,temp,temp};

	//For each Hall Sensor configuration, need to activate the correct pins via PWM
	//So, Need to give the pins that will be high to the PWM controller
	//All other pins will be given/left with the PIO for GPIO control
	//The high pins will be synchronized to Counter0
	
	//For PWM complementary outputs:
	//When non-inverted (CPOL=0), LOW OUTPUT has the defined duty cycle, and HIGH OUTPUT has 1-duty cycle
	//When inverted (CPOL=1), LOW OUTPUT has 1-duty cycle, and HIGH OUTPUT has the defined duty cycle
	//What this means for this function is that the pins controlling the (-) output should be non-inverted
	//and the pins controlling the (+) output should be inverted
	
	//REG_PIOD_OER |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
	//REG_PIOD_PER |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
	//pick which outputs to turn on
	//Direction == 0  --> Clockwise
	//Direction == 1  --> AntiClockwise
	if (Direction <= 2)
	{
		
		//commutation sequence
		//https://e2e.ti.com/blogs_/b/industrial_strength/archive/2013/11/08/generate-your-own-commutation-table-trapezoidal-control-3-phase-bldc-motors-using-hall-sensors
		//https://www.nxp.com/docs/en/application-note/AN4058.pdf
		if ((HallU == 0) && (HallV == 0) && (HallW > 0))//001
		{
			//This register controls which pins are controlled by the PWM.
			//In this case, we want Wupper, Wlower, and Vlower, to be controlled by the PWM.
			//The rest are GPIO
			//REG_PIOD_ABCDSR1 |= (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			//REG_PIOD_PDR |= (1<<V_UPPER_OUTPUT_PIN) | (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			//REG_PIOD_PER |= (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			
			/*
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);//U Output non-inverted
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);//V Output non-inverted
			REG_PWM_CMR2 |= PWM_CMR_CPOL;//W Output inverted
			//Now, set the duty cycle for both W and V to be pretty low, probs 15%.
			//Not using the update registers since the PWM is disabled above.
			REG_PWM_CDTY0 = 4500;//720;
			REG_PWM_CDTY1 = 4500;//720;
			REG_PWM_CDTY2 = 4500;//720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			*/
			
			
			EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			DisablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			DisablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			EnablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			
			
			//REG_PIOD_SODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//REG_PIOD_CODR |= (1<<U_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			//|float|-|+|
		}else if ((HallU > 0) && (HallV == 0) && (HallW > 0))//101
		{
			//Need Uupper, Ulower, and Vlower to be controlled by PWM
			//REG_PIOD_ABCDSR1 |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//REG_PIOD_PDR |= (1<<V_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			//REG_PIOD_PER |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			
			/*
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 4500;//720;
			REG_PWM_CDTY1 = 4500;//720;
			REG_PWM_CDTY2 = 4500;//720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			*/
			
			EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			DisablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			DisablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			EnablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			
			
			
			
			//REG_PIOD_SODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			//REG_PIOD_CODR =  (1<<W_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			//|+|-|float|
		}else if ((HallU > 0) && (HallV == 0) && (HallW == 0))//100
		{
			/*
			//Need Uupper, Ulower, and Wlower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			REG_PIOD_PDR |= (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			REG_PIOD_PER |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 4500;//720;
			REG_PWM_CDTY1 = 4500;//720;
			REG_PWM_CDTY2 = 4500;//720;		
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;
			*/
			
			
			
			EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			DisablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			DisablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			EnablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			
			
			
			//REG_PIOD_SODR = (1<<V_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			//REG_PIOD_CODR = (1<<U_LOWER_OUTPUT_PIN) | (1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//|+|float|-|
		}else if ((HallU > 0) && (HallV > 0) && (HallW == 0))//110
		{
			/*
			//Need Vupper, Vlower, and Wlower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			REG_PIOD_PDR |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 4500;//720;
			REG_PWM_CDTY1 = 4500;//720;
			REG_PWM_CDTY2 = 4500;//720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;	
			*/	
			
			
			
			DisablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			DisablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			EnablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;
			
			
			
			//REG_PIOD_SODR = (1<<U_LOWER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN);
			//REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (V_LOWER_OUTPUT_PIN);
			//|float|+|-|
		}else if ((HallU == 0) && (HallV > 0) && (HallW == 0))//010
		{
			/*
			//Need Vupper, Vlower, and Ulower to be PWM
			REG_PIOD_ABCDSR1 |= (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			REG_PIOD_PDR |= (1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			REG_PIOD_PER |= (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);

			//Disable PWM because polarity needs to be adjusted
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 &= ~(PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			REG_PWM_CDTY0 = 4500;//720;
			REG_PWM_CDTY1 = 4500;//720;
			REG_PWM_CDTY2 = 4500;//720;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			REG_PWM_ENA |= PWM_ENA_CHID0;	
			*/	
			
			
			DisablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			DisablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;	
			
			
			
			//REG_PIOD_SODR = (1<<U_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			//REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//|-|float|+|
		}else if ((HallU == 0) && (HallV > 0) && (HallW > 0))//011
		{
			
			EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			
			DisablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			
			EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			DisablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			
			REG_PIOD_CODR = 0xFFFFFFFF;//(1<<U_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			
			REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			REG_PWM_CMR1 &= ~(PWM_CMR_CPOL);
			REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			
			UpdateOutputPWMDutyCycles(hold);
			
			REG_PWM_ENA |= PWM_ENA_CHID0;	
			
			/*
			//Need Wupper and Vlower to be PWM
			//REG_PIOD_ABCDSR1 |= (1<<W_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//Disable PIO control of Wupper and Vlower (This gives control to the selected Peripheral, which is PWM)
			DisablePIOControl(V_LOWER_OUTPUT_PORT, V_LOWER_OUTPUT_PIN);
			DisablePIOControl(W_UPPER_OUTPUT_PORT, W_UPPER_OUTPUT_PIN);
			//REG_PIOD_PDR |= (1<<W_UPPER_OUTPUT_PIN) | (1<<V_LOWER_OUTPUT_PIN);
			//Enable PIO control of the remaining pins and set them all low.
			EnablePIOControl(U_UPPER_OUTPUT_PORT, U_UPPER_OUTPUT_PIN);
			EnablePIOControl(U_LOWER_OUTPUT_PORT, U_LOWER_OUTPUT_PIN);
			EnablePIOControl(V_UPPER_OUTPUT_PORT, V_UPPER_OUTPUT_PIN);
			EnablePIOControl(W_LOWER_OUTPUT_PORT, W_LOWER_OUTPUT_PIN);
			//REG_PIOD_PER |= (1<<U_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) ;
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) ;
			
			//Disable PWM because polarity needs to be adjusted
			//REG_PWM_DIS |= PWM_DIS_CHID0;
			//Set needed polarities. Setting all of them so I don't need to keep track.
			//REG_PWM_CMR0 |= (PWM_CMR_CPOL);
			//REG_PWM_CMR1 |= (PWM_CMR_CPOL);
			//REG_PWM_CMR2 |= (PWM_CMR_CPOL);
			//Now, set the duty cycle to be pretty low, probs 15%.
			
			UpdateOutputPWMDutyCycles(hold);
			//REG_PWM_CDTY0 = 00;
			//REG_PWM_CDTY1 = 00;
			//REG_PWM_CDTY2 = 00;
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;	
			//Enable PWM
			//REG_PWM_ENA |= PWM_ENA_CHID0;	
			*/
			
				
			
			//REG_PIOD_SODR = (1<<V_LOWER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
			//REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN);
			//|float|-|+|
		}else
		{
			
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
		}
	}else if (Direction > 2)
	{
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
	}else
	{
			REG_PIOD_SODR = (1<<V_LOWER_OUTPUT_PIN) | (1<<W_LOWER_OUTPUT_PIN) | (1<<U_LOWER_OUTPUT_PIN);
			REG_PIOD_CODR = (1<<U_UPPER_OUTPUT_PIN) | (1<<V_UPPER_OUTPUT_PIN) | (1<<W_UPPER_OUTPUT_PIN);
	}
	
	//Wait some more
	for (volatile uint32_t i = 0; i<OnMax;i++)
	{
	}
	
	//Set the outputs to 0.
	//This really sets all FET outputs to 0.
	//REG_PIOD_CODR = 0xFFFFFFFF;
	
	//wait some time
	//The waits are to make a (very) rough PWM to keep motor speed down
	//for (volatile uint32_t i = 0; i<OffMax;i++)
	//{
	//}
	
}
