
// Blink the top left LED in the LED matrix using zephyrs IO library
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
static const struct device *gpio_0;
int main(void)
{
	int ret;
	gpio_0=DEVICE_DT_GET(DT_NODELABEL(gpio0));  // Get a device structure for GPIO 0
	if (gpio_0 == NULL)
	{
		printk("Error acquiring GPIO 0 interface\n");
		return -1;
	}
	ret = gpio_pin_configure(gpio_0,21,GPIO_OUTPUT); // Row 1 Anode is connected to GPIO0.21
	ret = gpio_pin_configure(gpio_0,28,GPIO_OUTPUT); // Col 1 Cathode is connected to GPIO0.28
    gpio_pin_set(gpio_0,28,0); // Drive Cathode low

	while (1) {
		gpio_pin_set(gpio_0,21,1);		// drive anode high
		k_msleep(100);					// wait 100ms
		gpio_pin_set(gpio_0,21,0);		// drive anode low
		k_msleep(100);					// wait 100ms
	}
	return 0;
}
