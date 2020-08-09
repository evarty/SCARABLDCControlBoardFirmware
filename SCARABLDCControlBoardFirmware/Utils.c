#include "sam.h"
#include <math.h>
#include "Utils.h"


//Constructs an array of all needed sine values.
void SineArray(float SinArray[], uint32_t EncoderResolution){
	
	for(int i = 0; i < EncoderResolution; i++){
		SinArray[i] = sin((float) i/2000.*6.283185);
	}
	
}

//Constructs an array of all needed cosine values.
void CosineArray(float CosArray[], uint32_t EncoderResolution){
	
	for(int i = 0; i < EncoderResolution; i++){
		CosArray[i] = cos((float) i/2000.*6.283185);
	}
	
}

//Port: A = 0, B = 1, C = 2, D = 3
void DisablePIOWriteProtection(uint32_t port){
	if (port == 0)
	{
		REG_PIOA_WPMR = PIO_WPMR_WPKEY(0x50494fU) & ~(1<<0);
	}else if (port == 1)
	{
		REG_PIOB_WPMR = PIO_WPMR_WPKEY(0x50494fU) & ~(1<<0);
	}else if (port == 2)
	{
		REG_PIOC_WPMR = PIO_WPMR_WPKEY(0x50494fU) & ~(1<<0);
	}else if (port == 3)
	{
		REG_PIOD_WPMR = PIO_WPMR_WPKEY(0x50494fU) & ~(1<<0);
	}
}

//Port: A = 0, B = 1, C = 2, D = 3
//Direction: 0 = Input, 1 = Output
void SetPinIODirection(uint32_t port, uint32_t pin, uint32_t Direction){
	//DisablePIOWriteProtection(port);
	
	if(Direction == 0)//Input
	{
		if (port == 0)
		{
			REG_PIOA_ODR |= (1<<pin);//Disable the output, which makes the pin an input
		}else if (port == 1)
		{
			REG_PIOB_ODR |= (1<<pin);
		}else if (port == 2)
		{
			REG_PIOC_ODR |= (1<<pin);
		}else if (port == 3)
		{
			REG_PIOD_ODR |= (1<<pin);
		}
	}else if (Direction == 1)
	{
		if (port == 0)
		{
			REG_PIOA_OER |= (1<<pin);//Enable the output
		}else if (port == 1)
		{
			REG_PIOB_OER |= (1<<pin);
		}else if (port == 2)
		{
			REG_PIOC_OER |= (1<<pin);
		}else if (port == 3)
		{
			REG_PIOD_OER |= (1<<pin);
		}
	}
}

//Port: A = 0, B = 1, C = 2, D = 3
//value: 0 = off, 1 = on
void SetPinIOOutputValue(uint32_t port, uint32_t pin, uint32_t value){
	if (value == 1)
	{
		if (port == 0)
		{
			REG_PIOA_SODR |= (1 << pin);
		}else if (port == 1)
		{
			REG_PIOB_SODR |= (1 << pin);
		}else if (port == 2)
		{
			REG_PIOC_SODR |= (1 << pin);
		}else if (port == 3)
		{
			REG_PIOD_SODR |= (1 << pin);
		}
	}else if (value == 0)
	{
		if (port == 0)
		{
			REG_PIOA_CODR |= (1<<pin);
		}else if (port == 1)
		{
			REG_PIOB_CODR |= (1<<pin);
		}else if (port == 2)
		{
			REG_PIOC_CODR |= (1<<pin);
		}else if (port == 3)
		{
			REG_PIOD_CODR |= (1<<pin);
		}
	}
}

//Port: A = 0, B = 1, C = 2, D = 3
//value: 0 = off, 1 = on
uint32_t ReadIOPinValue(uint32_t port, uint32_t pin){
	uint32_t value = 0;
	
	if (port == 0)
	{
		value = (REG_PIOA_PDSR & (1<<pin));
	}else if (port == 1)
	{
		value = (REG_PIOB_PDSR & (1<<pin));
	}else if (port == 2)
	{
		value = (REG_PIOC_PDSR & (1<<pin));
	}else if (port == 3)
	{
		value = (REG_PIOD_PDSR & (1<<pin));
	}
	return value;
}


