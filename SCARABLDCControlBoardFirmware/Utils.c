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