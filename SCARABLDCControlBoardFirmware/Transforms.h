#ifndef TRANSFORMS
#define TRANSFORMS

//header file for the current and voltage transforms

void ClarkTransform(float ClarkCurrent[], uint32_t CurrentA, uint32_t CurrentB);

void ParkTransform(float ParkCurrent[], float ClarkCurrent[], uint32_t RotorPosition, float SinArray[], float CosArray[]);

void InversePark(float ClarkVoltageRef[], float StationaryVoltageRef[], uint32_t RotorPosition, float SinArray[], float CosArray[]);

void InverseClark(float OutputVoltageRef[], float ClarkVoltageRef[]);

#endif