void ClockSetup(){
	//Some of this is taken from/related to the systeminit function provided by atmel studio
	//Like this line
	//The provided headers to not define REG_EEFC_FMR so....
	//I defined it myself. address from 20.5.1
	(*(RwReg*)0x400E0A00U) |= ((0xF & 5) << 8);
	
	//Process is in several places in the datasheet. 28.5.7, 29.15 (which is the main one followed here)
	//Not mentioned in the section, but the write protection must be disabled
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43) & ~(1<<0);
	
	//folowing 28.5.7 since I need to enable the RC oscillator
	//step 1. Select the slow clock as main clock
	//Can just set the whole register to 0, since the over value don't matter right now.
	REG_PMC_MCKR = 0x0;
	//step 2. wait for the main clock to be ready
	while (!(REG_PMC_SR & PMC_SR_MCKRDY))
	{
	}
	//step 3. enable the crystal oscillator. This is also step 2 in 19.15
	//----29.15 step 2. Enable the oscillator (with the maximum startup time). also enable the RC oscillator
	REG_CKGR_MOR = CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCXTST(0xFF) | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCRCEN;//leave MOSCREF at default. Don't actually care about this, but datasheet says it needs to be on.
	//step 4. wait for crystal to stabilize by checking the PMC status register
	while (!(REG_PMC_SR & PMC_SR_MOSCXTS))
	{
	}
	//step 5. select the crystal as the main clock source
	REG_CKGR_MOR = CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCXTST(0xFF) | CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCSEL;
	//step 6. poll CKGR_MOL_MOSCEL until it is 1
	while (!(REG_CKGR_MOR & CKGR_MOR_MOSCSEL))
	{
	}
	//step 7. check the status of the main clock
	if (REG_PMC_SR & PMC_SR_MOSCSELS)
	{
		//step 7a. measure the main clock frequency
		REG_CKGR_MCFR |= CKGR_MCFR_RCMEAS;
		//step 7b. read mainfrdy until 1
		while(!(REG_CKGR_MCFR & CKGR_MCFR_MAINFRDY))
		{
		}
		//step 7c
		if ((REG_CKGR_MCFR & CKGR_MCFR_MAINF(0xFFFF)) >= 0x1700)//main clock should be 12MHz. slow clock is 32KHz. See 29.18.8
		{
			//step 7d. set the main clock as the crystal
			
		}else
		{
			REG_CKGR_MOR &= ~(CKGR_MOR_MOSCSEL);//disable the crystal and enable the RC oscillator
		}
	}
	//at this point, the crystal is selected as the main clock (unless it failed to boot up)
	//I now want the PLL to be activated.
	//Folowing 19.15 starting at step 6
	//Step 6. Setup PLL for div by 1 and mul by 10 (set for 9) and the max number of waiting cycles for lock
	REG_CKGR_PLLAR = (1<<29) | CKGR_PLLAR_MULA(0x9) | CKGR_PLLAR_DIVA(0x1) | CKGR_PLLAR_PLLACOUNT(0x3F);
	while(!(REG_PMC_SR & PMC_SR_LOCKA))
	{	
	}
	//step 7.
	//set the main clock to be PLLA with no prescalar or divby2. 
	//Datasheet gives a specific order to do this.
	REG_PMC_MCKR &= ~PMC_MCKR_PRES_Msk;//clears the PRES to select the clock with no prescale
	while(!(REG_PMC_SR & PMC_SR_MCKRDY))
	{
	}
	REG_PMC_MCKR |= PMC_MCKR_CSS_PLLA_CLK;
	while(!(REG_PMC_SR & PMC_SR_MCKRDY))
	{	
	}
	//step 8. I am not using the programmable clocks, so I will skip this.
	//step 9. Peripheral clocks will be enabled as needed by other code chunks.
	
	
	
	
	
	
	
	
	
	
	//OLD CLOCK CODE
	
	/*
	//Set up Crystal and main clock
	//This will occur only on startup, so speed is not a priority.
	//Following Process from datasheet section 29.15 Programming Sequence
	//Write protection starts as 0, so no need to deal with that 
	//Enable crystal oscillator
	REG_CKGR_MOR |= CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCXTEN;
	//Wait for MOSCXTST field in PMC_SR to be set
	while(!(REG_PMC_SR & PMC_SR_MOSCXTS));
	//Switch the main clock to the external crystal
	REG_CKGR_MOR |= CKGR_MOR_KEY(0x37) | CKGR_MOR_MOSCSEL;
	//Wait for MOSCELS bit to be set the PMC_SR register
	while(!(REG_PMC_SR & PMC_SR_MOSCSELS));
	//Check the clock frequency
	//Wait for check to be available
	while(!(REG_CKGR_MCFR & CKGR_MCFR_MAINFRDY));
	*/

	/*
	Check if the crystal is doing anything, 
	in particular, if it is close to the correct frequency,
	If not, revert to internal oscillator.
	The default slow clock is 4 MHz. The crystal in the design
	is 12 MHz. Reading from MAINF gives the number of crystal cycles
	in 16 slow clock cycles. The crystal is three times
	as fast as the slow clock. So, 16*3 = 48.
	*/

