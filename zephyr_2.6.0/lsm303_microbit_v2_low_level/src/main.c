/*
 * SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>	
#include <stdio.h>
#include <string.h>
#include "lsm303_ll.h"

void main(void)
{
	int ret;
		
	ret = lsm303_ll_begin();
	if (ret < 0)
	{
		printf("\nError initializing lsm303.  Error code = %d\n",ret);	
		while(1);
	}
	while(1)
	{    
        printf("Accel Y (x100) = %d\n",lsm303_ll_readAccelY());
         k_msleep(100);

	}
}
