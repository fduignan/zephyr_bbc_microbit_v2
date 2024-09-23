/*
 * SPDX-License-Identifier: Apache-2.0
*/
// Reads an analogue voltage on AIN1 and outputs a PWM signal in proportion to this on GPIO0.2
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>	
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include "adc.h"
#include "pwm.h"
static const struct device *temp_dev;

void main(void)
{
	int ret;
	temp_dev = DEVICE_DT_GET(DT_NODELABEL(temp));
	if (temp_dev == NULL)
	{
		printk("Error acquiring temperature sensor\n");
	}
	else
	{
		printk("got temperature sensor");
	}
	while(1) {
		struct sensor_value temperature;
		ret= sensor_sample_fetch(temp_dev);
		if (ret != 0)
		{
			printk("Error fetching temperature value\n");
		}
		sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &temperature);
		printk("Chip temperature = %d\n",temperature.val1);
		k_msleep(100);
	}
}
