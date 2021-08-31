/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <stdio.h>
void main(void)
{
	int ret;
	const struct device *gpio;
	printk("Hello World! %s\n", CONFIG_BOARD);
	const struct device *lsm303agr_a = DEVICE_DT_GET_ANY(st_lsm303agr_accel);
	if (lsm303agr_a == NULL)
	{
		printk("Couldn't get a handle for the accelerometer");
	}
        if (!device_is_ready(lsm303agr_a))
	{
                printk("Device is not ready\n");
        }

	gpio=device_get_binding("GPIO_0");
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	while(1)
	{
		gpio_pin_set(gpio,2,1);
		k_msleep(500);
		gpio_pin_set(gpio,2,0);
		k_msleep(500);
		struct sensor_value accel[3];
		ret =  sensor_sample_fetch(lsm303agr_a);
		if (ret < 0)
		{
			printf("error reading accelerometer\n");
		}
		else
		{
			ret = sensor_channel_get(lsm303agr_a, SENSOR_CHAN_ACCEL_XYZ, accel);
			if (ret < 0)
			{
				printf("error reading accelerometer\n");
			}
			else
			{
				printf("X: %f, Y: %f, Z: %f\n",sensor_value_to_double(&accel[0]),
							sensor_value_to_double(&accel[1]),
							sensor_value_to_double(&accel[2]));

			}
		}

	}
}
