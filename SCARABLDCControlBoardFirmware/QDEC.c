#include "sam.h"

uint32_t QDECGetDirection(void)
{
	return REG_TC0_QISR & TC_QISR_DIR;
}

uint32_t QDECGetPositionTotal(void)
{
	uint32_t Position = REG_TC0_CV0;
	uint32_t Revolution = REG_TC0_CV1;
	
	return Position + 2000*Revolution;
}

uint32_t QDECGetPostionSingle(void)
{
	return REG_TC0_CV0;
}