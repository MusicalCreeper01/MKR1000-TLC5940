
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define   DIV1     0
#define   DIV2     1
#define   DIV4     2
#define   DIV8     3
#define   DIV16    4
#define   DIV64    5
#define   DIV256   6
#define   DIV1024  7


extern void analogWritePrescale( uint32_t ulPin, uint32_t ulValue, uint32_t prescale);

#ifdef __cplusplus
}
#endif
