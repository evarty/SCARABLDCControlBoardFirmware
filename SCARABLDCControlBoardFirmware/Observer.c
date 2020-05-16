#include "sam.h"
#include "Observer.h"
#include "Utils.h"

//The following must be passed to this function: the observer matrices, the prior step input, the prior step output, the prior step state estimation, and the sizes of the various matrices involved. It is assumed that the input, output, and state are column vectors.
//The output is xNew, which is the observer estimation of the state variables for the step after the values given. All values must be given from the same time step or else the results are gibberish.
void LuenbergerObserverUpdate(float A[][], float B[][], float C[][], float L[][], float xPrior[][], float yPrior[][], float xNew[][], uint32_t StateLength, uint32_t ADimRow, uint32_t ADimCol, uint32_t BDimRow, uint32_t BDimCol, uint32_t CDimRow, uint32_t CDimCol, uint32_t LDimRow, uint32_t LDimCol, float Input[][], uint32_t InputLength, uint32_t OutputLength){

    //Define the various terms in the observer
    float AtimesXHat[StateLength][1];
    float BtimesInput[InputLength][1];
    float CtimesXHat[OutputLength][1];
    float OutputDifference[OutputLength][1];
    float LtimesOutputDifference[StateLength][1];

    //Compute the various terms in the observer
    MatrixMultiply(A, xPrior, ADimRow, ADimCol, StateLength, 1, AtimesXHat);
    MatrixMultiply(B, Input, BDimRow, BDimCol, InputLength, 1, BtimesInput);
    MatrixMultiply(C, xPrior, CDimRow, CDimCol, StateLength, 1, CtimesXHat);
    for(uint32_t f = 0; f < OutputLength; f++){
        OutputDifference[f][1] = yPrior[f][1] - CtimesXHat[f][1];
    }
    MatrixMultiply(L, OutputDifference, LDimRow, LDimCol, OutputLength, 1, LtimesOutputDifference);

    
    //Assemble the observer terms into the output.
    for(uint32_t i = 0; i < StateLength; i++){
        xNew[i][1] = AtimesXHat[i][1] + BtimesInput[i][1] + LtimesOutputDifference[i][1];

    }

}

void ReducedOrderObserverSetup(float Abb[][], float Aab[][], float Ba[][], float Bb[][], float L[][], uint32_t AbbDimRow, uint32_t AbbDimCol, uint32_t AabDimRow, uint32_t AabDimCol, uint32_t BaDimRow, uint32_t BaDimCol, uint32_t BbDimRow, uint32_t BbDimCol, uint32_t LDimRow, uint32_t LDimCol, uint32_t OutputDimRow, float Aba[][], uint32_t AbaDimRow, uint32_t AbaDimCol, float Aaa[][], uint32_t AaaDimRow, uint32_t AaaDimCol, float Hy[][], float Hu[][], float Gr[][]){

    float LtimesAab[AbbDimRow][AbbDimCol]; 
    MatrixMultiply(L, Aab, LDimRow, LDimCol, AabDimRow, AabDimCol, LtimesAab);

    //float Gr[AaDimRow][AaDimCol];
    for(uint32_t i = 0; i < AbbDimRow; i++){
        for(uint32_t j = 0; j< AbbDimCol; j++){
            Gr[i][j] = Abb[i][j] - LtimesAab[i][j];
        }
    }

    float LtimesBa[BbDimRow][BbDimCol];
    MatrixMultiply(L, Ba, LDimRow, LDimCol, BaDimRow, BaDimCol, LtimesBa);
    //float Hu[BbDimRow][BbDimCol];
    for(uint32_t i = 0; i < BbDimRow; i++){
        for(uint32_t j = 0; j< BbDimCol; j++){
            Hu[i][j] = Bb[i][j] - LtimesBa[i][j];
        }
    }

    float GrtimesL[AbaDimRow][AbaDimCol];
    MatrixMultiply(Gr, L, AbbDimRow, AbbDimCol, LDimRow, LDimCol, GrtimesL);
    float LtimesAaa[AbaDimRow][AbaDimCol];
    MatrixMultiply(L, Aaa, LDimRow, LDimCol, AaaDimRow, AaaDimCol, LtimesAaa);
    //float Hy[AbaDimRow][AbaDimCol];
    for(uint32_t i = 0; i < AbaDimRow; i++){
        for(uint32_t j = 0; j< AbaDimCol; j++){
            Hy[i][j] = GrtimesL[i][j] + Aba[i][j] - LtimesAaa[i][j];
        }
    }

}

void ReducedOrderObserverUpdate(float Gr[][], float EtaPrior[][], float Hu[][], vPrior[][], float Hy[][], yPrior[][], uint32_t GrDimRow, uint32_t GrDimCol, uint32_t EtaDimRow, uint32_t EtaDimCol, uint32_t HuDimRow, uint32_t HuDimCol, uint32_t vPriorDimRow, uint32_t vPriorDimCol, uint32_t HyDimRow, uint32_t HyDimCol, uint32_t yPriorDimRow, uint32_t yPriorDimCol, float Eta[][], float xhatb[][], float L[][], float yCurrent[][], uint32_t xhatbDimRow, uint32_t xhatbDimCol, uint32_t LDimRow, uint32_t LDimCol){

    float GrTimesEtaPrior[EtaDimRow][1];
    MatrixMultiply(Gr, EtaPrior, GrDimRow, GrDimCol, EtaDimRow, 1, GrTimesEtaPrior);
    float HuTimesvPrior[EtaDimRow][1];
    MatrixMultiply(Hu, vPrior, HuDimRow, HuDimCol, vPriorDimRow, vPriorDimCol, HuTimesvPrior);
    float HyTimesyPrior[EtaDimRow][1];
    MatrixMultiply(Hy, yPrior, HyDimRow, HyDimCol, yPriorDimRow, yPriorDimCol, HyTimesyPrior);
    float Eta[EtaDimRow][1];
    for(int i = 0; i < EtaDimRow; i++){
        Eta[i][1] = GrTimesEtaPrior[i][1] + HuTimesvPrior[i][1] + HyTimesyPrior[i][1];
    }

    float LTimesyCurrent[EtaDimRow][1];
    MatrixMultiply(L, yCurrent, LDimRow, LDimCol, yPriorDimRow, yPriorDimCol, LTimesyCurrent);
    for(int i = 0; i < xhatbDimCol; i++){
        xhatb[i][1] = Eta[i][1] + LTimesyCurrent[i][1];
    }

}











