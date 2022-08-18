/*
 * SPDX-License-Identifier: Apache-2.0
*/
// Reads an analogue voltage on AIN1 and outputs a PWM signal in proportion to this on GPIO0.2
#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>	
#include <stdio.h>
#include "adc.h"
#include "pwm.h"
void main(void)
{
	int ret;
    ret = adc_begin();	
	if (ret < 0)
	{
		printf("\nError initializing adc.  Error code = %d\n",ret);	
		while(1);
	}
	ret = pwm_begin();	
	if (ret < 0)
	{
		printf("\nError initializing PWM.  Error code = %d\n",ret);	
		while(1);
	}
	 pwm_write((2048* PWM_PERIOD_NS   )/4095);
		k_msleep(100);
	while(1)
	{       
		uint32_t adcvalue = adc_readDigital();
		printf("ADC Digital = %u\n",adcvalue);
		/* The default version of printf does not support floating point numbers so scale up to an integer */
		printf("ADC Voltage (mV) = %d\n",(int)(1000*adc_readVoltage()));
		pwm_write((adcvalue * PWM_PERIOD_NS   )/4095);
		k_msleep(100);
	}
}
