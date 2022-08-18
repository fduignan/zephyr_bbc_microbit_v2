/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
// Lights an LED attached to RING 0 of the connector/breakout when button A is pressed
static const struct gpio_dt_spec button_a = GPIO_DT_SPEC_GET(DT_NODELABEL(buttona), gpios);

void main(void)
{
	int ret;
	struct device *gpio;
	printk("Hello World! %s\n", CONFIG_BOARD);
	gpio=device_get_binding("GPIO_0");
	if (gpio==0)
	{
		printk("Error acquiring GPIO 0\n");
	}
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	ret = gpio_pin_configure_dt(&button_a,GPIO_INPUT | GPIO_PULL_UP);
	while(1)
	{
		//printf("res=%d\n",gpio_pin_set(gpio,2,1));
		if (gpio_pin_get_dt(&button_a))
		{
			printk("Set\n");
			gpio_pin_set(gpio,2,1);
		}
		else
		{
			printk("Clear\n");
			gpio_pin_set(gpio,2,0);
		}

	}
}
