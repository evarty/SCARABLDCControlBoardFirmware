#include "sam.h"
#include "LinearController.h"
#include "Utils.h"



void LinearControllerUpdate(float xhat[][], float K[][], float r[][], float v[][], uint32_t xhatDimRow, uint32_t KDimRow, uint32_t KDimCol, uint32_t RDimRow, uint32_t vDimRow){
    
    float KTimesxhat[vDimRow][1];
    MatrixMultiply(K, xhat, KDimRow, KDimCol, xhatDimRow, 1, KTimesxhat);

    for(int i = 0; i < vDimRow; i++){
        v[i][1] = r[i][1] - KTimesxhat[i][1];
    }

}

