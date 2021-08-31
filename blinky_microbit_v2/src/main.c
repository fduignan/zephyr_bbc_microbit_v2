/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/gpio.h>
// Blinks an LED attached to pin 2 of the edge connector/breakout board
void main(void)
{
	int ret;
	struct device *gpio;
	printk("Hello World! %s\n", CONFIG_BOARD);
	gpio=device_get_binding("GPIO_0");
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	while(1)
	{
		printf("res=%d\n",gpio_pin_set(gpio,2,1));

		k_msleep(500);
		gpio_pin_set(gpio,2,0);
		printk("off\n");
		k_msleep(500);
	}
}
