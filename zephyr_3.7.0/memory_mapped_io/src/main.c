
// Blink the top left LED in the LED matrix using director memory writes
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <stdint.h>

/* Port 0 starts at address 0x50000000
 * It occupies a number of addresses which are offset from this base address
 * The output register is at offset 0x504
 * The input register is at offset 0x510
 * The direction register is at offset  0x514
 * In the following example writes to registers are full 32 bit
 * writes (rather than focussing on a particular * bit).  This is
 * not good practice but it keeps the example really simple
*/
#define P0_OUT (*(volatile uint32_t *)0x50000504)
#define P0_IN  (*(volatile uint32_t *)0x50000510)
#define P0_DIR (*(volatile uint32_t *)0x50000514)


int main(void)
{
	int ret;
	// Row 1 Anode is connected to GPIO0.21
	// Col 1 Cathode is connected to GPIO0.28
	P0_DIR |= (1 << 21) + (1 << 28); // Make bits 21 and 28 outputs)
	P0_OUT &= ~(1 << 28); // Drive Cathode low
	while (1) {
		P0_OUT |= (1 << 21);	// drive anode high
		k_msleep(100);			// wait 100ms
		P0_OUT &= ~(1 << 21); 	// drive anode low
		k_msleep(100);			// wait 100ms
	}
	return 0;
}
