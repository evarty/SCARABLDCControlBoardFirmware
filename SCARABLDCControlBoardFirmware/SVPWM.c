#include "sam.h"
#include <math.h>

#define ONEOVERROOTTHREE 0.577350
#define TWOOVERROOTTHREE 1.154700

//Takes reference Clark voltages and the Max PWM count value as input.
//Returns PWM compare values for each mosfet and whether they should be inverted

void SVPWMBase(uint32_t SVPWMOuput[] ,float ClarkVoltageA, float ClarkVoltageB, uint32_t PWMMax)
{
	
	uint32_t Sector = 0;
	uint32_t Q = 0;
	float SVPWMVdc = 16;
	float ClarkVoltageAAbs = fabs(ClarkVoltageA);
	float ClarkVoltageBAbs = fabs(ClarkVoltageB);
	float Ur = 0;
	float Ul = 0;
	float T0 = 0;
	float T1 = 0;
	float T2 = 0;
	float USwitch;
	float VSwitch;
	float WSwitch;
	uint32_t Invert;
	//The first 6 values are the PWM compare values. The second 6 are whether they should be inverted. 1 is inverted. 0 is non-inverted.
	//uint32_t SVPWMOuput[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	
	
	//We will use b to determine the sector. a and c will be used later
	float a = ClarkVoltageAAbs + ONEOVERROOTTHREE*ClarkVoltageBAbs;
	float b = ClarkVoltageAAbs - ONEOVERROOTTHREE*ClarkVoltageBAbs;
	float c = TWOOVERROOTTHREE*ClarkVoltageBAbs;
	
	
	//Determine what sector we are in.
	//This used the flowchart from chapter 2 of "Vector Control of Three-Phase AC Machines" by Quang and Dittrich.
	if (ClarkVoltageB < 0)
	{
		if (ClarkVoltageA < 0)
		{
			Q = 3;
			if (b < 0)
			{
				Sector = 5;
			} 
			else
			{
				Q = 4;
				Sector = 4;
			}
		} 
		else
		{
			if (b < 0)
			{
				Sector = 5;
			} 
			else
			{
				Sector = 6;
			}
		}
	} 
	else
	{
		if (ClarkVoltageA < 0)
		{
			Q = 2;
			if (b < 0)
			{
				Sector = 2;
			} 
			else
			{
				Sector = 3;
			}
		} 
		else
		{
			Q = 1;
			if (b < 0)
			{
				Sector = 2;
			} 
			else
			{
				Sector = 1;
			}
		}
	}
	//Now we know what sector and quadrant we are in.
	//Switching times can now be calculated. Same source on equations.
	//Need to calculate Ur and Ul, the voltages on the right and left side of the desired voltage vector
	
	if ((Sector == 1) && (Q == 1))
	{
		Ur = b;
		Ul = c;
	}else if ((Sector == 2) && (Q == 1))
	{
		Ur = a;
		Ul = -b;
	}else if ((Sector == 2) && (Q == 2))
	{
		Ur = -b;
		Ul = a;
	}else if ((Sector == 3) && (Q == 2))
	{
		Ur = c;
		Ul = b;
	}else if ((Sector == 4) && (Q == 3))
	{
		Ur = b;
		Ul = c;
	}else if ((Sector == 5) && (Q == 3))
	{
		Ur = a;
		Ul = -b;
	}else if ((Sector == 5) && (Q == 4))
	{
		Ur = -b;
		Ul = a;
	}else if ((Sector == 6) && (Q == 4))
	{
		Ur = c;
		Ul = b;
	}
	//Now we have Ur and Ul. We can get the relative times by dividing these by the voltage of the full vectors Vdc
	T1 = Ur/SVPWMVdc;
	T2 = Ul/SVPWMVdc;
	T0 = 1 - T1 - T2;
	
	//So, we have the times for the three vectors, though the T0 will be split up.
	//Now, we turn to "Space Vector Pulse Width Modulation For Two Level Inverter" by Saritha, Abhiram, and Sumanth.
	//I could have used Table 2 for determining the switching times, but it needs sine evaluations
	//I decided the method used above is less computationally intensive to get the times.
	//But, I am going to use this new paper for the switching levels because they provide a nice chart with the values
	//laid out for all the switches. This is then easy to convert to PWM compare values.
	//I assume that the full phase correct PWM generation where the counter counts both up and down is used.
	//This generates center-aligned PWM.
	float x = (T0/2);
	float y = ((T0/2) + T1);
	float z = ((T0/2) + T1 + T2);
	if (Sector == 1)
	{
		USwitch = x;
		Invert = 0;
		VSwitch = y;
		WSwitch = z;
		
	}else if (Sector == 2)
	{
		USwitch = y;
		Invert = 1;
		VSwitch = z;
		WSwitch = x;
		
	}else if (Sector == 3)
	{
		USwitch = z;
		Invert = 0;
		VSwitch = x;
		WSwitch = y;
		
	}else if (Sector == 4)
	{
		USwitch = x;
		Invert = 1;
		VSwitch = y;
		WSwitch = z;
		
	}else if (Sector == 5)
	{
		USwitch = y;
		Invert = 0;
		VSwitch = z;
		WSwitch = x;
		
	}else if (Sector == 6)
	{
		USwitch = z;
		Invert = 1;
		VSwitch = x;
		WSwitch = y;
	
	}
	USwitch = USwitch*PWMMax;
	VSwitch = VSwitch*PWMMax;
	WSwitch = WSwitch*PWMMax;
	
	SVPWMOuput[0] = (uint32_t)USwitch;
	SVPWMOuput[1] = (uint32_t)VSwitch;
	SVPWMOuput[2] = (uint32_t)WSwitch;
	SVPWMOuput[3] = Invert;
	
	
}