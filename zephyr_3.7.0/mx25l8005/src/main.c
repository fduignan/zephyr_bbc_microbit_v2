/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include "ext_flash_mem.h"
// Pin usage by SPI bus defined in app.overlay.	
int main(void)
{
	int ret=0;
	uint8_t txdata[]={0x05, 0x06,0x07,0x08};		
	uint8_t rxdata[10];
	init_spi();
	write_enable();	
//	sector_erase(0xfed00);
	page_program(0xfed04,txdata,4);	
	while(1)
	{				
		read_data(0xfed04,rxdata,4);
		printk("rxdata = %x %x %x %x\n",rxdata[0],rxdata[1],rxdata[2],rxdata[3]);		
		k_msleep(10);
		
	}
	return ret;
}
