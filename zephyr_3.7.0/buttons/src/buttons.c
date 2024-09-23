#include <stdint.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "buttons.h"
// Both buttons are on GPIO0
#define BUTTON_A_PORT_BIT 14
#define BUTTON_B_PORT_BIT 23
static const struct device *gpio_0;
int get_buttonA()
{
	return gpio_pin_get(gpio_0,BUTTON_A_PORT_BIT);
}
int get_buttonB()
{
	return gpio_pin_get(gpio_0,BUTTON_B_PORT_BIT);
}
int buttons_begin()
{
	int ret;
	// Configure the GPIO's 	
	gpio_0=DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (gpio_0 == NULL)
	{
		printf("Error acquiring GPIO 0 interface\n");
		return -1;
	}
	ret = gpio_pin_configure(gpio_0,BUTTON_A_PORT_BIT,GPIO_INPUT);
	ret = gpio_pin_configure(gpio_0,BUTTON_B_PORT_BIT,GPIO_INPUT);
	return 0;
}
