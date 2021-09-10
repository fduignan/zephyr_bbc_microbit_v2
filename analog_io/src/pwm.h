#ifndef __PWM_H
#define __PWM_H
#define PWM_FREQ 10000
#define PWM_PERIOD_US (1000000/PWM_FREQ)
int pwm_begin();
int pwm_write(uint16_t value);
#endif
