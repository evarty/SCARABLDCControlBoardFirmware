#ifndef UTILS
#define UTILS

void SineArray(float SinArray[], uint32_t EncoderResolution);

void CosineArray(float CosArray[], uint32_t EncoderResolution);

//Multiplies matrices together
//This assumes that proper sized matrices are passed to it. Checks must be done outside the function.
void MatrixMultiply(float A[][], float B[][], uint32_t ADimRow, uint32_t ADimCol, uint32_t BDimRow, uint32_t BDimCol, float Result[][]);

#endif
