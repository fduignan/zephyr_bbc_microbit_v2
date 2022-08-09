#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>
#include <stdint.h>
#include "lsm303_ll.h"

static const struct device *gpio0;
static volatile uint32_t *pStepCount=NULL;
// The LSM303's interrupt output pin is connected to P0.25
#define STEP_INTERRUPT_PORT_BIT 25 
typedef void (*fptr)(void);



int lsm303_ll_readRegister(uint8_t RegNum, uint8_t *Value);
int lsm303_ll_writeRegister(uint8_t RegNum, uint8_t Value);
void readCalibrationData();
//static const struct spi_config * cfg;
static const struct device *i2c;
int lsm303_ll_begin()
{
	int nack;
	uint8_t device_id;
	// Set up the I2C interface
	i2c = device_get_binding("I2C_1");
	if (i2c==NULL)
	{
		printk("Error acquiring i2c1 interface\n");
		return -1;
	}	
	// Check to make sure the device is present by reading the WHO_AM_I register
	nack = lsm303_ll_readRegister(0x0f,&device_id);
	if (nack != 0)
	{
		printk("Error finding LSM303 on the I2C bus\n");
		return -2;
	}
	else	
	{
		printk("Found LSM303.  WHO_AM_I = %d\n",device_id);
	}
	lsm303_ll_writeRegister(0x20,0x77); //wake up LSM303 (max speed, all accel channels)
	lsm303_ll_writeRegister(0x23,0x08); //enable  high resolution mode +/- 2g
	
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
	gpio0=device_get_binding("GPIO_0");
	if (gpio0 == NULL)
	{
		printk("Error acquiring GPIO 0 interface\n");
		return -2;
	}
	ret = gpio_pin_configure(gpio0,STEP_INTERRUPT_PORT_BIT,GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0)
	{
		printk("Error configuring step interrupt pin\n");
		return -3;
	}
	if (gpio_pin_interrupt_configure(gpio0,STEP_INTERRUPT_PORT_BIT,GPIO_INT_EDGE_FALLING) < 0)
	{
		printk("Error configuring interrupt for step count\n");
		return -4;
	}
	gpio_init_callback(&stepcount_cb, stepcount_handler, (1 << STEP_INTERRUPT_PORT_BIT) );	
    if (gpio_add_callback(gpio0, &stepcount_cb) < 0)
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
int lsm303_ll_readAccelY()
{
	int16_t accel;
	uint8_t buf[2];
	buf[0] = 0x80+0x2a;	
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

