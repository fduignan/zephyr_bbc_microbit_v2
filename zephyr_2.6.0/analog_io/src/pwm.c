#include <stdint.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/pwm.h>
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
int pwm_write(uint16_t value)
{
	
	return pwm_pin_set_usec(pwm,3,PWM_PERIOD_US,value,0);
}
