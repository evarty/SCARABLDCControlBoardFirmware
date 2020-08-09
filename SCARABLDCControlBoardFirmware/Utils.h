#ifndef UTILS
#define UTILS

void SineArray(float SinArray[], uint32_t EncoderResolution);

void CosineArray(float CosArray[], uint32_t EncoderResolution);

void DisablePIOWriteProtection(uint32_t port);

void SetPinIODirection(uint32_t port, uint32_t pin, uint32_t Direction);

void SetPinIOOutputValue(uint32_t port, uint32_t pin, uint32_t value);

uint32_t ReadIOPinValue(uint32_t port, uint32_t pin);

void ClockSetup();

void PIOSetup();

void EnablePIOControl(uint32_t port, uint32_t pin);

void DisablePIOControl(uint32_t port, uint32_t pin);

//Multiplies matrices together
//This assumes that proper sized matrices are passed to it. Checks must be done outside the function.
//void MatrixMultiply(float A[][], float B[][], uint32_t ADimRow, uint32_t ADimCol, uint32_t BDimRow, uint32_t BDimCol, float Result[][]);

#endif
