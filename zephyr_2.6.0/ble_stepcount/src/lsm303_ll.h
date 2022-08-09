#ifndef __bmp280_h
#define __bmp280_h
#include <stdint.h>
#define LSM303_ACCEL_ADDRESS (0x19)
int lsm303_ll_begin();
int lsm303_ll_readAccelY();
int lsm303_countSteps(volatile uint32_t * pCount);

#endif
