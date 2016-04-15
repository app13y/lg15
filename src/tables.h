#ifndef GOSTHOPPER_DATA_HEADER_INCLUDED_
#define GOSTHOPPER_DATA_HEADER_INCLUDED_

#include <stdint.h>

extern const uint8_t roundConstants[512];
extern const uint8_t exponentialTable[256];
extern const uint8_t logarithmicTable[256];
extern const uint8_t Pi[256];
extern const uint8_t InversedPi[256];
extern const uint8_t LTransformationMatrix[16][16];
extern const uint8_t inversedLTransformationMatrix[16][16];

#endif
