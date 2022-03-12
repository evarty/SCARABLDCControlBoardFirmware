#include "sam.h"
//The provided Atmel headers have a weird error/omission regarding the PIO_ABCDSR registers.
//They provide only a definition for PIO_ABCDSR1, but call it PIO_ABCDSR.
//This bit here simply defines PIO_ABCDSR1 and PIO_ABCDSR2
#define REG_PIOA_ABCDSR1 (*(__IO uint32_t*)0x400E0E70U)
#define REG_PIOA_ABCDSR2 (*(__IO uint32_t*)0x400E0E74U)
#define REG_PIOD_ABCDSR1 (*(__IO uint32_t*)0x400E1470U)

uint32_t QDECGetDirection(void)
{
	return REG_TC0_QISR & TC_QISR_DIR;
}

int32_t QDECGetPositionTotal(void)
{
	//int32_t Position = REG_TC0_CV0;
	int32_t Revolution = REG_TC0_CV1;
	
	return Revolution;//Position + 8000*Revolution;
}

int32_t QDECGetPostionSingle(void)
{
	int32_t hold = REG_TC0_CV0;
	//Want the position to be positive for SVPWM, which only cares about position within one rotation.
	if(hold >= 0)
		return hold;
	else
		return (8000 + hold);
}

void QDECSetup(void)
{
	REG_TC0_WPMR = TC_WPMR_WPKEY(0x54494D);
	//REG_TC1_WPMR = TC_WPMR_WPKEY(0x54494D);
	//Setup the quadrature decoder
	//TIOA0 and TIOB0 are the quadrature signals.
	//TIOB1 is the index signal.
	//This follows AT11483 process in section 5.2.4
	//Step 1
	//Configure IO pins to their peripheral functions
	REG_PIOA_WPMR = PIO_WPMR_WPKEY(0x50494F);//Disable write protection
	REG_PIOA_ABCDSR1 |=  PIO_ABCDSR_P0 | PIO_ABCDSR_P1 | PIO_ABCDSR_P16;//Setup for peripheral B
	REG_PIOA_PDR |= PIO_PDR_P0 | PIO_PDR_P1 | PIO_PDR_P16;//Disable PIO and enable peripheral. Don't think it matter since input, but oh well
	//Step 2
	//Enable channel 0 and 1 peripheral clocks
	//CURRENT CODE ENABLES MODULE 0
	REG_PMC_WPMR = PMC_WPMR_WPKEY(0x504D43);
	REG_PMC_PCER0 |= PMC_PCER0_PID21 | PMC_PCER0_PID22 | PMC_PCER0_PID23;
	//Step 3	
	//REG_TC0_CMR0 &= ~(TC_CMR_WAVE);
	//REG_TC0_CMR1 &= ~(TC_CMR_WAVE);
	
	REG_TC0_CMR0 |= TC_CMR_TCCLKS_XC0;
	//REG_TC0_CMR1 |= TC_CMR_TCCLKS_XC0;
	
	REG_TC0_CMR0 |= TC_CMR_ETRGEDG_RISING;
	REG_TC0_CMR1 |= TC_CMR_ETRGEDG_RISING;
	
	REG_TC0_CMR0 |= TC_CMR_ABETRG;
	REG_TC0_CMR1 |= TC_CMR_ABETRG;
	
	//Step 4
	REG_TC0_BMR |= TC_BMR_QDEN | TC_BMR_POSEN | TC_BMR_EDGPHA;// | TC_BMR_MAXFILT(0);// | TC_BMR_TC1XC1S_TIOA0;
	//REG_TC1_BMR |= TC_BMR_QDEN | TC_BMR_POSEN | TC_BMR_EDGPHA | TC_BMR_MAXFILT(0);
	//extra
	//REG_TC0_RC0 |= TC_RC_RC(8000);
	//REG_TC0_CMR0 |= TC_CMR_CPCTRG;
	//Step 5. Start TC0 and TC1
	//extra
	
	REG_TC0_CCR0 |= TC_CCR_CLKEN;
	REG_TC0_CCR1 |= TC_CCR_CLKEN;
	//The QDEC using channels 0 and 1 of TC0 is enabled and configured.
}