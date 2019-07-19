#include "sam.h"
#include "Transforms.h"


//Clark Transform. Input is measured winding currents. Output is Clark currents, which are rotating.
//element[0] is i_alpha. element[1] is i_beta.
void ClarkTransform(float ClarkCurrent[], uint32_t CurrentA, uint32_t CurrentB){
		
	//ClarkCurrent[0] is i_alpha. ClarkCurrent[1] is i_beta.
	ClarkCurrent[0] = (float) CurrentA;
	ClarkCurrent[1] = (float) CurrentA/1.73205 + (float) CurrentB*1.15470;
	
}

//Park Transform. Input is Clark currents. Output is Park currents, which are stationary.
//element[0] is i_d (flux). element[1] is i_q (torque).
void ParkTransform(float ParkCurrent[], float ClarkCurrent[], uint32_t RotorPosition, float SinArray[], float CosArray[]){
	
	//ParkCurrent[0] is i_d. ParkCurrent[1] is i_q.
	ParkCurrent[0] = ClarkCurrent[0]*CosArray[RotorPosition] + ClarkCurrent[1]*SinArray[RotorPosition];
	ParkCurrent[1] = -ClarkCurrent[0]*SinArray[RotorPosition] + ClarkCurrent[1]*CosArray[RotorPosition];
	
}

//Inverse Park Transform. Input is stationary reference voltages, which are the Park reference voltages. Outputs the Clark reference voltages.
//element[0] is V_alpha_ref. element[1] is V_beta_ref.
//The stationary reference voltages come from the control loop.
void InversePark(float ClarkVoltageRef[], float StationaryVoltageRef[], uint32_t RotorPosition, float SinArray[], float CosArray[]){
	
	ClarkVoltageRef[0] = StationaryVoltageRef[0]*CosArray[RotorPosition] - StationaryVoltageRef[1]*SinArray[RotorPosition];
	ClarkVoltageRef[1] = StationaryVoltageRef[0]*SinArray[RotorPosition] + StationaryVoltageRef[1]*CosArray[RotorPosition];
	
}

//Inverse Clark Transform. Input is the Clark reference voltages, which are rotating. Outputs control voltages that need to go to motor windings.
//element[0] is V_a_ref. element[1] is V_b_ref. element[2] is V_c_ref.
void InverseClark(float OutputVoltageRef[], float ClarkVoltageRef[]){
	
	OutputVoltageRef[0] = ClarkVoltageRef[0];
	OutputVoltageRef[1] = -0.5*ClarkVoltageRef[0] + 0.866025*ClarkVoltageRef[1];
	OutputVoltageRef[2] = -0.5*ClarkVoltageRef[0] - 0.866025*ClarkVoltageRef[1];
	
}