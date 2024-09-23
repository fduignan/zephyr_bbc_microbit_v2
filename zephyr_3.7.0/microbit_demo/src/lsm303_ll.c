#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>
#include <stdint.h>
#include "lsm303_ll.h"


static const struct device *gpio_0;
static volatile uint32_t *pStepCount=NULL;
// The LSM303's interrupt output pin is connected to P0.25
#define STEP_INTERRUPT_PORT_BIT 25 
typedef void (*fptr)(void);
int lsm303_ll_readRegister(uint8_t RegNum, uint8_t *Value);
int lsm303_ll_writeRegister(uint8_t RegNum, uint8_t Value);
int lsm303_ll_writeMagRegister(uint8_t RegNum, uint8_t Value);
void readCalibrationData();
//static const struct spi_config * cfg;
static const struct device *i2c;
int lsm303_ll_begin()
{
	int nack;
	uint8_t device_id;
	// Set up the I2C interface
	i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
	if (i2c==NULL)
	{
		printf("Error acquiring i2c1 interface\n");
		return -1;
	}
	// Fix for version 2.21 of the Microbit.  
    // This code resets the I2C_INT_INT signal coming out of the interface IC (DAPLink)
    // There is an acknowledged bug in the firmware for this IC which leave the interrupt
    // line asserted under certain conditions.  This prevents the LSM303 from raising interrupts
    // A dummy read of the interface IC (I2C address 0x70) deasserts this signal
    // Thanks to Carlos in microbit for this help.
    k_msleep(1000); // allow interface MCU complete booting before dummy read
    uint8_t dummy_value[5];
    nack=i2c_read(i2c,dummy_value,1,0x70);  
    printk("nack=%x\n",nack);

	// Check to make sure the device is present by reading the WHO_AM_I register
	nack = lsm303_ll_readRegister(0x0f,&device_id);
	if (nack != 0)
	{
		printf("Error finding LSM303 on the I2C bus\n");
		return -2;
	}
	else	
	{
		printf("Found LSM303.  WHO_AM_I = %d\n",device_id);
	}
	lsm303_ll_writeRegister(0x20,0x77); //wake up LSM303 (max speed, all accel channels)
	lsm303_ll_writeRegister(0x23,0x08); //enable  high resolution mode +/- 2g
	lsm303_ll_writeMagRegister(0x60,0x80); // enable Temperature compensation and continuous 10Hz conversions
	lsm303_ll_writeMagRegister(0x62,0x10); // ensure bytes from different readings don't get mixed up.
	return 0;
}
static struct gpio_callback stepcount_cb;
static void stepcount_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (pStepCount != NULL)
	{
		*pStepCount = *pStepCount + 1;
		printk("step %x\n",*pStepCount);
	}	
}
int lsm303_countSteps(volatile uint32_t * pCount)
{
	if (pStepCount != NULL)
	{
		return -1; // counting already enabled
	}
	pStepCount = pCount;
	int ret;
	// Configure the GPIO's 	
	gpio_0=DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (gpio_0 == NULL)
	{
		printf("Error acquiring GPIO 0 interface\n");
		return -2;
	}
	ret = gpio_pin_configure(gpio_0,STEP_INTERRUPT_PORT_BIT,GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0)
	{
		printf("Error configuring step interrupt pin\n");
		return -3;
	}
	if (gpio_pin_interrupt_configure(gpio_0,STEP_INTERRUPT_PORT_BIT,GPIO_INT_EDGE_FALLING) < 0)
	{
		printf("Error configuring interrupt for step count\n");
		return -4;
	}
	gpio_init_callback(&stepcount_cb, stepcount_handler, (1 << STEP_INTERRUPT_PORT_BIT) );	
    if (gpio_add_callback(gpio_0, &stepcount_cb) < 0)
	{
		printk("Error adding callback for stepcount interrupt \n");
		return -5;
	}
	// All of the callback plumbing is now done
	// Need to configure the LSM303 to make it generate interrupt signals.
	lsm303_ll_writeRegister(0x22,0x40); // Send AOI1 interrupts to INT1 output
	lsm303_ll_writeRegister(0x30,0x80+0x15); // Interrupt on low accel on all 3 axes
	lsm303_ll_writeRegister(0x32,0x7f); // set the low accel threshold
	return 0;
}
int lsm303_ll_readAccelX() // returns Temperature * 100
{
	int16_t accel;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_ACCEL_ADDRESS,0xa8, buf,2);
	accel = buf[1];
	accel = accel << 8;
	accel = accel + buf[0];
	accel = accel / 16; // must shift right 4 bits as this is a left justified 12 bit result
	// now scale to m^3/s * 100.
	// +2047 = +2g
	int accel_32bit = accel; // go to be wary of numeric overflow
	accel_32bit = accel_32bit * 2*981 / 2047;
    return accel_32bit;    
}
int lsm303_ll_readAccelY() // returns Temperature * 100
{
	int16_t accel;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_ACCEL_ADDRESS,0xaa, buf,2);
	accel = buf[1];
	accel = accel << 8;
	accel = accel + buf[0];
	accel = accel / 16; // must shift right 4 bits as this is a left justified 12 bit result
	// now scale to m^3/s * 100.
	// +2047 = +2g
	int accel_32bit = accel; // go to be wary of numeric overflow
	accel_32bit = accel_32bit * 2*981 / 2047;
    return accel_32bit;    
}
int lsm303_ll_readAccelZ() // returns Temperature * 100
{
	int16_t accel;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_ACCEL_ADDRESS,0xac, buf,2);
	accel = buf[1];
	accel = accel << 8;
	accel = accel + buf[0];
	accel = accel / 16; // must shift right 4 bits as this is a left justified 12 bit result
	// now scale to m^3/s * 100.
	// +2047 = +2g
	int accel_32bit = accel; // go to be wary of numeric overflow
	accel_32bit = accel_32bit * 2*981 / 2047;
    return accel_32bit;    
}


