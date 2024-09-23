#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
// MX25L8005 Flash chip interface driver
static const struct spi_config cfg = {
	.frequency = 1000000,
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.slave = 0	
};
static const struct device *flash_chip;
static const struct device *gpio1;
int init_spi()
{
	
	// Now configure the GPIO's for use as DC and Reset
	// The MISO pin (not used) will be reassigned as Reset
	int ret=0;
    gpio1=DEVICE_DT_GET(DT_NODELABEL(gpio1));
	if (gpio1==0)
	{
		printk("Error acquiring GPIO 1 interface\n");
	}
	gpio_pin_set(gpio1,2,1);
	ret = gpio_pin_configure(gpio1,2,GPIO_OUTPUT);
	flash_chip = device_get_binding("FLASH_INTERFACE");
	if (flash_chip==NULL)
	{
		printf("Error acquiring SPI interface\n");
		return -1;
	}
	else
	{
		printf("Got SPI interface\n");
		if (!device_is_ready(flash_chip))
		{
			printf("Device is not ready\n");
			return -2;
		}
	}	
	return ret;
}
void spi_exchange(uint8_t * tx_data, uint32_t tx_len, uint8_t * rx_data, uint32_t expected_rx_len)
{
	gpio_pin_set(gpio1,2,0);	
	struct spi_buf tx_buf = {.buf = tx_data, .len = tx_len};
	struct spi_buf rx_buf = {.buf = rx_data, .len = expected_rx_len};
	struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};
	struct spi_buf_set rx_bufs = {.buffers = &rx_buf, .count = 1};   
	spi_transceive(flash_chip, &cfg, &tx_bufs, &rx_bufs);
	gpio_pin_set(gpio1,2,1);
}

int read_electronic_signature(uint8_t *sig,uint32_t sig_len)
{
	uint8_t cmd=0x90;
	spi_exchange(&cmd,1,sig,sig_len); // should return the values 0x13 and 0xc2 in locations 4 and 5 in sig	
	return 0;
}
int read_status_register(uint8_t *stat)
{
	uint8_t cmd=0x05;
	uint8_t rxdata[2];
	spi_exchange(&cmd,1,rxdata,2); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig
	*stat=rxdata[1];
	return 0;
}
int write_status_register(uint8_t stat)
{
	uint8_t txdata[2];
	txdata[0]=0x04;
	txdata[1]=stat;	
	spi_exchange(txdata,2,NULL,0); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig
	return 0;
}
int busy() {
// Must now wait for write to complete.
// Poll the Write In Progess bit in the status register (LSB)
	uint8_t stat;
	read_status_register(&stat);
	return (stat & 1);
}
int write_enable()
{
	uint8_t cmd=0x06;
	spi_exchange(&cmd,1,NULL,0); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig
	return 0;
}
int write_disable()
{
	uint8_t cmd=0x04;
	spi_exchange(&cmd,1,NULL,0); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig
	return 0;
}
int power_down(void)
{
	uint8_t cmd=0xb9;
	spi_exchange(&cmd,1,NULL,0); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig	
	return 0;
}
int power_up(void)
{
	uint8_t cmd=0xab;
	spi_exchange(&cmd,1,NULL,0); // should return the values 0xc2 and 0x13 in locations 4 and 5 in sig	
	return 0;
}
int read_data(uint32_t address, uint8_t *data, uint32_t len)
{
	uint8_t txdata[4];
	txdata[0]=0x03;
	txdata[1]=address>>16;	
	txdata[2]=(address >> 8) & 0xff;
	txdata[3]=address & 0xff;	
	gpio_pin_set(gpio1,2,0);	// CS low

	struct spi_buf cmd_buf = {.buf = txdata, .len = 4};
	struct spi_buf_set cmd_bufs = {.buffers = &cmd_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &cmd_bufs, NULL);

	struct spi_buf data_buf = {.buf = data, .len = len};
	struct spi_buf_set data_bufs = {.buffers = &data_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, NULL,&data_bufs);
	
	gpio_pin_set(gpio1,2,1); 	// CS High
	return 0;
}
int page_program(uint32_t address, uint8_t *data, uint32_t len) // program page.  8LSB's of address should be 0.  len <= 256;
{
	uint8_t txdata[5];
	txdata[0]=0x02;
	txdata[1]=address>>16;	
	txdata[2]=(address >> 8) & 0xff;
	txdata[3]=address & 0xff;
	
	gpio_pin_set(gpio1,2,0);	// CS low
	
	struct spi_buf cmd_buf = {.buf = txdata, .len = 4};
	struct spi_buf_set cmd_bufs = {.buffers = &cmd_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &cmd_bufs, NULL);
	
	struct spi_buf data_buf = {.buf = data, .len = len};
	struct spi_buf_set data_bufs = {.buffers = &data_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &data_bufs, NULL);

	
	gpio_pin_set(gpio1,2,1); 	// CS High
	
	while (busy())
	{
		printk(".");
	}
	return 0;
}
int sector_erase(uint32_t sector_address)
{
	uint8_t txdata[5];
	txdata[0]=0x20;
	txdata[1]=sector_address>>16;	
	txdata[2]=(sector_address >> 8) & 0xff;
	txdata[3]=0;
	
	gpio_pin_set(gpio1,2,0);	// CS low
	
	struct spi_buf cmd_buf = {.buf = txdata, .len = 4};
	struct spi_buf_set cmd_bufs = {.buffers = &cmd_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &cmd_bufs, NULL);
	gpio_pin_set(gpio1,2,1); 	// CS High
	while (busy())
	{
		printk(".");
	};
	return 0;
}
int block_erase(uint32_t block_address)
{
	uint8_t txdata[5];
	txdata[0]=0x52;
	txdata[1]=block_address>>16;	
	txdata[2]=(block_address >> 8) & 0xff;
	txdata[3]=0;
	
	gpio_pin_set(gpio1,2,0);	// CS low
	
	struct spi_buf cmd_buf = {.buf = txdata, .len = 4};
	struct spi_buf_set cmd_bufs = {.buffers = &cmd_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &cmd_bufs, NULL);
	gpio_pin_set(gpio1,2,1); 	// CS High
	while (busy())
	{
		printk(".");
	};
	return 0;
}
int chip_erase()
{
	uint8_t txdata[5];
	txdata[0]=0x60;
		
	gpio_pin_set(gpio1,2,0);	// CS low
	
	struct spi_buf cmd_buf = {.buf = txdata, .len = 1};
	struct spi_buf_set cmd_bufs = {.buffers = &cmd_buf, .count = 1};	
	spi_transceive(flash_chip, &cfg, &cmd_bufs, NULL);
	gpio_pin_set(gpio1,2,1); 	// CS High
	// show a "spinning" line on the console to indicate that we are waiting for chip erase to finish
	char spin[]="\\|/-";
	int spin_index=0;
	printk("\n");
	while (busy())
	{
		printk("\r%c",spin[spin_index]);
		spin_index++;
		if (spin_index >3)
			spin_index = 0;
		k_msleep(10);
	};	
	printk("\n");
	return 0;
}
