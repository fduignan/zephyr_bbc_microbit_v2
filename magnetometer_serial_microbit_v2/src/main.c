/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <drivers/i2c.h>
//#define I2C_DEV DT_LABEL(I2C_0)

void main(void)
{
	int ret;
	const struct device *i2c_dev;

	i2c_dev = device_get_binding("I2C_0");
        if (!i2c_dev) {
                printk("I2C: Device driver not found.\n");
        }

	const struct device *gpio;
	printk("Hello World! %s\n", CONFIG_BOARD);
	const struct device *lsm303agr_m = DEVICE_DT_GET_ANY(st_lsm303agr_magn);
	if (lsm303agr_m == NULL)
	{
		printk("Couldn't get a handle for the magnetometer");
	}
        if (!device_is_ready(lsm303agr_m))
	{
                printk("Device is not ready\n");
        }

	gpio=device_get_binding("GPIO_0");
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	uint8_t buf[]={0x60,0x80};
//	ret = i2c_write(i2c_dev, buf, 2, 0x1e);  // power up the magnetometer (driver is broken)
	while(1)
	{
		gpio_pin_set(gpio,2,1);
		k_msleep(500);
		gpio_pin_set(gpio,2,0);
		k_msleep(500);
		struct sensor_value mag[3];
		ret =  sensor_sample_fetch(lsm303agr_m);
		if (ret < 0)
		{
			printf("error reading magnetometer\n");
		}
		else
		{
			ret = sensor_channel_get(lsm303agr_m, SENSOR_CHAN_MAGN_XYZ, mag);
			if (ret < 0)
			{
				printf("error reading magetometer\n");
			}
			else
			{
 				printf("X: %f, Y: %f, Z: %f\n",sensor_value_to_double(&mag[0]),
							sensor_value_to_double(&mag[1]),
							sensor_value_to_double(&mag[2]));
			}
		}

	}
}
