#include <stdint.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/gpio.h>
#include <stdio.h>
#include "buttons.h"
// Both buttons are on GPIO0
#define BUTTON_A_PORT_BIT 14
#define BUTTON_B_PORT_BIT 23
static const struct device *gpio0;
int get_buttonA()
{
	return gpio_pin_get(gpio0,BUTTON_A_PORT_BIT);
}
int get_buttonB()
{
	return gpio_pin_get(gpio0,BUTTON_B_PORT_BIT);
}
int buttons_begin()
{
	int ret;
	// Configure the GPIO's 	
	gpio0=device_get_binding("GPIO_0");
	if (gpio0 == NULL)
	{
		printf("Error acquiring GPIO 0 interface\n");
		return -1;
	}
	ret = gpio_pin_configure(gpio0,BUTTON_A_PORT_BIT,GPIO_INPUT);
	ret = gpio_pin_configure(gpio0,BUTTON_B_PORT_BIT,GPIO_INPUT);
	return 0;
}

