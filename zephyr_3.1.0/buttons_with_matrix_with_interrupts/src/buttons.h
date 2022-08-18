#ifndef __BUTTONS_H
#define __BUTTONS_H
#include <zephyr/zephyr.h>
#include <zephyr/device.h>	
#include <stdio.h>
// A typedef for a function pointer for an interrupt callback
typedef void (*fptr)(void);

int get_buttonA();
int get_buttonB();
int buttons_begin();
int attach_callback_to_button_a(fptr callback_function);
#endif