int lsm303_ll_readMagX() // returns Temperature * 100
{
	int16_t mag;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_MAG_ADDRESS,0xe8, buf,2);
	mag = buf[1];
	mag = mag << 8;
	mag = mag + buf[0];
	int mag_32 = mag; // type convert and preserve sign
    return mag_32;    
}

int lsm303_ll_readMagY() // returns Temperature * 100
{
	int16_t mag;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_MAG_ADDRESS,0xea, buf,2);
	mag = buf[1];
	mag = mag << 8;
	mag = mag + buf[0];
	int mag_32 = mag; // type convert and preserve sign
    return mag_32;    
}

int lsm303_ll_readMagZ() // returns Temperature * 100
{
	int16_t mag;
	uint8_t buf[2];
	i2c_burst_read(i2c,LSM303_MAG_ADDRESS,0xec, buf,2);
	mag = buf[1];
	mag = mag << 8;
	mag = mag + buf[0];
	int mag_32 = mag; // type convert and preserve sign
    return mag_32;    
}


int lsm303_ll_readRegister(uint8_t RegNum, uint8_t *Value)
{
	    //reads a byte from a specific register
    int nack;   
	nack=i2c_reg_read_byte(i2c,LSM303_ACCEL_ADDRESS,RegNum,Value);
	return nack;
}
int lsm303_ll_writeRegister(uint8_t RegNum, uint8_t Value)
{
	//writes a byte to a specific register
    uint8_t Buffer[2];    
    Buffer[0]= Value;    
    int nack;    
	nack=i2c_reg_write_byte(i2c,LSM303_ACCEL_ADDRESS,RegNum,Value);
    return nack;
}
int lsm303_ll_writeMagRegister(uint8_t RegNum, uint8_t Value)
{
	//writes a byte to a specific register
    uint8_t Buffer[2];    
    Buffer[0]= Value;    
    int nack;    
	nack=i2c_reg_write_byte(i2c,LSM303_MAG_ADDRESS,RegNum,Value);
    return nack;
}

