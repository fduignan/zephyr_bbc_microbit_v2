/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Lights an LED attached to RING 0of the connector/breakout when button A is pressed
static const struct gpio_dt_spec button_a = GPIO_DT_SPEC_GET(DT_NODELABEL(buttona), gpios);
static struct gpio_callback button_a_cb;
struct device *gpio;
void button_a_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	gpio_pin_toggle(gpio,2);
}

void main(void)
{
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);
	gpio=device_get_binding("GPIO_0");
	ret = gpio_pin_configure(gpio,2,GPIO_OUTPUT);
	ret = gpio_pin_configure_dt(&button_a,GPIO_INPUT | GPIO_PULL_UP);
	ret = gpio_pin_interrupt_configure_dt(&button_a,GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_a_cb, button_a_pressed, BIT(button_a.pin) );
	gpio_add_callback(button_a.port, &button_a_cb);
	while(1)
	{

	}
}
