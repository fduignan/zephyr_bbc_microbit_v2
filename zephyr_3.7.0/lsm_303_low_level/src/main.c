
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>


#include <stdio.h>

#include "lsm303_ll.h"
#include "buttons.h"
#include "matrix.h"



void main(void)
{
	int err;
	int x_accel,y_accel,z_accel;	
	err = lsm303_ll_begin();
	if (err < 0)
	{
		 printf("\nError initializing lsm303.  Error code = %d\n",err);  
         while(1);

	}

	err = matrix_begin();
    if (err < 0)
	{
		 printf("\nError initializing LED matrix.  Error code = %d\n",err);  
         while(1);

	}
	matrix_all_off();

	while (1) {
		k_msleep(100);
        x_accel = lsm303_ll_readAccelX();
		y_accel = lsm303_ll_readAccelY();
        z_accel = lsm303_ll_readAccelZ();
        printk("Accel : %d, %d, %d\n",x_accel,y_accel,z_accel);
	}
}
