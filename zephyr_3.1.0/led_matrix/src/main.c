/*
 * SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>	
#include <stdio.h>
#include "matrix.h"

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
	while(1)
	{       
		matrix_put_pattern(rows, ~cols);
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
		k_msleep(100);
	}
}
