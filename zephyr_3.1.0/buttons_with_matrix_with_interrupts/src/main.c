/*
 * SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "matrix.h"
#include "buttons.h"
volatile uint8_t rows = 1;
volatile uint8_t cols = 1;
void button_a_pressed(void)
{
	cols = cols << 1;
	if (cols > 16)
	{
		cols = 1;
		rows = rows << 1;
		if (rows > 16)
		{
			rows = 1;
		}
	}
}
void main(void)
{
	int ret;
	ret = matrix_begin();
	if (ret < 0)
	{
		printf("\nError initializing LED matrix.  Error code = %d\n",ret);	
		while(1);
	}
	ret = buttons_begin();	
	if (ret < 0)
	{
		printf("\nError initializing buttons.  Error code = %d\n",ret);	
		while(1);
	}	
	attach_callback_to_button_a(button_a_pressed);
	while(1)
	{       
		matrix_put_pattern(rows, ~cols);
		k_msleep(100);
		
	}
}
