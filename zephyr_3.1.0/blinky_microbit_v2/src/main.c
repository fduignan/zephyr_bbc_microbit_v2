/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
// Blinks an LED attached to RING 0 of the edge connector/breakout board
void main(void)
{
	int ret;
	struct device *gpio;
	printk("Hello World! %s\n", CONFIG_BOARD);
	//gpio=device_get_binding("GPIO_0");
    gpio=DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (gpio==0)
	{
		printk("Error acquiring GPIO 0 interface\n");
	}
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	while(1)
	{
		printk("res=%d\n",gpio_pin_set(gpio,2,1));

		k_msleep(500);
		gpio_pin_set(gpio,2,0);
		printk("off\n");
		k_msleep(500);
	}
}
