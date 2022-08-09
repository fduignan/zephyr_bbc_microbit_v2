/*
 * SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>	
#include <stdio.h>
#include "matrix.h"
#include "buttons.h"
void main(void)
{
	int ret;
	uint8_t rows = 1;
	uint8_t cols = 1;
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
	while(1)
	{       
		matrix_put_pattern(rows, ~cols);
		if (get_buttonA()==0)
		{
			cols = cols << 1;
		}
		if (get_buttonB()==0)
		{
			rows = cols = 1;
		}
		if (cols > 16)
		{
			cols = 1;
			rows = rows << 1;
			if (rows > 16)
			{
				rows = 1;
			}
		}
		k_msleep(100);
	}
}
