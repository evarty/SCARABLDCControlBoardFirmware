#ifndef QDEC
#define QDEC

//Returns the direction of the encoder.
//1 is anticlockwise. 0 is clockwise.
uint32_t QDECGetDirection(void);

//Returns encoder position as a total number of steps since it was zeroed.
uint32_t QDECGetPositionTotal(void);

//Returns encoder position within a single revoltion
uint32_t QDECGetPostionSingle(void);

#endif