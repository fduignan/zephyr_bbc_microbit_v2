/*
 * SPDX-License-Identifier: Apache-2.0
*/
// Reads an analogue voltage on AIN1 and outputs a PWM signal in proportion to this on GPIO0.2
#include <zephyr/kernel.h>
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
		printk("\nError initializing adc.  Error code = %d\n",ret);	
		while(1);
	}
	ret = pwm_begin();	
	if (ret < 0)
	{
		printk("\nError initializing PWM.  Error code = %d\n",ret);	
		while(1);
	}
	while(1)
	{       
		uint32_t adcvalue = adc_readDigital();
		printk("ADC Digital = %u\n",adcvalue);
		/* The default version of printf does not support floating point numbers so scale up to an integer */
		printk("ADC Voltage (mV) = %d\n",(int)(1000*adc_readVoltage()));
		pwm_write((adcvalue * PWM_PERIOD_NS   )/4095);
		k_msleep(100);
	}
}
