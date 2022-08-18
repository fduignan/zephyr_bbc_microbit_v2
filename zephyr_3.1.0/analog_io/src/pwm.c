#include <stdint.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <stdio.h>
#include "pwm.h"
static const struct device *pwm;
int pwm_begin()
{
	// Configure the GPIO's 	
	pwm=device_get_binding("PWM_0");
	if (pwm == NULL)
	{
		printf("Error acquiring PWM interface \n");
		return -1;
	}
	return 0;
}
int pwm_write(uint32_t value)
{
	
    return pwm_set(pwm,0,PWM_PERIOD_NS,value,0);

}
