#ifndef OBSERVER
#define OBSERVER

//The following must be passed to this function: the observer matrices, the prior step input, the prior step output, the prior step state estimation, and the sizes of the various matrices involved. It is assumed that the input, output, and state are column vectors.
//The output is xNew, which is the observer estimation of the state variables for the step after the values given. All values must be given from the same time step or else the results are gibberish.
void LuenbergerObserverUpdate(float A[][], float B[][], float C[][], float L[][], float xPrior[][], float yPrior[][], float xNew[][], uint32_t StateLength, uint32_t ADimRow, uint32_t ADimCol, uint32_t BDimRow, uint32_t BDimCol, uint32_t CDimRow, uint32_t CDimCol, uint32_t LDimRow, uint32_t LDimCol, float Input[][], uint32_t InputLength, uint32_t OutputLength);


void ReducedOrderObserverSetup(float Abb[][], float Aab[][], float Ba[][], float Bb[][], float L[][], uint32_t AbbDimRow, uint32_t AbbDimCol, uint32_t AabDimRow, uint32_t AabDimCol, uint32_t BaDimRow, uint32_t BaDimCol, uint32_t BbDimRow, uint32_t BbDimCol, uint32_t LDimRow, uint32_t LDimCol, uint32_t OutputDimRow, float Aba[][], uint32_t AbaDimRow, uint32_t AbaDimCol, float Aaa[][], uint32_t AaaDimRow, uint32_t AaaDimCol, float Hy[][], float Hu[][], float Gr[][]);

void ReducedOrderObserverUpdate(float Gr[][], float EtaPrior[][], float Hu[][], vPrior[][], float Hy[][], yPrior[][], uint32_t GrDimRow, uint32_t GrDimCol, uint32_t EtaDimRow, uint32_t EtaDimCol, uint32_t HuDimRow, uint32_t HuDimCol, uint32_t vPriorDimRow, uint32_t vPriorDimCol, uint32_t HyDimRow, uint32_t HyDimCol, uint32_t yPriorDimRow, uint32_t yPriorDimCol, float Eta[][], float xhatb[][], float L[][], float yCurrent[][], uint32_t xhatbDimRow, uint32_t xhatbDimCol, uint32_t LDimRow, uint32_t LDimCol);

#endif
