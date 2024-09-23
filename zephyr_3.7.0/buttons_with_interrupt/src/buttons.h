#ifndef __BUTTONS_H
#define __BUTTONS_H
int get_buttonA();
int get_buttonB();
int buttons_begin();
typedef void (*fptr)(void);
int attach_callback_to_button_a(fptr callback_function);

#endif
