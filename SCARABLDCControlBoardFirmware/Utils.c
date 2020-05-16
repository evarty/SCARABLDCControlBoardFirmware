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
