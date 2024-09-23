#ifndef __bmp280_h
#define __bmp280_h
#include <stdint.h>
#define LSM303_ACCEL_ADDRESS (0x19)
#define LSM303_MAG_ADDRESS (0x1e  )

int lsm303_ll_begin();
int lsm303_ll_readAccelX();
int lsm303_ll_readAccelY();
int lsm303_ll_readAccelZ();
int lsm303_ll_readMagX();
int lsm303_ll_readMagY();
int lsm303_ll_readMagZ();
int lsm303_countSteps(volatile uint32_t * pCount);
#endif
