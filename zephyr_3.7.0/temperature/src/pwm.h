#ifndef __PWM_H
#define __PWM_H
// Set PWM period to 1ms (1 million nano seconds)
#define PWM_PERIOD_NS 1000000
int pwm_begin();
int pwm_write(uint32_t value);
#endif
