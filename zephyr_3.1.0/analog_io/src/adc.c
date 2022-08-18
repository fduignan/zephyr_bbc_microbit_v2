#include <stdint.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <stdio.h>
#include "adc.h"
/*
 * The internal voltage reference is 0.6V
 * The permissable gains are 1/6, 1/5, 1/4, 1/3, 1/2, 1, 2 and 4
 * If a gain of 1/5 is selected then the ADC range becomes 0 to 3V
 */
static const struct device *adc;
// Will read from analog input on P0.2 which is RING 0 on the microbit v2
#define ADC_PORT_BIT 2
#define ADC_RANGE 3.0
struct adc_channel_cfg channel_cfg = {
		/* gain of 1/5 */
		.gain = ADC_GAIN_1_5,
		/* 0.6V reference */
		.reference = ADC_REF_INTERNAL,
		/* sample and hold time of 3us is the default (0 setting) on the NRF52833 */
		.acquisition_time = ADC_ACQ_TIME_DEFAULT,
		/* Channel 0 */
		.channel_id = 1,
		/* AIN0 is specified by setting input_positive to 0+1 i.e. an offset of 1  */
		/* This is as a result of the way the PSELP and PSELN registers work in the NRF52 series of devices */
		/* see page 375 of the NRF52 product specificatoin version : 4452_021 v1.3 */
		.input_positive = 2,
		/* Using single ended conversions */
        .differential = 0
};
static int16_t channel_0_data;  // This will hold the adc result

struct adc_sequence sequence = {        
		/* This is a bitmask that tells the driver which channels to convert : bit n = 1 for channel n */		
		.channels    = (1 << 1),
		/* Where will the data be stored (could be an array if there are multiple channels to convert */
		.buffer      = &channel_0_data,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(channel_0_data),
		/* 12 bit resolution */
		.resolution  = 12,
		/* nulls for the rest of the fields */
		.options = NULL,
		.calibrate = 0,
		.oversampling = 0,        
};
int adc_readDigital()
{
	int ret;
	ret = adc_read(adc, &sequence);	
	return channel_0_data;
}
int adc_begin()
{
	int ret;
	// Configure the GPIO's 	
	adc=device_get_binding("ADC_0");
	if (adc == NULL)
	{
		printf("Error acquiring ADC \n");
		return -1;
	}
	ret = adc_channel_setup(adc, &channel_cfg);
	if (ret < 0)
	{
		printf("Error configuring ADC channel 0\n");
		return -2;
	}		
	return 0;
}
double adc_readVoltage()
{
	double returnValue = adc_readDigital();
	returnValue = (returnValue * ADC_RANGE) / 4095.0;
	return returnValue;
}
