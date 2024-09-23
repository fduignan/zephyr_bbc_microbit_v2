#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "matrix.h"
#include "buttons.h"

int main(void)
{
	int ret;
  	ret = buttons_begin();
  	if (ret < 0)
  	{
		printk("Error initializing the buttons\n");
		return -1;
	}
	ret = matrix_begin();
	if (ret < 0)
  	{
		printk("Error initializing the matrix\n");
		return -2;
	}
	while (1) {
		if (get_buttonA())
		{
			matrix_put_pattern(0x1f,0);
		}
		else
		{
			matrix_put_pattern(0,0);
		}
	}
	return 0;
}