/*
	uint32_t MainFrequencyCheckVariable = 0x0u;
	MainFrequencyCheckVariable = REG_CKGR_MCFR & CKGR_MCFR_MAINF_Msk;
	if(MainFrequencyCheckVariable<5800)
	{
		REG_CKGR_MOR &= ~(CKGR_MOR_MOSCSEL);
	}
	//At this point, the crystal is working and the main clock source
	//but I want a faster clock, so, break out the PLL
	
	//Setup PLL
	//Write the count register for the PLL setup time
	//Set PLL multiplier. multiplication value is this value + 1
	REG_CKGR_PLLAR = CKGR_PLLAR_PLLACOUNT(0x03F) | CKGR_PLLAR_MULA(0x019) | CKGR_PLLAR_DIVA(0x01) | (1<<29);
	//Wait for the LOCKA bit to be set
	while(!(REG_PMC_SR & PMC_SR_LOCKA));
	//Select the PLL as master clock, following datasheet
	REG_PMC_MCKR |= PMC_MCKR_PRES_CLK_1;
	while(!(REG_PMC_SR & PMC_SR_MCKRDY));
	REG_PMC_MCKR |= PMC_MCKR_CSS_PLLA_CLK;
	while(!(REG_PMC_SR & PMC_SR_MCKRDY));
	//At this point the master clock is the PLL
	//which is (9 + 1)*12 MHz = 120 MHz.
	while(!(REG_CKGR_MCFR & CKGR_MCFR_MAINFRDY));

	
	MainFrequencyCheckVariable = REG_CKGR_MCFR & CKGR_MCFR_MAINF_Msk;
	if(MainFrequencyCheckVariable < 58000)
	{
		REG_CKGR_MOR &= ~(CKGR_MOR_MOSCSEL);
	}
	
	*/
}

void PIOSetup(){
	
	DisablePIOWriteProtection(0);//A
	DisablePIOWriteProtection(1);//B
	DisablePIOWriteProtection(2);//C
	DisablePIOWriteProtection(3);//D
	
	//Turn on PIO clock for all PIO channels. Because why not. Even E, which isn't on my chip
	REG_PMC_PCER0 |= PMC_PCER0_PID9 | PMC_PCER0_PID10 | PMC_PCER0_PID11 | PMC_PCER0_PID12 | PMC_PCER0_PID13;
	
	
	
	
	
	//OLD PIO SETUP CODE
	/*
	//Setup PIO
	//Disable PIO write protection
	REG_PIOA_WPMR = AFE_WPMR_WPKEY(0x50494F) & ~(1<<0);
	REG_PIOD_WPMR = AFE_WPMR_WPKEY(0x50494F) & ~(1<<0);
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
	*/
	
}


void EnablePIOControl(uint32_t port, uint32_t pin){
	
	if (port == 0)
		{
			REG_PIOA_PER |= (1<<pin);//Enable PIO control of the pin
		}else if (port == 1)
		{
			REG_PIOB_PER |= (1<<pin);
		}else if (port == 2)
		{
			REG_PIOC_PER |= (1<<pin);
		}else if (port == 3)
		{
			REG_PIOD_PER |= (1<<pin);
		}
}

void DisablePIOControl(uint32_t port, uint32_t pin){
	
	if (port == 0)
		{
			REG_PIOA_PDR |= (1<<pin);//Enable PIO control of the pin
		}else if (port == 1)
		{
			REG_PIOB_PDR |= (1<<pin);
		}else if (port == 2)
		{
			REG_PIOC_PDR |= (1<<pin);
		}else if (port == 3)
		{
			REG_PIOD_PDR |= (1<<pin);
		}
}







/*
//Multiplies matrices together
//This assumes that proper sized matrices are passed to it. Checks must be done outside the function.
void MatrixMultiply(float A[][], float B[][], uint32_t ADimRow, uint32_t ADimCol, uint32_t BDimRow, uint32_t BDimCol, float Result[][]){

    for(uint32_t i = 0; i < ADimRow; i++){
        for(uint32_t j = 0; j < BDimCol; j++){

            Result[i][j] = 0;

            for(uint32_t k = 0; k < ADimCol; k++){

                Result[i][j] += A[i][k]*B[k][j];

            }
        }
    }

}

//Matrix differencing
void MatrixSubtracting(float A[][], float B[][]


*/