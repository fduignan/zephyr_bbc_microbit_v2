/*
 * SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>	
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "bmp280.h"
// ST7789 display demo
uint32_t prbs()
{
    // This is an unverified 31 bit PRBS generator
    // It should be maximum length but this has not been checked 
    static uint32_t shift_register=0xaa551199;
    unsigned long new_bit=0;
    static int busy=0; // need to prevent re-entrancy here  
    if (!busy)
    {
        busy=1;
        new_bit= ((shift_register & (1<<27))>>27) ^ ((shift_register & (1<<30))>>30);
        new_bit= ~new_bit;
        new_bit = new_bit & 1;
        shift_register=shift_register << 1;
        shift_register=shift_register | (new_bit);
        busy=0;
    }
    return shift_register & 0x7ffffff; // return 31 LSB's 
}
uint32_t random(uint32_t lower,uint32_t upper)
{
    return (prbs()%(upper-lower))+lower;
}
void main(void)
{
	int ret;
	
	ret = display_begin();
	if (ret < 0)
	{
		printf("\nError initializing display.  Error code = %d\n",ret);
		while(1);
	}
	ret = bmp280_begin();
	if (ret < 0)
	{
		printf("\nError initializing i2c interface.  Error code = %d\n",ret);	
		while(1);
	}
	display_fillRectangle(0,0,SCREEN_WIDTH, SCREEN_HEIGHT,display_RGBToWord(0xff,0xff,0));
	display_fillRectangle(10,10,SCREEN_WIDTH-20, (SCREEN_HEIGHT/2)-20,display_RGBToWord(0xaf,0xaf,0xff));
	display_fillRectangle(10,(SCREEN_HEIGHT/2)+10,SCREEN_WIDTH-20, (SCREEN_HEIGHT/2)-20,display_RGBToWord(0xff,0xaf,0xaf));
	while(1)
	{    
        char display_string[256];
		sprintf(display_string,"Temp (x100)=%d",bmp280_readTemperature());
		display_print(display_string,strlen(display_string),20,30,0,display_RGBToWord(0xaf,0xaf,0xff));
		sprintf(display_string,"Pressure (x100) = %d",bmp280_readPressure());
		display_print(display_string,strlen(display_string),20,(SCREEN_HEIGHT/2)+30,0,display_RGBToWord(0xff,0xaf,0xaf));
        printf("Temp (x100) = %d\n",bmp280_readTemperature());
		printf("Pressure(x 100) = %d\n",bmp280_readPressure());

	}
}
