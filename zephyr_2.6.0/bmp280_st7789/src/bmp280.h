#ifndef __bmp280_h
#define __bmp280_h
#include <stdint.h>
#define BMP280_R_ADDRESS (0x76)
int bmp280_begin();
int32_t bmp280_readPressure(); // returns Pressure * 100
int32_t bmp280_readTemperature(); // returns Temperature * 100
int bmp280_readRegister(uint8_t RegNum, uint8_t *Value);
int bmp280_writeRegister(uint8_t RegNum, uint8_t Value);
void bmp280_readCalibrationData(void);

#endif
