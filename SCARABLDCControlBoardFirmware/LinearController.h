#ifndef LINEARCONTROLLER
#define LINEARCONTROLLER

void LinearControllerUpdate(float xhat[][], float K[][], float r[][], float v[][], uint32_t xhatDimRow, uint32_t KDimRow, uint32_t KDimCol, uint32_t RDimRow, uint32_t vDimRow);

#endif
