#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "matrix.h"
#include "buttons.h"

void my_button_a_callback(void);

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
	attach_callback_to_button_a(my_button_a_callback);
	while (1) {
		k_msleep(100);
	}
	return 0;
}
void my_button_a_callback(void)
{
	static int state=0;
	if (state == 1)
	{
		matrix_put_pattern(0x1f,0);
	}
	else
	{
		matrix_put_pattern(0,0);
	}
	state = state ^ 1;
}
